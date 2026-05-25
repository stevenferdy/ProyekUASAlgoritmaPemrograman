@echo off
title Quiz Server - Alpro UI
echo ================================================
echo   Kompilasi Quiz Server (Winsock2)
echo ================================================

cd /d "%~dp0server"

echo [Build] Mengompilasi server...
:: Di sini kita langsung panggil main_server.cpp karena sudah di dalam folder server
g++ main_server.cpp -o server.exe -lws2_32

if %errorlevel% == 0 (
    echo [Build] Kompilasi BERHASIL!
    echo.
    echo ================================================
    echo   Menjalankan Quiz Server di port 9090
    echo   Tekan Ctrl+C untuk menghentikan server
    echo ================================================
    echo.
    server.exe
) else (
    echo.
    echo [Error] Kompilasi GAGAL!
    echo Periksa kembali sintaksis kode program atau konfigurasi compiler Anda.
    echo.
    pause
)