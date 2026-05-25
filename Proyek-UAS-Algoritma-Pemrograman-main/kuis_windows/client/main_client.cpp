#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h> // Diwajibkan untuk fungsi threading Windows API
#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <string>
#include <cstring>
#include "../common/JsonParser.h"

#define SERVER_IP  "127.0.0.1"
#define PORT       9090
#define MAX_BUF    4096

using namespace std;

SOCKET clientSocket;  
bool   running = true;

// Fungsi loop penerima pesan yang akan dijalankan oleh Thread Windows
DWORD WINAPI receiveLoop(LPVOID lpParam) {
    char buf[MAX_BUF];
    while (running) {
        memset(buf, 0, MAX_BUF);
        int n = recv(clientSocket, buf, MAX_BUF-1, 0);
        if (n <= 0) { running = false; break; }

        auto data  = JsonParser::deserialize(string(buf));
        string type = data["type"];

        if (type == "question") {
            cout << data["text"] << flush;

        } else if (type == "result") {
            bool correct = (data["correct"] == "1");
            if (correct)
                cout << "\n   [v] BENAR! +" << data["points"]
                     << " poin | Total: " << data["totalScore"] << " poin" << endl;
            else
                cout << "\n   [x] Salah.  Total: " << data["totalScore"] << " poin" << endl;

        } else if (type == "leaderboard") {
            cout << "\n" << string(20, '=') << endl;
            cout << data["content"];
            cout << string(20, '=') << endl;

        } else if (type == "notif") {
            cout << "\n[Server] " << data["message"] << endl;
            if (data["message"].find("selesai") != string::npos)
                running = false;
        }
    }
    return 0;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "[Error] WSAStartup gagal." << endl;
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "[Error] Gagal membuat socket." << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(PORT);
    
    // MENGGANTI inet_pton dengan inet_addr agar kompatibel dengan compiler Win32 lama
    addr.sin_addr.s_addr = inet_addr(SERVER_IP); 

    if (connect(clientSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cerr << "[Error] Tidak bisa terhubung ke server." << endl;
        cerr << "        Pastikan server.exe sudah dijalankan terlebih dahulu!" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "================================================" << endl;
    cout << "      PLATFORM KUIS ONLINE - Alprog UI          " << endl;
    cout << "================================================" << endl;
    cout << "Masukkan nama peserta: ";
    string name;
    getline(cin, name);

    string login = JsonParser::makeLogin(name) + "\n";
    send(clientSocket, login.c_str(), (int)login.size(), 0);

    // MENGGANTI std::thread dengan CreateThread Windows API
    HANDLE hThread = CreateThread(NULL, 0, receiveLoop, NULL, 0, NULL);

    while (running) {
        string ans;
        if (!getline(cin, ans) || !running) break;

        string packet = JsonParser::makeAnswer(name, "0", ans) + "\n";
        send(clientSocket, packet.c_str(), (int)packet.size(), 0);
    }

    // Menunggu thread selesai (pengganti join)
    if (hThread != NULL) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }

    closesocket(clientSocket); 
    WSACleanup();             

    cout << "\nTerima kasih sudah mengikuti kuis, " << name << "!" << endl;
    cout << "Tekan Enter untuk keluar...";
    cin.get();
    return 0;
}