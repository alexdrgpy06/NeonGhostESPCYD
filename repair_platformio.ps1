Write-Host "Cleaning corrupted PlatformIO environment..."
Remove-Item -Recurse -Force "C:\Users\alex0\.platformio\penv" -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force "C:\Users\alex0\.platformio\packages\tool-esptoolpy" -ErrorAction SilentlyContinue
Write-Host "Cleanup complete."
Write-Host "Please now try to BUILD the project again (Click the Checkmark icon or run 'pio run')."
Write-Host "PlatformIO will reinstall the necessary tools automatically."
