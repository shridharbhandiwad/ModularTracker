# C++ Radar Tracking System - Windows Implementation

This is a comprehensive, production-ready radar tracking system implemented in modern C++ for Windows platforms. The system supports both Track While Scan (TWS) and dedicated beam request tracking modes for defense applications.

## 🚀 Quick Start for Windows

### Prerequisites

#### Option 1: Visual Studio (Recommended)
- **Visual Studio 2019 or 2022** with C++ development tools
- **CMake** (included with Visual Studio or download from https://cmake.org/)
- **Git** for version control

#### Option 2: MinGW-w64 + MSYS2
- **MSYS2** from https://www.msys2.org/
- **MinGW-w64** toolchain

#### Option 3: vcpkg Package Manager
- **vcpkg** for dependency management (recommended)

### Installing Dependencies

#### Using vcpkg (Recommended)

```cmd
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install required packages
.\vcpkg install yaml-cpp:x64-windows
.\vcpkg install spdlog:x64-windows
.\vcpkg install eigen3:x64-windows
.\vcpkg install boost:x64-windows
.\vcpkg install gtest:x64-windows

# Integrate with Visual Studio
.\vcpkg integrate install
```

#### Using MSYS2/MinGW-w64

```bash
# Open MSYS2 terminal
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-yaml-cpp
pacman -S mingw-w64-x86_64-spdlog
pacman -S mingw-w64-x86_64-eigen3
pacman -S mingw-w64-x86_64-boost
pacman -S mingw-w64-x86_64-gtest
```

### Build Instructions

#### Visual Studio with vcpkg

```cmd
# Clone or navigate to project directory
cd C:\path\to\radar-tracking-system

# Create build directory
mkdir build
cd build

# Configure with CMake (vcpkg integration)
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake ^
         -DCMAKE_BUILD_TYPE=Release ^
         -DBUILD_TESTING=ON ^
         -DBUILD_SIMULATION=ON ^
         -A x64

# Build the solution
cmake --build . --config Release

# Run tests (optional)
ctest -C Release --output-on-failure
```

#### MinGW-w64 with MSYS2

```bash
# Open MSYS2 MinGW 64-bit terminal
cd /c/path/to/radar-tracking-system

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -G "MinGW Makefiles" \
         -DCMAKE_BUILD_TYPE=Release \
         -DBUILD_TESTING=ON \
         -DBUILD_SIMULATION=ON

# Build the system
mingw32-make -j$(nproc)

# Run tests (optional)
ctest --output-on-failure
```

### First Run

```cmd
# Create necessary directories
mkdir logs
mkdir logs\data
mkdir scenarios

# Run the radar simulator to generate test data
.\build\Release\radar_simulator.exe --generate single --mode console

# In another command prompt, run the tracking system
.\build\Release\radar_tracking_system.exe --config config\system_config.yaml
```

## 🔧 Windows-Specific Configuration

### Visual Studio Integration

The project includes Visual Studio solution files when built with CMake:

```cmd
# Generate Visual Studio solution
cmake .. -G "Visual Studio 16 2019" -A x64 ^
         -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake

# Open in Visual Studio
start radar_tracking_system.sln
```

### Windows Service Installation

To run as a Windows service:

```cmd
# Install as service (run as administrator)
sc create RadarTrackingService binPath= "C:\path\to\radar_tracking_system.exe --service" ^
   start= auto DisplayName= "Radar Tracking Service"

# Start the service
sc start RadarTrackingService

# Stop the service
sc stop RadarTrackingService

# Remove the service
sc delete RadarTrackingService
```

### Windows-Specific Features

#### Performance Monitoring
- Integration with Windows Performance Toolkit (WPT)
- Event Tracing for Windows (ETW) support
- Windows Performance Counters

#### Security
- Windows Access Control Lists (ACL) support
- Windows Credential Manager integration
- Service account configuration

## 📁 Windows Project Structure

```
radar_tracking_system\
├── CMakeLists.txt                  # Cross-platform build config
├── README_WINDOWS.md               # This file
├── build_windows.bat               # Windows build script
├── install_dependencies.bat        # Dependency installation script
├── config\                         # Configuration files
│   ├── system_config.yaml
│   └── algorithms\
├── include\                        # Header files
├── src\                           # Source files
├── tools\                         # Development tools
│   └── simulator\
├── scenarios\                      # Test scenarios
├── scripts\windows\                # Windows-specific scripts
│   ├── install_service.bat
│   ├── uninstall_service.bat
│   └── setup_environment.bat
└── logs\                          # Runtime logs
```

## 🎮 Radar Simulator for Windows

### Command Line Usage

```cmd
# Generate single target scenario
radar_simulator.exe --generate single --mode console

# Multi-target scenario saving to CSV
radar_simulator.exe --generate multi --targets 10 --mode file --output test_data.csv

# Load custom scenario
radar_simulator.exe --scenario scenarios\crossing_targets.yaml --mode network --port 8080

# List available scenarios
radar_simulator.exe --list
```

### Windows GUI Wrapper (Optional)

Create a simple GUI wrapper using:
- **Qt** for cross-platform GUI
- **Windows Forms** (.NET Framework)
- **WPF** (.NET Core/5+)

## ⚙️ Windows Configuration

### Registry Settings

The system can store configuration in Windows Registry:

```
HKEY_LOCAL_MACHINE\SOFTWARE\RadarTrackingSystem\
├── InstallPath
├── ConfigPath
├── LogPath
└── ServiceMode
```

### Environment Variables

```cmd
# Set environment variables
set RADAR_CONFIG_PATH=C:\RadarTracking\config
set RADAR_LOG_PATH=C:\RadarTracking\logs
set RADAR_SCENARIO_PATH=C:\RadarTracking\scenarios
```

### Windows Firewall Configuration

```cmd
# Allow through Windows Firewall (run as administrator)
netsh advfirewall firewall add rule name="Radar Tracking UDP" dir=in action=allow protocol=UDP localport=8080
netsh advfirewall firewall add rule name="Radar Tracking TCP" dir=in action=allow protocol=TCP localport=9090
```

## 🚦 Running the System on Windows

### Interactive Mode

```cmd
# Run with default configuration
radar_tracking_system.exe

# Specify custom configuration
radar_tracking_system.exe --config my_config.yaml

# Run with debug output
radar_tracking_system.exe --log-level DEBUG
```

### Service Mode

```cmd
# Run as Windows service
radar_tracking_system.exe --service

# Test service mode in console
radar_tracking_system.exe --service --console
```

### Command Line Options

```
Options:
  -h, --help              Show help message
  -c, --config FILE       Configuration file path
  -l, --log-level LEVEL   Log level (TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL)
  -s, --service           Run as Windows service
  -v, --validate          Validate configuration and exit
  --console               Show console output in service mode
  --version               Show version information
```

## 📊 Windows Testing and Debugging

### Visual Studio Debugging

1. Set `radar_tracking_system` as startup project
2. Set command line arguments in project properties
3. Use Visual Studio debugger for step-through debugging

### Performance Analysis

```cmd
# Using Windows Performance Toolkit
wpr -start perf.wprp
# Run application
wpr -stop perf.etl

# Analyze with Windows Performance Analyzer (WPA)
wpa perf.etl
```

### Memory Analysis

```cmd
# Using Application Verifier
appverif -enable Heaps -for radar_tracking_system.exe

# Using Visual Studio Diagnostic Tools
devenv radar_tracking_system.sln /DebugExe radar_tracking_system.exe
```

## 🔧 Windows Build Automation

### Batch Scripts

Create automated build scripts:

```cmd
# build_release.bat
@echo off
mkdir build 2>nul
cd build
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cd ..
echo Build completed!
```

### PowerShell Scripts

```powershell
# Build.ps1
param(
    [string]$Configuration = "Release",
    [switch]$Clean
)

if ($Clean) {
    Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
}

New-Item -ItemType Directory -Force -Path build
Set-Location build

cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config $Configuration

Set-Location ..
Write-Host "Build completed successfully!" -ForegroundColor Green
```

## 🐛 Windows Troubleshooting

### Common Issues

1. **Missing DLL Errors**:
   ```cmd
   # Copy required DLLs to executable directory
   copy C:\vcpkg\installed\x64-windows\bin\*.dll build\Release\
   ```

2. **Path Issues**:
   ```cmd
   # Use full paths in Windows
   radar_tracking_system.exe --config "C:\full\path\to\config.yaml"
   ```

3. **Permission Errors**:
   ```cmd
   # Run as administrator for service operations
   runas /user:Administrator cmd
   ```

### Debug Commands

```cmd
# Check DLL dependencies
dumpbin /dependents radar_tracking_system.exe

# Monitor file/registry access
procmon.exe

# Network debugging
netstat -an | findstr 8080

# Event logs
wevtutil qe System /c:10 /rd:true /f:text
```

## 🔌 Windows Development Environment

### Recommended IDEs

1. **Visual Studio 2022** (Primary recommendation)
   - Full IntelliSense support
   - Integrated debugger
   - CMake integration
   - Git integration

2. **Visual Studio Code**
   - C++ extension pack
   - CMake tools extension
   - Windows Subsystem for Linux (WSL) support

3. **CLion** (JetBrains)
   - Cross-platform IDE
   - Excellent CMake support
   - Advanced debugging features

### Package Managers

1. **vcpkg** (Microsoft - Recommended)
   - Official Microsoft package manager
   - Excellent Visual Studio integration
   - Wide library support

2. **Conan**
   - Cross-platform package manager
   - Advanced dependency resolution

3. **Chocolatey**
   - Windows package manager for tools
   - Easy installation of development tools

## 📚 Windows-Specific Documentation

### MSDN Resources
- Windows SDK documentation
- Visual C++ documentation
- Windows Service development guide

### Performance Guidelines
- Windows performance best practices
- Memory management on Windows
- Thread synchronization patterns

## 🔒 Windows Security

### Code Signing
```cmd
# Sign the executable (requires certificate)
signtool sign /f certificate.pfx /p password /t http://timestamp.digicert.com radar_tracking_system.exe
```

### Security Features
- DEP (Data Execution Prevention)
- ASLR (Address Space Layout Randomization)
- Control Flow Guard
- Windows Defender integration

---

## 🚀 Quick Setup Script

Save this as `setup_windows.bat`:

```cmd
@echo off
echo Setting up Radar Tracking System for Windows...

echo Installing vcpkg...
if not exist vcpkg (
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    call bootstrap-vcpkg.bat
    cd ..
)

echo Installing dependencies...
cd vcpkg
.\vcpkg install yaml-cpp:x64-windows spdlog:x64-windows eigen3:x64-windows boost:x64-windows gtest:x64-windows
.\vcpkg integrate install
cd ..

echo Creating directories...
mkdir build logs logs\data scenarios 2>nul

echo Building project...
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake -A x64
cmake --build . --config Release
cd ..

echo Setup completed!
echo Run: build\Release\radar_simulator.exe --generate single --mode console
pause
```

**Built for Windows Excellence** 🪟

This Windows adaptation provides all the tools and instructions needed to build and run the radar tracking system on Windows platforms with professional development practices.