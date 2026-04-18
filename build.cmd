@echo off
setlocal enabledelayedexpansion
:: ---------------------------------------------------------------------------
:: build.cmd — Build AtlasWorkspace with MSBuild (VS 2022/2019).
::
:: Can be run from ANY Command Prompt — MSBuild is auto-located via
:: vswhere.exe if it is not already on PATH.
::
:: Usage:
::   build.cmd                Build Debug (default)
::   build.cmd Release        Build Release
::   build.cmd Debug /bl      Build Debug with MSBuild binary logging (.binlog)
::   build.cmd Rebuild        Rebuild Debug
::   build.cmd Clean          Clean Debug artifacts
::
:: Output is displayed on screen AND written to Saved\Logs\build.log.
::
:: The generated .binlog can be opened with MSBuild Structured Log Viewer:
::   https://msbuildlog.com/
::
:: If you haven't generated the .sln yet, run first:
::   cmake --preset vs2022
::   (or: Scripts\generate_vs_solution.bat)
:: ---------------------------------------------------------------------------

:: Ensure we are in the repo root.
cd /d "%~dp0"

:: Create log directory if needed -------------------------------------------
if not exist "Saved\Logs" mkdir "Saved\Logs"
set "LOGFILE=Saved\Logs\build.log"

:: Start log ----------------------------------------------------------------
echo ========================================================== > "%LOGFILE%"
echo  AtlasWorkspace build started: %DATE% %TIME%                   >> "%LOGFILE%"
echo ========================================================== >> "%LOGFILE%"

:: Detect VS MSBuild --------------------------------------------------------
:: Prefer msbuild already on PATH (Developer Command Prompt). Otherwise
:: fall back to vswhere.exe to locate the VS 2022 (or 2019) C++ MSBuild.
set "MSBUILD=msbuild.exe"
where msbuild.exe >nul 2>&1
if errorlevel 1 (
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if not exist "!VSWHERE!" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "!VSWHERE!" (
        for /f "usebackq delims=" %%i in (
            `"!VSWHERE!" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`
        ) do set "MSBUILD=%%i"
    )
    "!MSBUILD!" /version >nul 2>&1
    if errorlevel 1 (
        echo.
        echo [ERROR] MSBuild.exe not found.
        echo         Make sure Visual Studio 2022 ^(or 2019^) with the
        echo         "Desktop development with C++" workload is installed,
        echo         OR open a "Developer Command Prompt for VS 2022" and retry.
        echo         Do NOT use "dotnet build" -- the .NET SDK MSBuild lacks C++ targets.
        echo.
        echo [ERROR] MSBuild.exe not found. >> "%LOGFILE%"
        echo Build log saved to: %LOGFILE%
        pause
        exit /b 1
    )
    echo [INFO] Located MSBuild via vswhere: !MSBUILD!
)

:: Parse arguments -----------------------------------------------------------
set CONFIG=Debug
set TARGET=Build
set EXTRA_ARGS=

:parse_args
if "%~1"=="" goto done_args
if /i "%~1"=="Debug"   ( set CONFIG=Debug&   shift& goto parse_args )
if /i "%~1"=="Release" ( set CONFIG=Release& shift& goto parse_args )
if /i "%~1"=="Rebuild" ( set TARGET=Rebuild& shift& goto parse_args )
if /i "%~1"=="Clean"   ( set TARGET=Clean&   shift& goto parse_args )
set EXTRA_ARGS=!EXTRA_ARGS! %1
shift
goto parse_args
:done_args

:: Ensure the .sln exists ---------------------------------------------------
set "SLN_PATH=Builds\vs2022\AtlasWorkspace.sln"
if not exist "%SLN_PATH%" (
    echo.
    echo [INFO] Solution not found at %SLN_PATH%.
    echo [INFO] Running: cmake --preset vs2022
    echo.
    cmake --preset vs2022
    if errorlevel 1 (
        echo [ERROR] CMake configure failed.
        echo Build log saved to: %LOGFILE%
        pause
        exit /b 1
    )
)

:: Build ---------------------------------------------------------------------
echo.
echo === AtlasWorkspace %CONFIG% %TARGET% ===
echo.
echo === AtlasWorkspace %CONFIG% %TARGET% === >> "%LOGFILE%"
echo. >> "%LOGFILE%"

"!MSBUILD!" "%SLN_PATH%" /t:%TARGET% /p:Configuration=%CONFIG% /p:Platform=x64 /m %EXTRA_ARGS%
set BUILD_EXIT=!ERRORLEVEL!

:: Also append to log (without blocking the console output above).
echo Build exit code: !BUILD_EXIT! >> "%LOGFILE%"
echo Build completed: %DATE% %TIME% >> "%LOGFILE%"

if !BUILD_EXIT! neq 0 (
    echo.
    echo [FAILED] Build returned error code !BUILD_EXIT!.
    echo.
    echo Tip: re-run with /bl to capture a binary log for debugging:
    echo   build.cmd %CONFIG% /bl
    echo Then open the .binlog with https://msbuildlog.com/
    echo.
    echo [FAILED] Build returned error code !BUILD_EXIT! >> "%LOGFILE%"
    echo Build log saved to: %LOGFILE%
    pause
    exit /b !BUILD_EXIT!
)

echo.
echo [OK] AtlasWorkspace %CONFIG% %TARGET% succeeded.
echo.
echo [OK] AtlasWorkspace %CONFIG% %TARGET% succeeded. >> "%LOGFILE%"
echo Build log saved to: %LOGFILE%
pause
