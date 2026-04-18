@echo off
setlocal enabledelayedexpansion
:: ╔══════════════════════════════════════════════════════════════════╗
:: ║  AtlasWorkspace — Windows Build System with Progress & Log Mirror   ║
:: ║  All output shown on screen AND mirrored to Logs\build.log     ║
:: ╚══════════════════════════════════════════════════════════════════╝

set "ROOT_DIR=%~dp0.."
set "BUILD_TYPE=%~1"
if "%BUILD_TYPE%"=="" set "BUILD_TYPE=Debug"

set "BUILD_DIR=%ROOT_DIR%\Builds\vs2022"
set "LOG_DIR=%ROOT_DIR%\Logs"
set "LOG_FILE=%LOG_DIR%\build.log"
set "TEST_LOG_FILE=%LOG_DIR%\test.log"
set "RUN_TESTS=0"
set "TARGET="
set "BL_FLAG="

:: Parse arguments
:parse_args
if "%~2"=="" goto :done_args
set "ARG=%~2"
if /I "%ARG%"=="--test" set "RUN_TESTS=1"
if /I "%ARG%"=="--editor" set "TARGET=AtlasWorkspaceEditor"
if /I "%ARG%"=="--game" set "TARGET=AtlasWorkspaceGame"
if /I "%ARG%"=="--server" set "TARGET=AtlasWorkspaceServer"
if /I "%ARG%"=="/bl" set "BL_FLAG=/bl"
shift
goto :parse_args
:done_args

:: Setup directories
if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

:: Start log
echo === AtlasWorkspace Build Log === > "%LOG_FILE%"
echo Started: %DATE% %TIME% >> "%LOG_FILE%"
echo Build Type: %BUILD_TYPE% >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

:: Banner
echo.
echo  ╔══════════════════════════════════════════════════════════╗
echo  ║  AtlasWorkspace Build System v0.1.0 — Windows               ║
echo  ╠══════════════════════════════════════════════════════════╣
echo  ║  Build Type:  %BUILD_TYPE%
echo  ║  Build Dir:   %BUILD_DIR%
echo  ║  Log File:    %LOG_FILE%
if not "%TARGET%"=="" echo  ║  Target:      %TARGET%
if "%RUN_TESTS%"=="1" echo  ║  Run Tests:   Yes
echo  ╚══════════════════════════════════════════════════════════╝
echo.

:: ── Step 1: Configure / Generate Solution ────────────────────────
echo  [*] Step 1/3: Configuring CMake...
echo [STEP] Configuring CMake >> "%LOG_FILE%"

:: Generate VS solution if it doesn't exist
if not exist "%BUILD_DIR%\AtlasWorkspace.sln" (
    echo  Generating Visual Studio solution...
    cmake --preset vs2022 >> "%LOG_FILE%" 2>&1
    if errorlevel 1 (
        echo  [ERROR] CMake configure failed! See %LOG_FILE%
        exit /b 1
    )
)

echo  [OK] Configure complete
echo [INFO] Configure complete >> "%LOG_FILE%"
echo.

:: ── Step 2: Locate MSBuild ───────────────────────────────────────
echo  [*] Step 2/3: Building...
echo [STEP] Building >> "%LOG_FILE%"

:: Try vswhere first
set "MSBUILD="
for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe 2^>nul`) do (
    set "MSBUILD=%%i"
)

:: Fall back to cmake --build
if "%MSBUILD%"=="" (
    echo  Using cmake --build fallback...
    if not "%TARGET%"=="" (
        cmake --build "%BUILD_DIR%" --config %BUILD_TYPE% --target %TARGET% --parallel 2>&1 | findstr /V "^$" >> "%LOG_FILE%"
    ) else (
        cmake --build "%BUILD_DIR%" --config %BUILD_TYPE% --parallel 2>&1 | findstr /V "^$" >> "%LOG_FILE%"
    )
    if errorlevel 1 (
        echo  [ERROR] Build failed! See %LOG_FILE%
        exit /b 1
    )
    goto :build_done
)

:: Build with MSBuild
echo  Using MSBuild: %MSBUILD%
set "MSB_TARGET="
if not "%TARGET%"=="" set "MSB_TARGET=/t:%TARGET%"

"%MSBUILD%" "%BUILD_DIR%\AtlasWorkspace.sln" /p:Configuration=%BUILD_TYPE% /m %MSB_TARGET% %BL_FLAG% /fl /flp:logfile="%LOG_FILE%;append" /v:normal /clp:Summary;ShowTimestamp 2>&1
if errorlevel 1 (
    echo  [ERROR] Build failed! See %LOG_FILE%
    exit /b 1
)

:build_done
echo.
echo  [OK] Build complete
echo [INFO] Build complete >> "%LOG_FILE%"
echo.

:: ── Build Output Summary ─────────────────────────────────────────
echo  ──────────────────────────────────────────────────
echo  Build Output:
echo.

:: List executables
echo  Executables:
if exist "%BUILD_DIR%\bin\%BUILD_TYPE%\AtlasWorkspaceEditor.exe" (
    echo    [OK] AtlasWorkspaceEditor.exe
)
if exist "%BUILD_DIR%\bin\%BUILD_TYPE%\AtlasWorkspaceGame.exe" (
    echo    [OK] AtlasWorkspaceGame.exe
)
if exist "%BUILD_DIR%\bin\%BUILD_TYPE%\AtlasWorkspaceServer.exe" (
    echo    [OK] AtlasWorkspaceServer.exe
)

:: List test executables
if exist "%BUILD_DIR%\bin\%BUILD_TYPE%\Tests" (
    echo.
    echo  Test Executables:
    for %%f in ("%BUILD_DIR%\bin\%BUILD_TYPE%\Tests\NF_*.exe") do (
        echo    [OK] %%~nxf
    )
)

echo.

:: ── Step 3: Tests (optional) ─────────────────────────────────────
if "%RUN_TESTS%"=="1" (
    echo  [*] Step 3/3: Running tests...
    echo [STEP] Running tests >> "%LOG_FILE%"
    ctest --test-dir "%BUILD_DIR%" -C %BUILD_TYPE% --output-on-failure > "%TEST_LOG_FILE%" 2>&1
    type "%TEST_LOG_FILE%"
    echo. >> "%LOG_FILE%"
    type "%TEST_LOG_FILE%" >> "%LOG_FILE%"
    echo  [OK] Tests complete — see %TEST_LOG_FILE%
) else (
    echo  Skipping tests (use --test to run^)
)

echo.

:: ── Atlas Workspace scaffolding ──────────────────────────────────
echo  [*] Scaffolding Atlas Workspace...
if not exist "%ROOT_DIR%\Atlas\Workspace\Sessions" mkdir "%ROOT_DIR%\Atlas\Workspace\Sessions"
if not exist "%ROOT_DIR%\.novaforge\pipeline\changes" mkdir "%ROOT_DIR%\.novaforge\pipeline\changes"
if not exist "%ROOT_DIR%\.novaforge\pipeline\assets" mkdir "%ROOT_DIR%\.novaforge\pipeline\assets"
if not exist "%ROOT_DIR%\.novaforge\pipeline\worlds" mkdir "%ROOT_DIR%\.novaforge\pipeline\worlds"
if not exist "%ROOT_DIR%\.novaforge\pipeline\scripts" mkdir "%ROOT_DIR%\.novaforge\pipeline\scripts"
if not exist "%ROOT_DIR%\.novaforge\pipeline\animations" mkdir "%ROOT_DIR%\.novaforge\pipeline\animations"
if not exist "%ROOT_DIR%\.novaforge\pipeline\sessions" mkdir "%ROOT_DIR%\.novaforge\pipeline\sessions"
echo  [OK] Atlas Workspace ready
echo.

:: ── Summary ──────────────────────────────────────────────────────
echo  ╔══════════════════════════════════════════════════════════╗
echo  ║  Build Complete!                                        ║
echo  ╠══════════════════════════════════════════════════════════╣
echo  ║  Build Type:    %BUILD_TYPE%
echo  ║  Log File:      %LOG_FILE%
echo  ╚══════════════════════════════════════════════════════════╝
echo.

echo Finished: %DATE% %TIME% >> "%LOG_FILE%"
endlocal
