$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vsPath = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) { throw '需要安装 Visual Studio 2022 的 C++ 桌面开发组件。' }
$msbuild = Join-Path $vsPath 'MSBuild\Current\Bin\MSBuild.exe'
New-Item -ItemType Directory -Force work,outputs/windows | Out-Null

function Invoke-Checked([string]$Program, [string[]]$Arguments, [string]$Log) {
    & $Program @Arguments *> $Log
    if ($LASTEXITCODE -ne 0) {
        Get-Content -LiteralPath $Log -Tail 35
        throw "构建失败，日志：$Log"
    }
}

if (-not (Test-Path nana/build/vc2022/nana.sln)) {
    Write-Host '正在解压附带的依赖源码...'
    Invoke-Checked tar @('-xf', 'ytdlp-interface dependencies.7z') 'work/extract.log'
}
$common = @('/m:4', '/p:Configuration=Release', '/p:Platform=x64', '/p:PlatformToolset=v143', '/v:minimal', '/nologo')
foreach ($entry in @(
    @('nana/build/vc2022/nana.sln', 'nana'),
    @('bit7z/bit7z.sln', 'bit7z'),
    @('libpng/libpng.sln', 'libpng')
)) {
    Write-Host "正在构建 $($entry[1])..."
    Invoke-Checked $msbuild (@($entry[0]) + $common) "work/build-$($entry[1]).log"
}
Write-Host '正在构建 libjpeg-turbo...'
Invoke-Checked cmake @('-S', 'libjpeg-turbo-3.1.2', '-B', 'libjpeg-turbo-3.1.2/out/build/x64-Release', '-G', 'Visual Studio 17 2022', '-A', 'x64', '-DENABLE_SHARED=OFF', '-DWITH_CRT_DLL=OFF', '-DWITH_SIMD=OFF') 'work/configure-jpeg.log'
Invoke-Checked cmake @('--build', 'libjpeg-turbo-3.1.2/out/build/x64-Release', '--config', 'Release', '--target', 'turbojpeg-static', '--parallel', '4') 'work/build-jpeg.log'
Write-Host '正在构建 Windows 界面程序...'
Invoke-Checked $msbuild (@('ytdlp-interface/ytdlp-interface.sln') + $common) 'work/build-app.log'

# Run the tests present in the checked-out feature branch.
foreach ($testName in @('i18n_smoke', 'runtime_regression')) {
    $projectPath = "tests/$testName.vcxproj"
    if (Test-Path $projectPath) {
        Invoke-Checked $msbuild (@($projectPath) + $common) "work/build-$testName.log"
        Push-Location work/tests
        try { Invoke-Checked (Join-Path $PSScriptRoot "work/tests/$testName.exe") @() (Join-Path $PSScriptRoot "work/$testName-results.log") }
        finally { Pop-Location }
    }
}
Copy-Item -LiteralPath 'ytdlp-interface/x64/Release/ytdlp-interface.exe' -Destination 'outputs/windows/ytdlp-interface.exe' -Force
Copy-Item -LiteralPath LICENSE -Destination outputs/windows -Force
New-Item -ItemType Directory -Force outputs/windows/licenses | Out-Null
Copy-Item nana/LICENSE outputs/windows/licenses/Nana-LICENSE.txt -Force
Copy-Item bit7z/LICENSE outputs/windows/licenses/bit7z-LICENSE.txt -Force
Copy-Item libjpeg-turbo-3.1.2/LICENSE.md outputs/windows/licenses/libjpeg-turbo-LICENSE.md -Force
Copy-Item libjpeg-turbo-3.1.2/README.ijg outputs/windows/licenses/libjpeg-README.ijg -Force
Copy-Item libpng/packages/libpng.static.1.6.37/build/native/png/png.h outputs/windows/licenses/libpng-license-header.h -Force
Copy-Item libpng/packages/zlib.static.1.2.5/build/native/include/zlib.h outputs/windows/licenses/zlib-license-header.h -Force
Get-FileHash -LiteralPath outputs/windows/ytdlp-interface.exe -Algorithm SHA256 |
    ForEach-Object { $_.Hash + '  ytdlp-interface.exe' } |
    Set-Content -LiteralPath outputs/windows/SHA256.txt -Encoding ascii
Write-Host '构建完成：outputs/windows/ytdlp-interface.exe'
