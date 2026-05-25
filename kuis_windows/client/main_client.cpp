// ============================================================
//  WINSOCK2 — Header khusus Windows (HARUS di paling atas)
// ============================================================
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include "../common/JsonParser.h"

#define SERVER_IP  "127.0.0.1"
#define PORT       9090
#define MAX_BUF    4096

using namespace std;

SOCKET clientSocket;   // Windows: SOCKET bukan int
bool   running = true;
bool   quizDone = false;

// ---- Thread: terima pesan dari server terus-menerus ----
void receiveLoop() {
    char buf[MAX_BUF];
    while (running) {
        memset(buf, 0, MAX_BUF);
        int n = recv(clientSocket, buf, MAX_BUF-1, 0);
        if (n <= 0) { running = false; break; }

        auto data   = JsonParser::deserialize(string(buf));
        string type = data["type"];

        if (type == "question") {
            cout << "\n" << string(48, '-') << "\n";
            cout << data["text"];
            cout << "Jawaban: " << flush;

        } else if (type == "result") {
            bool correct = (data["correct"] == "1");
            if (correct)
                cout << "\n  [v] BENAR! +" << data["points"]
                     << " poin | Total: " << data["totalScore"] << " poin" << endl;
            else
                cout << "\n  [x] Salah.  Total: "
                     << data["totalScore"] << " poin" << endl;

        } else if (type == "notif") {
            cout << "\n[Server] " << data["message"] << endl;
            if (data["message"].find("selesai") != string::npos)
                quizDone = true;  // tandai selesai, tapi jangan disconnect

        } else if (type == "leaderboard") {
            cout << "\n" << string(48, '=') << "\n";
            cout << data["content"];
            cout << string(48, '=') << "\n";
            if (quizDone)
                cout << "Tekan Enter untuk keluar..." << flush;
        }
    }
}

int main() {
    // ---- Inisialisasi Winsock2 (WAJIB) ----
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "[Error] WSAStartup gagal." << endl;
        return 1;
    }

    // Buat socket & koneksi ke server
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "[Error] Gagal membuat socket." << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cerr << "[Error] Tidak bisa terhubung ke server." << endl;
        cerr << "        Pastikan server.exe sudah dijalankan terlebih dahulu!" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Tampilan header
    cout << "================================================" << endl;
    cout << "      PLATFORM KUIS ONLINE - Alprog UI          " << endl;
    cout << "================================================" << endl;
    cout << "Masukkan nama peserta: ";
    string name;
    getline(cin, name);

    // Kirim login
    string login = JsonParser::makeLogin(name) + "\n";
    send(clientSocket, login.c_str(), (int)login.size(), 0);

    // Spawn thread penerima pesan
    thread recvThread(receiveLoop);

    // Main thread: baca input jawaban dari user
    while (running) {
        string ans;
        if (!getline(cin, ans)) break;
        if (!running) break;

        if (quizDone) break;  // sudah selesai → Enter untuk keluar

        string packet = JsonParser::makeAnswer(name, "0", ans) + "\n";
        send(clientSocket, packet.c_str(), (int)packet.size(), 0);
    }

    running = false;
    if (recvThread.joinable()) recvThread.join();

    closesocket(clientSocket);
    WSACleanup();

    cout << "\nTerima kasih sudah mengikuti kuis, " << name << "!" << endl;
    return 0;
}

