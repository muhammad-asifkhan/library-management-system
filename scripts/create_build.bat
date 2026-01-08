@echo off
set BUILD_DIR=build-gui
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cmake -S . -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=Release
cmake --build %BUILD_DIR% --config Release
echo Build complete. Run: %BUILD_DIR%\library_gui.exe
