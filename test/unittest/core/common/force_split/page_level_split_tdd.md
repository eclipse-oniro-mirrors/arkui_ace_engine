# Page-level split TDD verification

## Scope and status

The baseline is `15aeee621d303eca5f8de196ec6ae8320f1edd3c^2`. The first-parent
diff includes unrelated upstream merge changes and must not be used as the
feature scope. Verification also includes `f2bffa094cdb6329bb2318c669efd05107d057c3`
and the current workspace changes.

The C++ tests below follow the existing `HWTEST_F`, numbered test names and
`@tc.name` / `@tc.desc` / `@tc.type` conventions. New Navigation cases share the existing
`NavigationPatternTestFiveNg` fixture. They explicitly call `InitPageLevelSplitTest`
and use guarded per-case cleanup so policies, selectors and logical sessions do not
leak between new cases or change the lifecycle of existing cases. Existing
entry/continuity cases explicitly set the display and stack-change state they
intend to exercise. The Stage test file's committed conflict markers
were removed while preserving both sides' tests.

**These C++ tests have not been built or executed in this Windows checkout. No
measured branch coverage is claimed. The acceptance threshold remains 90%.**

## Automated case map

| Area | Test cases | Assertions |
| --- | --- | --- |
| Parsing | `ForceSplitUtilsTest.ParseForceSplitParam031` through `040`, `ParsePageSplitOptions001` through `004` | Missing/true/false/invalid flags, malformed legacy fields ignored when enabled, invalid array items skipped, duplicate entries last-write-wins, empty arrays clear stale entries, both Router and Navigation configs |
| Policies | `ForceSplitManagerTestNg.IsPageSplitDisabled001` through `008`, `IsUpstreamPageSplitDisabled001` through `003`, `SetPageLevelSplitConfig002` | Explicit and inherited states, nearest explicit ancestor, default no-split, repeated names, absent/bottom/empty stack, replacement of policies and switching inheritance off |
| Transition policy | `ForceSplitManagerTestNg.IsTransitionShouldMovePageToPrimary003` through `006` | Directional page pairs, transPage on either side, legacy fullscreen gate ignored only when enabled, feature-off regression |
| Notifications | `ForceSplitManagerTestNg.UpdateIsTopPageSplitDisabled001/002`, existing Navigation notification cases | State-change versus duplicate notification, initial no-split state, feature-off behavior |
| Home selection | Existing `NavigationPatternTestFiveNg.TryEstablishPageLevelSplitHome001` through `011`; `NavigationPatternTestFiveNg.TryEstablishPageLevelSplitHome012` through `016` | Valid NavBar only for a sole stack entry, Navigation versus Displace reentry, duplicate node instances, demotion of the previous home, rotation without stack operations, disabled feature/missing stack |
| Session guards | `NavigationPatternTestFiveNg.ShouldKeepPageLevelSplitSession001`, `ShouldKeepPageLevelSplitHome001` | Display state, stack changes, mode, active logical session, valid/hidden NavBar, missing home or same-name replacement, split/non-split predecessor |
| Helper boundaries | `NavigationPatternTestFiveNg.CollectPageLevelSplitDestinations001/002`, `InitializePageLevelSplitHome001/002`, `ReanchorPageLevelSplitHome001` through `003` | Empty/invalid stack entries, raw versus filtered stack sizes, stable node identity, positional anchoring, missing patterns |
| Navigation selector | `NavigationPatternTestFiveNg.PageLevelNavigationSelector001/002` | Excluded Navigation cannot overwrite shared policy state; the selected Navigation refreshes its own stack before inheritance |
| Portrait logical columns | Existing `NavigationPatternTestFiveNg.PageLevelDisplacePortraitColumnsAndResume`; `NavigationPatternTestFiveNg.UpdatePageLevelDisplaceColumns001` through `009` | Push/pop/replace, same-name new instances, transparent/dialog pages, primary/home guards, removed home ends session, invalid nodes/incomplete operation, destination-home preservation |
| Stack sync isolation | `NavigationPatternTestFiveNg.AdjustNodeForStackSyncPageLevel001` | Disabled top ends page-level Displace session; feature-off keeps legacy behavior |
| Split result cache | `NavigationPatternTestFourNg.TryForceSplitIfNeeded008` through `010`; `NavigationPatternTestFiveNg.TryForceSplitIfNeededPageLevel001/002` | Restore pipeline state on unchanged true/false results; changed home instance rebuilds physical containers even with unchanged split booleans |
| Animation | `NavigationPatternTestFiveNg.CreateAnimationWithPushPageLevel001/002`, `TransitionWithPushPageLevel001/002` | Count real outgoing NavBar animation/finish calls, missing pre-node, custom transition and overlay guards, NavBar visibility |
| Stage layout | Existing Stage mode/flag tests; `ParallelStageTestFourNg.OnHomePageDetectedVirtualStack001` through `003`, `PushPagePageLevelPair001`, `PushPageNeedOperationInSplitMode004` | Cache invalidation before column queries, recognition in portrait, actual page-pair columns with duplicate URLs, consumed operation flag in split mode |

## Build and run

Use a complete OpenHarmony checkout. These force-split targets are device targets
(`!is_host_product`). Ensure `ace_enable_full_test_suite=true`; the reduced test
suite excludes the configuration parser cases. The value is already true in this
checkout's `ace_config.gni`.

From the OpenHarmony root:

```bash
./build.sh --product-name rk3568 --build-target //foundation/arkui/ace_engine/test/unittest/core/common/force_split:force_split_utils_test
./build.sh --product-name rk3568 --build-target //foundation/arkui/ace_engine/test/unittest/core/manager:force_split_manager_test_ng
./build.sh --product-name rk3568 --build-target //foundation/arkui/ace_engine/test/unittest/core/pattern/navigation:navigation_test_ng
./build.sh --product-name rk3568 --build-target //foundation/arkui/ace_engine/test/unittest/core/pattern/stage:force_split_stage_test_ng
```

Deploy the binaries to their matching device/runtime. Run the full affected suites
to include legacy regressions, not only the newly added cases:

```bash
./force_split_utils_test --gtest_filter='ForceSplitUtilsTest.*' --gtest_output=xml:force_split_utils.xml
./force_split_manager_test_ng --gtest_filter='ForceSplitManagerTestNg.*' --gtest_output=xml:force_split_manager.xml
./navigation_test_ng --gtest_filter='NavigationPatternTestFiveNg.*:NavigationPatternTestFourNg.*' --gtest_output=xml:navigation_page_level.xml
./force_split_stage_test_ng --gtest_filter='ParallelStageTest*' --gtest_output=xml:force_split_stage.xml
```

All selected cases must pass. Inspect the XML files for actual passed/failed/skipped
counts. A test source syntax check or a coverage-tool self-check is not a gtest run.

## Frontend/platform integration cases still required

The existing component test targets do not compile
`parallel_page_router_manager.cpp` or `UIContentImpl::SetForceSplitConfig`.
The Stage duplicate-URL test verifies the backend; it does not execute Router's
effective-stack collection and home-continuity entry points. Those branches need
a frontend/platform integration harness or an instrumented demo run. No frontend
module dependency was added to the component tests.

Use actual node IDs as well as URLs when verifying the following cases:

1. NAVIGATION, inheritance on: `Home -> Detail2(a) -> Detail1 -> Detail2(b)`,
   pair `Detail1 -> Detail2`. After the final push, the policy stack contains all
   four instances/names, `Detail1` is left, `Detail2(b)` is right, and the session
   home remains `Detail2(a)`.
2. Pop before the real Router stack is mutated: effective policy collection stops
   at the target **instance** and excludes outgoing pages, including duplicate
   URLs. A same-name node above the pop target cannot keep a stale home alive.
3. DISPLACE, inheritance off: `Home -> Detail1 -> Detail2 -> Detail1`, with only
   Detail1 split-enabled. Final columns are `[Detail2 | new Detail1]`.
4. DISPLACE, inheritance on, transparent Detail3: `Home -> Detail1 -> Detail3 ->
   Detail1`, rotate to portrait, pop, rotate back. Final columns retain
   `[Home | Detail3]`. Repeat rotation without a stack operation.
5. Router entry guards: disabled feature, null context/page, target not in stack,
   expired weak nodes, non-page nodes and a top page whose split policy is disabled.
   No guard should replace the current home; target-not-found must not overwrite
   the previously cached policy stack.
6. Router notification: unsupported Router split; unchanged/changed page policy;
   clearing a legacy fullscreen state; feature-off legacy fullscreen push/pop;
   no related/placeholder page loaded when page-level split is enabled.
7. Platform configuration injection: Router/Navigation configurations both forward
   the three new settings. Page-level Navigation disables placeholders; a disabled
   feature retains the configured legacy placeholder option and navigation selector.

## Branch coverage acceptance

The repository's `ace_engine_feature_enable_coverage` config uses `--coverage`
(GCOV-compatible data), not LLVM source-based instrumentation. Capture branch data
with the coverage tools matching the compiler; for example, from the OpenHarmony
root after collecting the device's `.gcda` files alongside the matching `.gcno`:

```bash
lcov --capture --directory out/rk3568 --output-file page_level.info --rc lcov_branch_coverage=1
```

An independently configured LLVM source-based instrumentation build can instead
use `llvm-profdata merge` and full `llvm-cov export` JSON. Do not pass summary-only
JSON, line-only LCOV reports, or mix LCOV and LLVM reports in a single check.

From the ace_engine root, check the combined unit/integration reports:

```bash
python3 test/unittest/core/common/force_split/check_page_level_split_coverage.py page_level.info --minimum 90 --output page_level_coverage.json
```

Several files may be supplied when reports were exported per test binary. Branch
regions at identical source positions are merged instead of double-counted. Run
the script against the **same source revision used for the instrumented build**;
build-path prefixes can differ, but source contents and line numbers must match.

The check uses true/false branch outcomes at changed source lines against the
feature baseline, requires at least 90% for each changed source and overall, and
lists unexecuted outcomes. Missing changed files or missing branch records fail
the check rather than disappearing from the denominator. In particular, component
reports alone cannot pass the Router and platform-injection coverage requirements.
Use the uncovered regions to add cases and repeat verification. Whole-function
coverage and unreachable defensive macro branches should also be reviewed in the
native coverage report; changed-line coverage is explicitly a separate metric.
