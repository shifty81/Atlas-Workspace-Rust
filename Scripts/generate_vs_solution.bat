@echo off
REM ── AtlasWorkspace — Generate Visual Studio Solution ──────────────────
REM Usage:
REM   generate_vs_solution.bat           (VS 2022 by default)
REM   generate_vs_solution.bat 2019      (VS 2019)
REM
REM Output: Builds/vs20XX/AtlasWorkspace.sln

setlocal enabledelayedexpansion

set VS_VERSION=%1
if "%VS_VERSION%"=="" set VS_VERSION=2022

if "%VS_VERSION%"=="2022" (
    set PRESET=vs2022
    set GENERATOR=Visual Studio 17 2022
) else if "%VS_VERSION%"=="2019" (
    set PRESET=vs2019
    set GENERATOR=Visual Studio 16 2019
) else (
    echo Error: Unsupported Visual Studio version: %VS_VERSION%
    echo Supported versions: 2019, 2022
    exit /b 1
)

echo.
echo ══════════════════════════════════════════════════════════
echo   AtlasWorkspace — Generating VS %VS_VERSION% Solution
echo ══════════════════════════════════════════════════════════
echo.

pushd "%~dp0.."

cmake --preset %PRESET%
if errorlevel 1 (
    echo.
    echo ERROR: CMake configure failed.
    popd
    exit /b 1
)

echo.
echo ══════════════════════════════════════════════════════════
echo   Solution generated successfully!
echo   Open: Builds\%PRESET%\AtlasWorkspace.sln
echo ══════════════════════════════════════════════════════════
echo.

popd
endlocal
