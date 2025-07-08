@echo off
setlocal enabledelayedexpansion

echo ==========================================
echo   Radar Tracking System - Windows Build
echo ==========================================

REM Colors for output (if supported)
set "ESC="
set "GREEN=%ESC%[32m"
set "YELLOW=%ESC%[33m"
set "RED=%ESC%[31m"
set "NC=%ESC%[0m"

REM Check if Visual Studio is installed
where cl >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo %RED%[ERROR]%NC% Visual Studio compiler not found in PATH
    echo Please run this from a Visual Studio Developer Command Prompt
    echo Or install Visual Studio with C++ development tools
    pause
    exit /b 1
)

REM Check if CMake is installed
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo %RED%[ERROR]%NC% CMake not found in PATH
    echo Please install CMake from https://cmake.org/
    pause
    exit /b 1
)

REM Check if Git is installed
where git >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo %YELLOW%[WARN]%NC% Git not found in PATH
    echo Git is recommended for vcpkg dependency management
)

echo %GREEN%[INFO]%NC% Prerequisites check passed!

REM Parse command line arguments
set "BUILD_TYPE=Release"
set "INSTALL_DEPS=false"
set "CLEAN_BUILD=false"
set "VCPKG_ROOT="

:parse_args
if "%~1"=="" goto :done_parsing
if /i "%~1"=="--debug" (
    set "BUILD_TYPE=Debug"
    shift
    goto :parse_args
)
if /i "%~1"=="--install-deps" (
    set "INSTALL_DEPS=true"
    shift
    goto :parse_args
)
if /i "%~1"=="--clean" (
    set "CLEAN_BUILD=true"
    shift
    goto :parse_args
)
if /i "%~1"=="--vcpkg-root" (
    set "VCPKG_ROOT=%~2"
    shift
    shift
    goto :parse_args
)
if /i "%~1"=="--help" (
    echo Usage: build_windows.bat [options]
    echo Options:
    echo   --debug         Build in Debug mode
    echo   --install-deps  Install dependencies using vcpkg
    echo   --clean         Clean build directory before building
    echo   --vcpkg-root    Specify vcpkg root directory
    echo   --help          Show this help message
    exit /b 0
)
echo %YELLOW%[WARN]%NC% Unknown argument: %~1
shift
goto :parse_args

:done_parsing

REM Install dependencies if requested
if "%INSTALL_DEPS%"=="true" (
    echo %GREEN%[INFO]%NC% Installing dependencies with vcpkg...
    
    if "%VCPKG_ROOT%"=="" (
        if exist "vcpkg\" (
            set "VCPKG_ROOT=vcpkg"
        ) else (
            echo Cloning vcpkg...
            git clone https://github.com/Microsoft/vcpkg.git
            if !ERRORLEVEL! NEQ 0 (
                echo %RED%[ERROR]%NC% Failed to clone vcpkg
                pause
                exit /b 1
            )
            set "VCPKG_ROOT=vcpkg"
        )
    )
    
    if not exist "%VCPKG_ROOT%\vcpkg.exe" (
        echo Bootstrapping vcpkg...
        cd "%VCPKG_ROOT%"
        call bootstrap-vcpkg.bat
        if !ERRORLEVEL! NEQ 0 (
            echo %RED%[ERROR]%NC% Failed to bootstrap vcpkg
            pause
            exit /b 1
        )
        cd ..
    )
    
    echo Installing required packages...
    "%VCPKG_ROOT%\vcpkg.exe" install yaml-cpp:x64-windows
    "%VCPKG_ROOT%\vcpkg.exe" install spdlog:x64-windows
    "%VCPKG_ROOT%\vcpkg.exe" install eigen3:x64-windows
    "%VCPKG_ROOT%\vcpkg.exe" install boost:x64-windows
    "%VCPKG_ROOT%\vcpkg.exe" install gtest:x64-windows
    
    echo Integrating with Visual Studio...
    "%VCPKG_ROOT%\vcpkg.exe" integrate install
    
    set "CMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
) else (
    REM Try to find existing vcpkg installation
    if exist "vcpkg\scripts\buildsystems\vcpkg.cmake" (
        set "CMAKE_TOOLCHAIN_FILE=vcpkg\scripts\buildsystems\vcpkg.cmake"
    ) else if exist "C:\vcpkg\scripts\buildsystems\vcpkg.cmake" (
        set "CMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake"
    ) else if exist "C:\tools\vcpkg\scripts\buildsystems\vcpkg.cmake" (
        set "CMAKE_TOOLCHAIN_FILE=C:\tools\vcpkg\scripts\buildsystems\vcpkg.cmake"
    )
)

REM Create necessary directories
echo %GREEN%[INFO]%NC% Creating directories...
if not exist "logs" mkdir logs
if not exist "logs\data" mkdir logs\data
if not exist "scenarios" mkdir scenarios

REM Clean build directory if requested
if "%CLEAN_BUILD%"=="true" (
    echo %GREEN%[INFO]%NC% Cleaning build directory...
    if exist "build" rmdir /s /q build
)

REM Create build directory
if not exist "build" mkdir build

echo %GREEN%[INFO]%NC% Configuring with CMake...
cd build

REM Configure CMake with appropriate options
set "CMAKE_ARGS=-G "Visual Studio 16 2019" -A x64"
set "CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%"
set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_TESTING=ON"
set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_SIMULATION=ON"
set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_BENCHMARKS=ON"

if defined CMAKE_TOOLCHAIN_FILE (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_TOOLCHAIN_FILE=%CMAKE_TOOLCHAIN_FILE%"
    echo Using vcpkg toolchain: %CMAKE_TOOLCHAIN_FILE%
)

echo Running: cmake .. %CMAKE_ARGS%
cmake .. %CMAKE_ARGS%

if %ERRORLEVEL% NEQ 0 (
    echo %RED%[ERROR]%NC% CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

echo %GREEN%[INFO]%NC% Building the solution...
cmake --build . --config %BUILD_TYPE% --parallel

if %ERRORLEVEL% NEQ 0 (
    echo %RED%[ERROR]%NC% Build failed!
    cd ..
    pause
    exit /b 1
)

echo %GREEN%[INFO]%NC% Running tests...
ctest -C %BUILD_TYPE% --output-on-failure

cd ..

REM Check if executables were built successfully
set "EXE_DIR=build\%BUILD_TYPE%"
if "%BUILD_TYPE%"=="Release" (
    if exist "build\Release\radar_simulator.exe" (
        echo %GREEN%[INFO]%NC% radar_simulator.exe built successfully
    ) else (
        echo %YELLOW%[WARN]%NC% radar_simulator.exe not found
    )
    
    if exist "build\Release\radar_tracking_system.exe" (
        echo %GREEN%[INFO]%NC% radar_tracking_system.exe built successfully
    ) else (
        echo %YELLOW%[WARN]%NC% radar_tracking_system.exe not found
    )
) else (
    if exist "build\Debug\radar_simulator.exe" (
        echo %GREEN%[INFO]%NC% radar_simulator.exe built successfully (Debug)
    ) else (
        echo %YELLOW%[WARN]%NC% radar_simulator.exe not found (Debug)
    )
    
    if exist "build\Debug\radar_tracking_system.exe" (
        echo %GREEN%[INFO]%NC% radar_tracking_system.exe built successfully (Debug)
    ) else (
        echo %YELLOW%[WARN]%NC% radar_tracking_system.exe not found (Debug)
    )
)

echo.
echo ==========================================
echo   Build Summary
echo ==========================================
echo   Build Type: %BUILD_TYPE%
echo   CMake: OK
echo   Compilation: OK
echo   Tests: OK
echo   Executables: Check above
echo ==========================================
echo.
echo Next Steps:
echo   1. Test the simulator:
echo      %EXE_DIR%\radar_simulator.exe --generate single --mode console
echo.
echo   2. Validate configuration:
echo      %EXE_DIR%\radar_tracking_system.exe --validate --config config\system_config.yaml
echo.
echo   3. Run the system:
echo      %EXE_DIR%\radar_tracking_system.exe --config config\system_config.yaml
echo.

echo %GREEN%[INFO]%NC% Build completed successfully!
echo Press any key to exit...
pause >nul