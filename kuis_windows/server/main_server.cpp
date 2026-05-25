#include "QuizServer.h"

int main() {
    QuizServer server;
    server.start();
    return 0;
}

// ============================================================
//  KOMPILASI DI WINDOWS (MinGW/MSYS2):
//  g++ -std=c++17 -pthread -I../common main_server.cpp -o server.exe -lws2_32
//
//  JALANKAN:
//  server.exe
// ============================================================
