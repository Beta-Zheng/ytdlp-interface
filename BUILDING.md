# Windows build

Install Visual Studio 2022 Desktop development with C++ (v143 and Windows SDK), and CMake. From the repository root, run `./build_windows.ps1` in PowerShell. The script extracts the bundled dependencies, builds Release/x64, runs any test projects present in the current branch, and copies the executable, dependency licenses and SHA256 to `outputs/windows`.

JPEG SIMD is disabled so NASM is not required. Release compiler debug information is disabled to avoid mismatched PDB tools. The color editor member rename avoids a namespace lookup conflict with `widgets` on VS2022. Generated dependencies, intermediate files and outputs are ignored.
