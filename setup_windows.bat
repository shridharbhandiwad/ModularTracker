@echo off
setlocal enabledelayedexpansion

echo ==========================================
echo   Radar Tracking System - Windows Setup
echo ==========================================

REM Check if running from correct directory
if not exist "CMakeLists.txt" (
    echo ERROR: This script must be run from the project root directory
    echo Make sure CMakeLists.txt is in the current directory
    pause
    exit /b 1
)

REM Check for required tools
echo Checking prerequisites...

where git >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Git is not installed or not in PATH
    echo Please install Git from https://git-scm.com/
    pause
    exit /b 1
)

where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake is not installed or not in PATH
    echo Please install CMake from https://cmake.org/
    pause
    exit /b 1
)

where cl >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Visual Studio compiler (cl.exe) not found
    echo Please install Visual Studio with C++ development tools
    echo Or run this from a Visual Studio Developer Command Prompt
    pause
    exit /b 1
)

echo Prerequisites check passed!

REM Install vcpkg if not present
echo.
echo ==========================================
echo   Installing vcpkg Package Manager
echo ==========================================

if not exist "vcpkg\" (
    echo Cloning vcpkg...
    git clone https://github.com/Microsoft/vcpkg.git
    if !ERRORLEVEL! NEQ 0 (
        echo ERROR: Failed to clone vcpkg
        pause
        exit /b 1
    )
) else (
    echo vcpkg directory already exists
)

if not exist "vcpkg\vcpkg.exe" (
    echo Bootstrapping vcpkg...
    cd vcpkg
    call bootstrap-vcpkg.bat
    if !ERRORLEVEL! NEQ 0 (
        echo ERROR: Failed to bootstrap vcpkg
        pause
        exit /b 1
    )
    cd ..
) else (
    echo vcpkg is already bootstrapped
)

REM Install dependencies
echo.
echo ==========================================
echo   Installing Dependencies
echo ==========================================

echo Installing required packages (this may take several minutes)...

set "PACKAGES=yaml-cpp spdlog eigen3 boost gtest"

for %%p in (%PACKAGES%) do (
    echo.
    echo Installing %%p:x64-windows...
    vcpkg\vcpkg.exe install %%p:x64-windows
    if !ERRORLEVEL! NEQ 0 (
        echo ERROR: Failed to install %%p
        pause
        exit /b 1
    )
)

echo.
echo Integrating vcpkg with Visual Studio...
vcpkg\vcpkg.exe integrate install

REM Create directories
echo.
echo ==========================================
echo   Creating Directories
echo ==========================================

if not exist "logs" mkdir logs
if not exist "logs\data" mkdir logs\data
if not exist "scenarios" mkdir scenarios
if not exist "scripts\windows" mkdir scripts\windows

echo Directories created successfully

REM Build the project
echo.
echo ==========================================
echo   Building the Project
echo ==========================================

if not exist "build" mkdir build
cd build

echo Configuring with CMake...
cmake .. -G "Visual Studio 16 2019" -A x64 ^
         -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake ^
         -DCMAKE_BUILD_TYPE=Release ^
         -DBUILD_TESTING=ON ^
         -DBUILD_SIMULATION=ON ^
         -DBUILD_BENCHMARKS=ON

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed
    cd ..
    pause
    exit /b 1
)

echo Building the solution...
cmake --build . --config Release --parallel

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    cd ..
    pause
    exit /b 1
)

echo Running tests...
ctest -C Release --output-on-failure

cd ..

REM Check build results
echo.
echo ==========================================
echo   Verifying Build Results
echo ==========================================

set "SUCCESS=true"

if exist "build\Release\radar_simulator.exe" (
    echo [OK] radar_simulator.exe built successfully
) else (
    echo [ERROR] radar_simulator.exe not found
    set "SUCCESS=false"
)

if exist "build\Release\radar_tracking_system.exe" (
    echo [OK] radar_tracking_system.exe built successfully
) else (
    echo [ERROR] radar_tracking_system.exe not found
    set "SUCCESS=false"
)

if "%SUCCESS%"=="false" (
    echo.
    echo Some executables were not built successfully
    echo Check the build output for errors
    pause
    exit /b 1
)

REM Setup Windows Firewall rules (optional)
echo.
echo ==========================================
echo   Windows Firewall Configuration
echo ==========================================

set /p "SETUP_FIREWALL=Configure Windows Firewall rules? (y/n): "
if /i "%SETUP_FIREWALL%"=="y" (
    echo Adding firewall rules...
    
    netsh advfirewall firewall show rule name="Radar Tracking UDP" >nul 2>&1
    if %ERRORLEVEL% NEQ 0 (
        netsh advfirewall firewall add rule name="Radar Tracking UDP" dir=in action=allow protocol=UDP localport=8080
        echo Added UDP rule for port 8080
    )
    
    netsh advfirewall firewall show rule name="Radar Tracking TCP" >nul 2>&1
    if %ERRORLEVEL% NEQ 0 (
        netsh advfirewall firewall add rule name="Radar Tracking TCP" dir=in action=allow protocol=TCP localport=9090
        echo Added TCP rule for port 9090
    )
    
    echo Firewall rules configured successfully
)

REM Final summary
echo.
echo ==========================================
echo   Setup Complete!
echo ==========================================
echo.
echo The Radar Tracking System has been successfully set up!
echo.
echo Build Output:
echo   Executables: build\Release\
echo   Libraries: build\Release\
echo.
echo Quick Start:
echo   1. Test the simulator:
echo      build\Release\radar_simulator.exe --generate single --mode console
echo.
echo   2. Validate configuration:
echo      build\Release\radar_tracking_system.exe --validate --config config\system_config.yaml
echo.
echo   3. Run the system:
echo      build\Release\radar_tracking_system.exe --config config\system_config.yaml
echo.
echo   4. Install as Windows service (optional):
echo      Run scripts\windows\install_service.bat as Administrator
echo.
echo Development:
echo   - Open radar_tracking_system.sln in Visual Studio
echo   - Use build_windows.bat for command-line builds
echo   - See README_WINDOWS.md for detailed documentation
echo.

pause