#!/usr/bin/env python3
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Export UISession request/result markers as derived async lanes in JSON trace format."""

import argparse
from collections import defaultdict
from decimal import Decimal
import json
from pathlib import Path
import re


MARKER = re.compile(
    r"(?P<time>\d+\.\d+):\s+(?:tracing_mark_write|trace_marker):\s+"
    r"(?P<phase>[SF])\|(?P<pid>\d+)\|H:(?:\[[^\]\r\n]*\]#)?"
    r"SendCommand\.FinishCallbackIPC\.(?P<request>\d+)\|0(?:\||\s*$)"
)
LIMIT_NS = 5_000_000
REQUEST_MARKER = re.compile(
    r"(?P<time>\d+\.\d+):\s+(?:tracing_mark_write|trace_marker):\s+"
    r"(?P<phase>[SF])\|(?P<pid>\d+)\|H:(?:\[[^\]\r\n]*\]#)?"
    r"SendCommand\.Request\.(?P<mode>Legacy|Callback)\.(?P<kind>Node|Global)\|0(?:\||\s*$)"
)
REQUEST_DELTA_LIMIT_NS = 1_000_000
TRACE_RECORD = re.compile(r"(?:tracing_mark_write|trace_marker):\s+")
COMMAND_NAME = re.compile(r"\|(?:H:)?(?:\[[^\]\r\n]*\]#)?SendCommand\.")
DIAGNOSTIC_MARKER = re.compile(r"C\|\d+\|H:SendCommand\.TraceDiagnostic\|1(?:\||\s*$)")


def endpoint_error(pair):
    starts, ends = len(pair["S"]), len(pair["F"])
    if starts == 1 and ends == 1:
        return None
    if starts > 1 or ends > 1:
        reason = "duplicate endpoints; capture only one request per entry"
    else:
        reason = "missing start" if not starts else "missing end"
    return {"reason": reason, "starts": starts, "ends": ends}


class CaptureDiagnostics:
    """Inspect the input in one streaming pass; never count scheduler process names as markers."""

    def __init__(self, metric):
        self.metric = metric
        self.counts = dict(lines=0, trace_records=0, send_command_records=0,
                           request_markers=0, finish_markers=0, diagnostic_probes=0,
                           unrecognized_markers=0)
        self.unrecognized_lines = []

    def scan(self, lines):
        for line in lines:
            self.counts["lines"] += 1
            if "\0" in line:
                raise ValueError("binary trace detected; decode it to HiTrace/ftrace text first")
            self.inspect_record(line)
            yield line

    def inspect_record(self, line):
        record = TRACE_RECORD.search(line)
        if not record:
            return
        self.counts["trace_records"] += 1
        payload = line[record.end():]
        if not COMMAND_NAME.search(payload):
            return
        self.counts["send_command_records"] += 1
        self.counts["diagnostic_probes"] += bool(DIAGNOSTIC_MARKER.match(payload))
        self.counts["request_markers"] += bool(REQUEST_MARKER.search(line))
        self.counts["finish_markers"] += bool(MARKER.search(line))
        pattern = REQUEST_MARKER if self.metric == "request" else MARKER
        prefix = "SendCommand.Request." if self.metric == "request" else "SendCommand.FinishCallbackIPC."
        if prefix in payload and not pattern.search(line):
            self.counts["unrecognized_markers"] += 1
            if len(self.unrecognized_lines) < 5:
                self.unrecognized_lines.append(self.counts["lines"])

    def finish(self, result):
        result["diagnostics"] = dict(self.counts, unrecognized_lines=self.unrecognized_lines)
        if self.counts["diagnostic_probes"]:
            result["excluded"].append({"reason": "diagnostic capture; disable SendCommandTraceDiagnostic "
                                       "and collect a fresh trace for performance measurements"})
        if self.counts["unrecognized_markers"]:
            result["excluded"].append({"reason": "unrecognized marker format", "lines": self.unrecognized_lines})

    def explain(self):
        print("Input: " + ", ".join(f"{key}={value}" for key, value in self.counts.items()))
        if self.counts[f"{self.metric}_markers"]:
            return
        if self.counts["unrecognized_markers"]:
            print(f"ERROR: {self.metric} names exist but their format is unsupported; "
                  f"inspect input lines {self.unrecognized_lines}.")
        elif not self.counts["trace_records"]:
            print("ERROR: no HiTrace/ftrace marker records found. Check that this is a decoded trace text file.")
        elif not self.counts["send_command_records"]:
            print("ERROR: no SendCommand. markers in this capture. No latency was measured. "
                  "Check trace emission and capture during the command.")
        else:
            print(f"ERROR: SendCommand markers exist, but none match --metric {self.metric}. "
                  "Check the metric and deployed trace version.")


def convert(lines):
    """Require one start and one end per request in a single SA lifetime/capture."""
    markers = defaultdict(lambda: {"S": [], "F": []})
    for line in lines:
        match = MARKER.search(line)
        if match:
            timestamp = int(Decimal(match["time"]) * 1_000_000_000)
            markers[match["request"]][match["phase"]].append((timestamp, int(match["pid"])))

    events, samples, excluded = [], [], []
    for request, pair in markers.items():
        error = endpoint_error(pair)
        if error:
            excluded.append(dict(request_id=request, **error))
            continue
        start, app_pid = pair["S"][0]
        end, sa_pid = pair["F"][0]
        if end < start or app_pid == sa_pid:
            excluded.append({"request_id": request, "reason": "invalid time order or same process"})
            continue
        duration = end - start
        sample = {"request_id": request, "app_pid": app_pid, "sa_pid": sa_pid,
                  "duration_ms": duration / 1_000_000, "within_5ms": duration <= LIMIT_NS}
        samples.append(sample)
        # A derived display lane uses the app PID at both ends. Actual source PIDs
        # remain in args; no original trace lines or measured timestamps are changed.
        common = {"name": "SendCommand.FinishCallbackIPC", "cat": "UISession",
                  "pid": app_pid, "tid": app_pid, "id": hex(int(request)), "args": sample}
        events.append(dict(common, ph="b", ts=start / 1000))
        events.append(dict(common, ph="e", ts=end / 1000))

    events.sort(key=lambda event: event["ts"])
    return {"traceEvents": events, "displayTimeUnit": "ms", "samples": samples, "excluded": excluded}


def exclude_overlapping_requests(pairs, excluded):
    overlapping = set()
    for key, (start, end, _, _) in pairs.items():
        for other, (other_start, other_end, _, _) in pairs.items():
            if key != other and start < other_end and other_start < end:
                overlapping.update((key, other))
    for mode, kind in sorted(overlapping):
        excluded.append({"mode": mode, "kind": kind, "reason": "overlapping requests"})
        del pairs[(mode, kind)]


def convert_requests(lines):
    """Compare one serial legacy/callback pair per entry kind in a controlled capture."""
    markers = defaultdict(lambda: {"S": [], "F": []})
    for line in lines:
        match = REQUEST_MARKER.search(line)
        if match:
            timestamp = int(Decimal(match["time"]) * 1_000_000_000)
            markers[(match["mode"], match["kind"])][match["phase"]].append((timestamp, int(match["pid"])))

    pairs, excluded = {}, []
    for (mode, kind), pair in markers.items():
        error = endpoint_error(pair)
        if error:
            excluded.append(dict(mode=mode, kind=kind, **error))
            continue
        start, sa_pid = pair["S"][0]
        end, app_pid = pair["F"][0]
        if end < start or app_pid == sa_pid:
            excluded.append({"mode": mode, "kind": kind, "reason": "invalid time order or same process"})
            continue
        pairs[(mode, kind)] = (start, end, sa_pid, app_pid)

    exclude_overlapping_requests(pairs, excluded)

    comparisons = []
    for kind in sorted({kind for _, kind in markers}):
        legacy, callback = pairs.get(("Legacy", kind)), pairs.get(("Callback", kind))
        if legacy is None or callback is None:
            excluded.append({"kind": kind, "reason": "no valid legacy/callback comparison"})
            continue
        if legacy[2:] != callback[2:]:
            excluded.append({"kind": kind, "reason": "legacy/callback process pair differs"})
            continue
        legacy_ns, callback_ns = legacy[1] - legacy[0], callback[1] - callback[0]
        delta_ns = callback_ns - legacy_ns
        comparisons.append({"kind": kind, "sa_pid": legacy[2], "app_pid": legacy[3],
                            "legacy_ms": legacy_ns / 1_000_000, "callback_ms": callback_ns / 1_000_000,
                            "delta_ms": delta_ns / 1_000_000, "within_1ms": delta_ns <= REQUEST_DELTA_LIMIT_NS})

    events, samples = [], []
    for trace_id, ((mode, kind), (start, end, sa_pid, app_pid)) in enumerate(sorted(pairs.items()), 1):
        sample = {"mode": mode, "kind": kind, "sa_pid": sa_pid, "app_pid": app_pid,
                  "duration_ms": (end - start) / 1_000_000}
        samples.append(sample)
        # SA is the display lane; the actual UI-process endpoint remains in args.
        common = {"name": f"SendCommand.Request.{mode}.{kind}", "cat": "UISession",
                  "pid": sa_pid, "tid": sa_pid, "id": hex(trace_id), "args": sample}
        events.append(dict(common, ph="b", ts=start / 1000))
        events.append(dict(common, ph="e", ts=end / 1000))
    events.sort(key=lambda event: event["ts"])
    return {"traceEvents": events, "displayTimeUnit": "ms", "samples": samples,
            "comparisons": comparisons, "excluded": excluded}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path, help="decoded HiTrace/ftrace text from one SA lifetime")
    parser.add_argument("output", type=Path, help="derived JSON trace for trace visualization tools")
    parser.add_argument("--metric", choices=("finish", "request"), default="finish",
                        help="finish: result IPC <=5ms; request: callback minus legacy dispatch <=1ms")
    args = parser.parse_args()
    if args.trace.resolve() == args.output.resolve():
        parser.error("output must differ from the original trace")
    diagnostics = CaptureDiagnostics(args.metric)
    try:
        with args.trace.open(encoding="utf-8-sig") as source:
            lines = diagnostics.scan(source)
            result = convert_requests(lines) if args.metric == "request" else convert(lines)
    except (OSError, ValueError) as error:
        parser.error(f"cannot read trace as UTF-8 HiTrace/ftrace text: {error}")
    diagnostics.finish(result)
    args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    diagnostics.explain()
    if args.metric == "request":
        measurements, limit = result["comparisons"], "1ms"
        for comparison in measurements:
            print(f"{comparison['kind']}: legacy_ms={comparison['legacy_ms']}, "
                  f"callback_ms={comparison['callback_ms']}, delta_ms={comparison['delta_ms']}")
        print(f"comparisons={len(measurements)}")
    else:
        measurements, limit = result["samples"], "5ms"
    slow = sum(not sample[f"within_{limit}"] for sample in measurements)
    print(f"paired={len(result['samples'])}, over_{limit}={slow}, excluded={len(result['excluded'])}")
    for item in result["excluded"]:
        print("EXCLUDED: " + json.dumps(item, ensure_ascii=False))
    print(f"Analysis JSON: {args.output}")
    if not result["traceEvents"]:
        print("No async lane generated: no valid start/end pair.")
    if not measurements or result["excluded"]:
        print("INCOMPLETE: no performance pass/fail conclusion; see diagnostics above.")
        return 2
    return 1 if slow else 0


if __name__ == "__main__":
    raise SystemExit(main())
