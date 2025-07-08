@echo off
setlocal

echo ==========================================
echo  Uninstall Radar Tracking Windows Service
echo ==========================================

REM Check if running as administrator
net session >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: This script must be run as Administrator
    echo Right-click and select "Run as administrator"
    pause
    exit /b 1
)

set "SERVICE_NAME=RadarTrackingService"

REM Check if service exists
sc query "%SERVICE_NAME%" >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Service "%SERVICE_NAME%" does not exist or is not installed
    echo Nothing to uninstall
    pause
    exit /b 0
)

echo Found service: %SERVICE_NAME%

REM Get service status
echo Checking service status...
for /f "tokens=3" %%i in ('sc query "%SERVICE_NAME%" ^| findstr "STATE"') do set "SERVICE_STATE=%%i"

echo Service state: %SERVICE_STATE%

REM Stop the service if it's running
if /i "%SERVICE_STATE%"=="RUNNING" (
    echo Stopping service...
    sc stop "%SERVICE_NAME%"
    
    REM Wait for service to stop
    echo Waiting for service to stop...
    timeout /t 10 /nobreak >nul
    
    REM Check if it stopped
    for /f "tokens=3" %%i in ('sc query "%SERVICE_NAME%" ^| findstr "STATE"') do set "NEW_STATE=%%i"
    if /i "%NEW_STATE%"=="RUNNING" (
        echo WARNING: Service is still running after stop command
        echo You may need to stop it manually before uninstalling
        set /p "CONTINUE=Continue with uninstall anyway? (y/n): "
        if /i not "%CONTINUE%"=="y" (
            echo Uninstall cancelled
            pause
            exit /b 1
        )
    ) else (
        echo Service stopped successfully
    )
) else if /i "%SERVICE_STATE%"=="STOPPED" (
    echo Service is already stopped
) else (
    echo Service is in state: %SERVICE_STATE%
)

REM Delete the service
echo Removing service...
sc delete "%SERVICE_NAME%"

if %ERRORLEVEL% EQU 0 (
    echo Service "%SERVICE_NAME%" removed successfully!
) else (
    echo ERROR: Failed to remove service
    echo The service may still be running or in use
    echo Try stopping all related processes and run this script again
    pause
    exit /b 1
)

echo.
echo ==========================================
echo   Service Uninstallation Complete
echo ==========================================
echo Service "%SERVICE_NAME%" has been removed from the system
echo.
echo Note: Log files and configuration files have NOT been removed
echo You can manually delete them if no longer needed:
echo   - Configuration: config\system_config.yaml
echo   - Log files: logs\ directory
echo.

pause