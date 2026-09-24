# Run MSVC tests across C++14, C++17, C++20
$standards = @("14", "17", "20")
$root = "D:\program\C++\CPP-Compat"
$cmake = "C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$msbuild = "C:\Program Files\Microsoft Visual Studio\18\Insiders\MSBuild\Current\Bin\MSBuild.exe"

Remove-Item Env:http_proxy -ErrorAction SilentlyContinue
Remove-Item Env:https_proxy -ErrorAction SilentlyContinue
Remove-Item Env:all_proxy -ErrorAction SilentlyContinue

$results = @{}

foreach ($std in $standards) {
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host " MSVC: Testing C++$std" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    $bdir = "$root\build_msvc_cxx$std"
    if (Test-Path $bdir) { Remove-Item -Recurse -Force $bdir }
    
    $stdArg = "-DCMAKE_CXX_STANDARD=$std"
    & $cmake -B $bdir -S $root -G "Visual Studio 18 2026" -A x64 $stdArg -DCOMPAT_FORCE_SELF_IMPLEMENTATION=1 -DCOMPAT_BUILD_BENCHMARKS=OFF
    if ($LASTEXITCODE -ne 0) {
        $results["MSVC C++$std Configure"] = "FAILED"
        continue
    }
    
    & $msbuild "$bdir\compat_test.vcxproj" /p:Configuration=Debug /p:Platform=x64
    if ($LASTEXITCODE -ne 0) {
        $results["MSVC C++$std Build"] = "FAILED"
        continue
    }
    
    & "$bdir\Debug\compat_test.exe"
    if ($LASTEXITCODE -ne 0) {
        $results["MSVC C++$std Test"] = "FAILED"
    } else {
        $results["MSVC C++$std"] = "PASSED"
    }
}

Write-Host "========================================" -ForegroundColor Green
Write-Host " MSVC Summary:" -ForegroundColor Green
foreach ($k in $results.Keys) {
    $val = $results[$k]
    Write-Host "  ${k}: $val"
}
Write-Host "========================================" -ForegroundColor Green
