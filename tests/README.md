# Runtime regression checks

Build the production application as Release/x64 first, then build `runtime_regression.vcxproj` with MSBuild (v143) and run `../work/tests/runtime_regression.exe` from `../work/tests`.

The test project links the production objects except `main.obj`. Use a clean application build when switching branches. Tests cover nonzero exits (including 259 and a crash status), launch failure, cancellation, partial and legacy settings, invalid values, Unicode paths, and backup preservation. The startup recovery dialog and network downloads are not automated here.

The separate `build/windows-vs2022` branch supplies VS2022 compatibility changes and `build_windows.ps1`, which builds the application and runs this test automatically when both changes are combined. This runtime branch does not depend on the translation module.
