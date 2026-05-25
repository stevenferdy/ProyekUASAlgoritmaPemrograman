@echo off
title Quiz Client - Alpro UI
echo ================================================
echo   Kompilasi Quiz Client (Winsock2)
echo ================================================

cd /d "%~dp0client"

echo [Build] Mengompilasi client...
g++ -std=c++17 -pthread -I../common main_client.cpp -o client.exe -lws2_32

if %errorlevel% == 0 (
    echo [Build] Kompilasi BERHASIL!
    echo.
    echo ================================================
    echo   Menjalankan Quiz Client
    echo   Pastikan server.exe sudah berjalan dulu!
    echo ================================================
    echo.
    client.exe
) else (
    echo.
    echo [Error] Kompilasi GAGAL!
    echo Pastikan MinGW/MSYS2 sudah terinstall dan g++ ada di PATH.
    echo.
    pause
)
