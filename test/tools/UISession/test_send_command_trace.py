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

import contextlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from send_command_trace import convert, convert_requests, main


def marker(phase, pid, timestamp, request="1", chain=""):
    return (f"worker-{pid} [000] ...1 {timestamp}: tracing_mark_write: "
            f"{phase}|{pid}|H:{chain}SendCommand.FinishCallbackIPC.{request}|0|I39\n")


class SendCommandTraceTest(unittest.TestCase):
    def test_cross_process_pair_preserves_times_and_process_ids(self):
        result = convert([marker("S", 100, "1.000000"), marker("F", 200, "1.003000")])
        self.assertEqual(result["excluded"], [])
        self.assertEqual(result["samples"][0]["duration_ms"], 3)
        self.assertEqual(result["samples"][0]["sa_pid"], 200)
        self.assertEqual([event["pid"] for event in result["traceEvents"]], [100, 100])
        self.assertEqual([event["ts"] for event in result["traceEvents"]], [1_000_000, 1_003_000])
        self.assertEqual([event["ph"] for event in result["traceEvents"]], ["b", "e"])

    def test_threshold_uses_nanoseconds_without_rounding_down(self):
        for end, expected in [("1.005000000", True), ("1.005000001", False)]:
            with self.subTest(end=end):
                result = convert([marker("S", 100, "1.000000"), marker("F", 200, end)])
                self.assertEqual(result["samples"][0]["within_5ms"], expected)

    def test_full_uint64_id_and_distributed_prefix(self):
        request = str(2**64 - 1)
        result = convert([marker("S", 100, "1.0", request, "[abc,2,1]#"),
                          marker("F", 200, "1.001", request)])
        self.assertEqual(result["samples"][0]["request_id"], request)
        self.assertEqual(result["traceEvents"][0]["id"], "0xffffffffffffffff")

    def test_missing_or_duplicate_endpoints_are_excluded(self):
        start, end = marker("S", 100, "1.0"), marker("F", 200, "1.001")
        for lines in ([start], [end], [start, start, end], [start, end, end]):
            with self.subTest(lines=lines):
                result = convert(lines)
                self.assertEqual(result["traceEvents"], [])
                self.assertEqual(len(result["excluded"]), 1)

    def test_wrong_order_or_same_process_is_excluded(self):
        for end in [marker("F", 200, "0.999"), marker("F", 100, "1.001")]:
            result = convert([marker("S", 100, "1.0"), end])
            self.assertEqual(result["samples"], [])
            self.assertEqual(len(result["excluded"]), 1)

    def test_unordered_input_and_interleaved_requests(self):
        result = convert([marker("F", 200, "1.004", "2"), marker("S", 100, "1.0"),
                          marker("S", 300, "1.002", "2"), marker("F", 200, "1.003")])
        self.assertEqual(len(result["samples"]), 2)
        self.assertEqual(result["excluded"], [])
        times = [event["ts"] for event in result["traceEvents"]]
        self.assertEqual(times, sorted(times))

    def test_unrelated_trace_is_ignored(self):
        self.assertEqual(convert(["unrelated trace\n"])["samples"], [])


def request_marker(phase, pid, timestamp, mode="Legacy", kind="Node"):
    return (f"worker-{pid} [000] ...1 {timestamp}: trace_marker: "
            f"{phase}|{pid}|H:[abc,2,1]#SendCommand.Request.{mode}.{kind}|0|I39\n")


def request_comparison(callback_end="1.012700000", kind="Node"):
    return [request_marker("S", 100, "1.0", kind=kind),
            request_marker("F", 200, "1.002", kind=kind),
            request_marker("S", 100, "1.010", "Callback", kind),
            request_marker("F", 200, callback_end, "Callback", kind)]


class SendCommandRequestTraceTest(unittest.TestCase):
    def test_cross_process_comparison_preserves_times_and_process_ids(self):
        result = convert_requests(reversed(request_comparison()))
        self.assertEqual(result["excluded"], [])
        self.assertEqual(result["comparisons"], [{"kind": "Node", "sa_pid": 100, "app_pid": 200,
                                               "legacy_ms": 2, "callback_ms": 2.7,
                                               "delta_ms": 0.7, "within_1ms": True}])
        self.assertEqual([event["pid"] for event in result["traceEvents"]], [100] * 4)
        self.assertEqual({event["id"] for event in result["traceEvents"]}, {"0x1", "0x2"})
        self.assertEqual([event["ts"] for event in result["traceEvents"]],
                         [1_000_000, 1_002_000, 1_010_000, 1_012_700])
        self.assertEqual([event["ph"] for event in result["traceEvents"]], ["b", "e", "b", "e"])

    def test_delta_threshold_uses_nanoseconds_and_allows_faster_callback(self):
        for end, expected in [("1.013000000", True), ("1.013000001", False), ("1.011", True)]:
            with self.subTest(end=end):
                result = convert_requests(request_comparison(end))
                self.assertEqual(result["comparisons"][0]["within_1ms"], expected)

    def test_global_entry_has_independent_comparison(self):
        result = convert_requests(request_comparison(kind="Global"))
        self.assertEqual(result["comparisons"][0]["kind"], "Global")
        self.assertEqual(result["excluded"], [])

    def test_missing_and_duplicate_endpoints_prevent_comparison(self):
        lines = request_comparison()
        for incomplete in (lines[:-1], lines[1:], lines + [lines[0]], lines + [lines[-1]], lines[2:]):
            with self.subTest(lines=incomplete):
                result = convert_requests(incomplete)
                self.assertEqual(result["comparisons"], [])
                self.assertTrue(result["excluded"])

    def test_repeated_serial_commands_are_ambiguous(self):
        result = convert_requests(request_comparison() + [
            request_marker("S", 100, "2.0"), request_marker("F", 200, "2.002")])
        self.assertEqual(result["comparisons"], [])
        self.assertTrue(result["excluded"])

    def test_overlapping_requests_are_excluded(self):
        lines = request_comparison("1.011")
        lines[2] = request_marker("S", 100, "1.001", "Callback")
        result = convert_requests(lines)
        self.assertEqual(result["traceEvents"], [])
        self.assertEqual(result["comparisons"], [])
        self.assertTrue(any(item["reason"] == "overlapping requests" for item in result["excluded"]))

    def test_invalid_order_same_process_and_different_process_pair(self):
        for pid, end in [(200, "0.99"), (100, "1.013"), (300, "1.013")]:
            with self.subTest(pid=pid, end=end):
                lines = request_comparison()
                lines[-1] = request_marker("F", pid, end, "Callback")
                result = convert_requests(lines)
                self.assertEqual(result["comparisons"], [])
                self.assertTrue(result["excluded"])

    def test_finish_markers_do_not_become_request_samples(self):
        result = convert_requests([marker("S", 100, "1.0"), marker("F", 200, "1.003")])
        self.assertEqual(result["samples"], [])
        self.assertEqual(result["comparisons"], [])


class SendCommandTraceCliTest(unittest.TestCase):
    def run_cli(self, lines, metric="request"):
        with tempfile.TemporaryDirectory() as directory:
            source, output = Path(directory) / "input.trace", Path(directory) / "output.json"
            source.write_text("".join(lines), encoding="utf-8")
            stdout = io.StringIO()
            with patch("sys.argv", ["send_command_trace.py", str(source), str(output), "--metric", metric]), \
                    contextlib.redirect_stdout(stdout):
                code = main()
            return code, stdout.getvalue(), json.loads(output.read_text(encoding="utf-8"))

    def test_no_command_markers_explains_empty_result(self):
        lines = ["worker [000] 1.0: tracing_mark_write: B|100|H:Other|I39\n",
                 "worker [000] 1.1: sched_switch: next_comm=.sendcommandzdh next_pid=100\n"]
        code, text, result = self.run_cli(lines)
        self.assertEqual(code, 2)
        self.assertIn("no SendCommand. markers", text)
        self.assertIn("No latency was measured", text)
        self.assertIn("No async lane generated", text)
        self.assertEqual(result["diagnostics"]["trace_records"], 1)
        self.assertEqual(result["diagnostics"]["send_command_records"], 0)

    def test_empty_file_is_not_a_performance_pass(self):
        code, text, result = self.run_cli([])
        self.assertEqual(code, 2)
        self.assertIn("no HiTrace/ftrace marker records", text)
        self.assertEqual(result["diagnostics"]["lines"], 0)

    def test_wrong_metric_is_explained(self):
        for lines, metric in [(request_comparison(), "finish"),
                              ([marker("S", 200, "1.0"), marker("F", 100, "1.003")], "request")]:
            with self.subTest(metric=metric):
                code, text, _ = self.run_cli(lines, metric)
                self.assertEqual(code, 2)
                self.assertIn(f"none match --metric {metric}", text)

    def test_missing_and_duplicate_endpoints_are_printed(self):
        lines = request_comparison()
        for data, reason in [(lines[:-1], "missing end"), (lines[1:], "missing start"),
                             (lines + [lines[0]], "duplicate endpoints")]:
            with self.subTest(reason=reason):
                code, text, result = self.run_cli(data)
                self.assertEqual(code, 2)
                self.assertIn(reason, text)
                self.assertIn("starts", result["excluded"][0])
                self.assertIn("ends", result["excluded"][0])

    def test_callback_only_reports_missing_baseline(self):
        code, text, result = self.run_cli(request_comparison()[2:])
        self.assertEqual(code, 2)
        self.assertIn("no valid legacy/callback comparison", text)
        self.assertEqual(result["diagnostics"]["request_markers"], 2)

    def test_unrecognized_named_markers_do_not_silently_disappear(self):
        malformed = request_marker("S", 100, "3.0").replace("|0|", "|99|")
        code, text, result = self.run_cli([malformed])
        self.assertEqual(code, 2)
        self.assertIn("format is unsupported", text)
        self.assertEqual(result["diagnostics"]["unrecognized_lines"], [1])
        # Valid samples must not hide another unrecognized request in the same capture.
        code, text, result = self.run_cli(request_comparison() + [malformed])
        self.assertEqual(code, 2)
        self.assertIn("unrecognized marker format", text)

    def test_diagnostic_probe_invalidates_performance_capture(self):
        probe = "worker [000] 1.0: tracing_mark_write: C|100|H:SendCommand.TraceDiagnostic|1|I39\n"
        for data, metric in [(request_comparison(), "request"),
                             ([marker("S", 200, "1.0"), marker("F", 100, "1.003")], "finish")]:
            with self.subTest(metric=metric):
                code, text, result = self.run_cli(data + [probe], metric)
                self.assertEqual(code, 2)
                self.assertIn("diagnostic capture", text)
                self.assertEqual(result["diagnostics"]["diagnostic_probes"], 1)

    def test_utf8_bom_and_crlf_are_supported(self):
        lines = request_comparison()
        lines[0] = "\ufeff" + lines[0]
        code, _, result = self.run_cli([line.replace("\n", "\r\n") for line in lines])
        self.assertEqual(code, 0)
        self.assertEqual(result["diagnostics"]["request_markers"], 4)

    def test_binary_or_non_utf8_input_does_not_overwrite_output(self):
        for data in (b"\0binary trace", b"\xff\xfe\x00\x00"):
            with self.subTest(data=data), tempfile.TemporaryDirectory() as directory:
                source, output = Path(directory) / "input.trace", Path(directory) / "output.json"
                source.write_bytes(data)
                output.write_text("previous result", encoding="utf-8")
                stderr = io.StringIO()
                with patch("sys.argv", ["send_command_trace.py", str(source), str(output)]), \
                        contextlib.redirect_stderr(stderr), self.assertRaises(SystemExit) as error:
                    main()
                self.assertEqual(error.exception.code, 2)
                self.assertIn("HiTrace/ftrace text", stderr.getvalue())
                self.assertEqual(output.read_text(encoding="utf-8"), "previous result")

    def test_request_exit_codes_and_default_finish_compatibility(self):
        cases = [(request_comparison(), ["--metric", "request"], 0),
                 (request_comparison("1.013000001"), ["--metric", "request"], 1),
                 (request_comparison()[:-1], ["--metric", "request"], 2),
                 ([], ["--metric", "request"], 2),
                 ([marker("S", 100, "1.0"), marker("F", 200, "1.003")], [], 0)]
        for lines, options, expected in cases:
            with self.subTest(options=options, expected=expected), tempfile.TemporaryDirectory() as directory:
                source, output = Path(directory) / "input.trace", Path(directory) / "output.json"
                original = "".join(lines)
                source.write_text(original, encoding="utf-8")
                with patch("sys.argv", ["send_command_trace.py", str(source), str(output), *options]), \
                        contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(main(), expected)
                self.assertIn("traceEvents", json.loads(output.read_text(encoding="utf-8")))
                self.assertEqual(source.read_text(encoding="utf-8"), original)

    def test_original_capture_cannot_be_overwritten(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "input.trace"
            source.write_text("original", encoding="utf-8")
            with patch("sys.argv", ["send_command_trace.py", str(source), str(source)]), \
                    contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as error:
                main()
            self.assertEqual(error.exception.code, 2)
            self.assertEqual(source.read_text(encoding="utf-8"), "original")


if __name__ == "__main__":
    unittest.main()
