@echo off
setlocal
cd /d "%~dp0"

set QT_BIN=C:\Qt6\6.5.11\msvc2019_64\bin

echo ===== Server: lupdate =====
"%QT_BIN%\lupdate.exe" -recursive ChatServer ^
    -ts ChatServer\translations\messenger_server_en.ts ^
        ChatServer\translations\messenger_server_ru.ts
if errorlevel 1 goto :error

echo.
echo ===== Server: lrelease =====
"%QT_BIN%\lrelease.exe" ^
    ChatServer\translations\messenger_server_en.ts ^
    ChatServer\translations\messenger_server_ru.ts
if errorlevel 1 goto :error

echo.
echo ===== Client: lupdate =====
"%QT_BIN%\lupdate.exe" -recursive ChatClient ^
    -ts ChatClient\translations\messenger_client_en.ts ^
        ChatClient\translations\messenger_client_ru.ts
if errorlevel 1 goto :error

echo.
echo ===== Client: lrelease =====
"%QT_BIN%\lrelease.exe" ^
    ChatClient\translations\messenger_client_en.ts ^
    ChatClient\translations\messenger_client_ru.ts
if errorlevel 1 goto :error

echo.
echo ===== Done =====
echo Edit .ts files to translate strings, then run this script again.
pause
exit /b 0

:error
echo.
echo *** Failed ***
pause
exit /b 1