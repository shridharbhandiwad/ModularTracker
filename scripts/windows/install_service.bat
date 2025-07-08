@echo off
setlocal

echo ==========================================
echo   Install Radar Tracking Windows Service
echo ==========================================

REM Check if running as administrator
net session >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: This script must be run as Administrator
    echo Right-click and select "Run as administrator"
    pause
    exit /b 1
)

REM Get the current directory
set "CURRENT_DIR=%~dp0"
set "SERVICE_NAME=RadarTrackingService"
set "DISPLAY_NAME=Radar Tracking Service"
set "DESCRIPTION=Radar Tracking System for Defense Applications"

REM Look for the executable
set "EXE_PATH="
if exist "%CURRENT_DIR%..\..\build\Release\radar_tracking_system.exe" (
    set "EXE_PATH=%CURRENT_DIR%..\..\build\Release\radar_tracking_system.exe"
) else if exist "%CURRENT_DIR%..\..\build\Debug\radar_tracking_system.exe" (
    set "EXE_PATH=%CURRENT_DIR%..\..\build\Debug\radar_tracking_system.exe"
) else if exist "%CURRENT_DIR%radar_tracking_system.exe" (
    set "EXE_PATH=%CURRENT_DIR%radar_tracking_system.exe"
) else (
    echo ERROR: Could not find radar_tracking_system.exe
    echo Please build the project first or copy the executable to this directory
    pause
    exit /b 1
)

echo Found executable: %EXE_PATH%

REM Get full path to config file
set "CONFIG_PATH=%CURRENT_DIR%..\..\config\system_config.yaml"
if not exist "%CONFIG_PATH%" (
    echo WARNING: Configuration file not found at %CONFIG_PATH%
    echo The service may not start properly without a valid configuration
)

REM Create the service
echo Creating Windows service...
sc create "%SERVICE_NAME%" ^
    binPath= "\"%EXE_PATH%\" --service --config \"%CONFIG_PATH%\"" ^
    start= auto ^
    DisplayName= "%DISPLAY_NAME%" ^
    description= "%DESCRIPTION%"

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to create service
    echo Make sure the service doesn't already exist
    echo You can remove it with: sc delete %SERVICE_NAME%
    pause
    exit /b 1
)

echo Service created successfully!

REM Set service to restart on failure
echo Configuring service recovery options...
sc failure "%SERVICE_NAME%" reset= 86400 actions= restart/5000/restart/10000/restart/30000

REM Ask if user wants to start the service now
set /p "START_NOW=Start the service now? (y/n): "
if /i "%START_NOW%"=="y" (
    echo Starting service...
    sc start "%SERVICE_NAME%"
    if %ERRORLEVEL% EQU 0 (
        echo Service started successfully!
    ) else (
        echo Warning: Service creation succeeded but failed to start
        echo Check the Windows Event Log for details
        echo You can start it manually with: sc start %SERVICE_NAME%
    )
)

echo.
echo ==========================================
echo   Service Installation Complete
echo ==========================================
echo Service Name: %SERVICE_NAME%
echo Executable: %EXE_PATH%
echo Config: %CONFIG_PATH%
echo.
echo Management Commands:
echo   Start:   sc start %SERVICE_NAME%
echo   Stop:    sc stop %SERVICE_NAME%
echo   Status:  sc query %SERVICE_NAME%
echo   Remove:  sc delete %SERVICE_NAME%
echo.
echo You can also manage the service through:
echo   - Services.msc (Windows Services console)
echo   - Computer Management
echo   - PowerShell: Get-Service, Start-Service, Stop-Service
echo.

pause