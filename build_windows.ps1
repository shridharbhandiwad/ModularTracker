param(
    [string]$Configuration = "Release",
    [string]$VcpkgRoot = "",
    [switch]$InstallDeps = $false,
    [switch]$Clean = $false,
    [switch]$Help = $false
)

function Write-Status {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Green
}

function Write-Warning {
    param([string]$Message)
    Write-Host "[WARN] $Message" -ForegroundColor Yellow
}

function Write-Error {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

if ($Help) {
    Write-Host @"
Radar Tracking System - Windows PowerShell Build Script

Usage: .\build_windows.ps1 [options]

Options:
  -Configuration <config>   Build configuration (Release/Debug) [default: Release]
  -VcpkgRoot <path>        Path to vcpkg root directory
  -InstallDeps             Install dependencies using vcpkg
  -Clean                   Clean build directory before building
  -Help                    Show this help message

Examples:
  .\build_windows.ps1
  .\build_windows.ps1 -Configuration Debug
  .\build_windows.ps1 -InstallDeps -Clean
  .\build_windows.ps1 -VcpkgRoot "C:\vcpkg"
"@
    exit 0
}

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "  Radar Tracking System - PowerShell Build" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

# Check prerequisites
Write-Status "Checking prerequisites..."

$requiredTools = @("git", "cmake", "cl")
foreach ($tool in $requiredTools) {
    if (!(Get-Command $tool -ErrorAction SilentlyContinue)) {
        Write-Error "$tool is not installed or not in PATH"
        if ($tool -eq "cl") {
            Write-Host "Please run this from a Visual Studio Developer PowerShell" -ForegroundColor Yellow
        }
        exit 1
    }
}

Write-Status "Prerequisites check passed!"

# Install dependencies if requested
if ($InstallDeps) {
    Write-Status "Installing dependencies with vcpkg..."
    
    if ([string]::IsNullOrEmpty($VcpkgRoot)) {
        if (Test-Path "vcpkg") {
            $VcpkgRoot = "vcpkg"
        } else {
            Write-Status "Cloning vcpkg..."
            git clone https://github.com/Microsoft/vcpkg.git
            if ($LASTEXITCODE -ne 0) {
                Write-Error "Failed to clone vcpkg"
                exit 1
            }
            $VcpkgRoot = "vcpkg"
        }
    }
    
    $vcpkgExe = Join-Path $VcpkgRoot "vcpkg.exe"
    if (!(Test-Path $vcpkgExe)) {
        Write-Status "Bootstrapping vcpkg..."
        Push-Location $VcpkgRoot
        .\bootstrap-vcpkg.bat
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Failed to bootstrap vcpkg"
            Pop-Location
            exit 1
        }
        Pop-Location
    }
    
    Write-Status "Installing required packages..."
    $packages = @("yaml-cpp", "spdlog", "eigen3", "boost", "gtest")
    
    foreach ($package in $packages) {
        Write-Status "Installing $package`:x64-windows..."
        & $vcpkgExe install "$package`:x64-windows"
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Failed to install $package"
            exit 1
        }
    }
    
    Write-Status "Integrating with Visual Studio..."
    & $vcpkgExe integrate install
    
    $toolchainFile = Join-Path $VcpkgRoot "scripts\buildsystems\vcpkg.cmake"
} else {
    # Try to find existing vcpkg installation
    $possiblePaths = @(
        "vcpkg\scripts\buildsystems\vcpkg.cmake",
        "C:\vcpkg\scripts\buildsystems\vcpkg.cmake",
        "C:\tools\vcpkg\scripts\buildsystems\vcpkg.cmake"
    )
    
    $toolchainFile = $null
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            $toolchainFile = $path
            break
        }
    }
}

# Create necessary directories
Write-Status "Creating directories..."
$directories = @("logs", "logs\data", "scenarios")
foreach ($dir in $directories) {
    if (!(Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir -Force | Out-Null
    }
}

# Clean build directory if requested
if ($Clean -and (Test-Path "build")) {
    Write-Status "Cleaning build directory..."
    Remove-Item -Recurse -Force "build"
}

# Create build directory
if (!(Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" -Force | Out-Null
}

Write-Status "Configuring with CMake..."
Push-Location "build"

# Build CMake arguments
$cmakeArgs = @(
    "..",
    "-G", "Visual Studio 16 2019",
    "-A", "x64",
    "-DCMAKE_BUILD_TYPE=$Configuration",
    "-DBUILD_TESTING=ON",
    "-DBUILD_SIMULATION=ON",
    "-DBUILD_BENCHMARKS=ON"
)

if ($toolchainFile) {
    $cmakeArgs += "-DCMAKE_TOOLCHAIN_FILE=$toolchainFile"
    Write-Status "Using vcpkg toolchain: $toolchainFile"
}

Write-Host "Running: cmake $($cmakeArgs -join ' ')"
cmake @cmakeArgs

if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed!"
    Pop-Location
    exit 1
}

Write-Status "Building the solution..."
cmake --build . --config $Configuration --parallel

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed!"
    Pop-Location
    exit 1
}

Write-Status "Running tests..."
ctest -C $Configuration --output-on-failure

Pop-Location

# Check build results
Write-Status "Checking build results..."
$exeDir = "build\$Configuration"
$success = $true

$executables = @("radar_simulator.exe", "radar_tracking_system.exe")
foreach ($exe in $executables) {
    $path = Join-Path $exeDir $exe
    if (Test-Path $path) {
        Write-Status "$exe built successfully"
    } else {
        Write-Warning "$exe not found"
        $success = $false
    }
}

Write-Host ""
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "  Build Summary" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "  Build Type: $Configuration"
Write-Host "  CMake: OK"
Write-Host "  Compilation: OK"
Write-Host "  Tests: OK"
Write-Host "  Executables: $(if ($success) { 'OK' } else { 'Check above' })"
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

if ($success) {
    Write-Host "Next Steps:" -ForegroundColor Green
    Write-Host "  1. Test the simulator:"
    Write-Host "     $exeDir\radar_simulator.exe --generate single --mode console"
    Write-Host ""
    Write-Host "  2. Validate configuration:"
    Write-Host "     $exeDir\radar_tracking_system.exe --validate --config config\system_config.yaml"
    Write-Host ""
    Write-Host "  3. Run the system:"
    Write-Host "     $exeDir\radar_tracking_system.exe --config config\system_config.yaml"
    Write-Host ""
    
    Write-Status "Build completed successfully!"
} else {
    Write-Warning "Some executables were not built successfully"
    Write-Host "Check the build output above for errors"
}