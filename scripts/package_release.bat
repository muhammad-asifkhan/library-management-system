@echo off
set dt=%date:~10,4%%date:~4,2%%date:~7,2%
set RELEASE_DIR=release_%dt%
mkdir %RELEASE_DIR%
copy build\library_console.exe %RELEASE_DIR%\
copy build\library_gui.exe %RELEASE_DIR%\
xcopy data %RELEASE_DIR%\data /E /H /C /I
copy README.md %RELEASE_DIR%\
copy CMakeLists.txt %RELEASE_DIR%\
powershell Compress-Archive -Path %RELEASE_DIR% -DestinationPath %RELEASE_DIR%.zip
rmdir /s /q %RELEASE_DIR%
echo Release package created: %RELEASE_DIR%.zip
