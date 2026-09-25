# Run MSVC tests across C++14, C++17, C++20, C++23 with dual-mode (default vs fallback)
$standards = @("14", "17", "20", "23")
$modes = @("default", "fallback")
$root = Resolve-Path "$PSScriptRoot\.."
$cmake = "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$msbuild = "C:\Program Files\Microsoft Visual Studio\18\Insiders\MSBuild\Current\Bin\MSBuild.exe"

Remove-Item Env:http_proxy -ErrorAction SilentlyContinue
Remove-Item Env:https_proxy -ErrorAction SilentlyContinue
Remove-Item Env:all_proxy -ErrorAction SilentlyContinue

$results = [ordered]@{}

foreach ($std in $standards) {
    foreach ($mode in $modes) {
        $tag = "MSVC C++$std ($mode)"
        Write-Host "========================================" -ForegroundColor Cyan
        Write-Host " $tag" -ForegroundColor Cyan
        Write-Host "========================================" -ForegroundColor Cyan
        $bdir = "$root\build_msvc_cxx${std}_${mode}"
        if (Test-Path $bdir) { Remove-Item -Recurse -Force $bdir }
        
        $flags = @("-DCMAKE_CXX_STANDARD=$std", "-DCOMPAT_BUILD_BENCHMARKS=OFF")
        if ($mode -eq "fallback") {
            $flags += "-DCOMPAT_FORCE_FALLBACK=ON"
            $flags += "-DCOMPAT_FORCE_SELF_IMPLEMENTATION=1"
        }
        
        & $cmake -B $bdir -S $root -G "Visual Studio 18 2026" -A x64 @flags
        if ($LASTEXITCODE -ne 0) {
            $results[$tag] = "CONFIGURE FAILED"
            continue
        }
        
        & $msbuild "$bdir\compat_test.vcxproj" /p:Configuration=Release /p:Platform=x64
        if ($LASTEXITCODE -ne 0) {
            $results[$tag] = "BUILD FAILED"
            continue
        }
        
        & "$bdir\Release\compat_test.exe"
        if ($LASTEXITCODE -ne 0) {
            $results[$tag] = "TEST FAILED"
        } else {
            $results[$tag] = "PASSED"
        }
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host " MSVC Test Matrix Summary:" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
foreach ($k in $results.Keys) {
    $val = $results[$k]
    $color = if ($val -eq "PASSED") { "Green" } else { "Red" }
    Write-Host "  ${k}: $val" -ForegroundColor $color
}
Write-Host "========================================" -ForegroundColor Green
