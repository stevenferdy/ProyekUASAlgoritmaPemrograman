#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h> // DIWAJIBKAN untuk CRITICAL_SECTION dan CreateThread
#pragma comment(lib, "ws2_32.lib")   

#include "../common/QuestionTypes.h"
#include "../common/Participant.h"
#include "../common/LinkedList.h"
#include "../common/Algorithms.h"
#include "../common/JsonParser.h"
#include <vector>
#include <cstring>
#include <iostream>

#define PORT     9090
#define MAX_BUF  4096

using namespace std;

class QuizServer {
private:
    SOCKET            serverSocket;   
    vector<Question*> questionBank;
    ParticipantList   participants;
    
    // MENGGANTI std::mutex MENJADI CRITICAL_SECTION WINDOWS
    CRITICAL_SECTION  mtx;

    void initQuestions() {
        questionBank.push_back(new MultipleChoice(
            1, "Apa kepanjangan dari OOP?",
            "OOP", 10,
            {" Object Oriented Programming",
             " Object Oriented Protocol",
             " Output Oriented Programming",
             " None of the above"},
            'A'
        ));
        questionBank.push_back(new MultipleChoice(
            2, "Manakah pilar OOP yang benar?",
            "OOP", 10,
            {" Abstraksi, Enkapsulasi, Inheritance, Polimorfisme",
             " Array, Stack, Queue, Tree",
             " TCP, UDP, HTTP, FTP",
             " Quick Sort, Merge Sort, Bubble Sort"},
            'A'
        ));
        questionBank.push_back(new MultipleChoice(
            3, "Big O dari Binary Search adalah?",
            "Algoritma", 15,
            {" O(n)", " O(n^2)", " O(log n)", " O(n log n)"},
            'C'
        ));
        questionBank.push_back(new MultipleChoice(
            4, "Big O Merge Sort pada worst case adalah?",
            "Algoritma", 15,
            {" O(n^2)", " O(n log n)", " O(log n)", " O(1)"},
            'B'
        ));
        questionBank.push_back(new TrueFalse(
            5, "Quick Sort selalu lebih cepat dari Merge Sort.",
            "Algoritma", 10, false
        ));
        questionBank.push_back(new TrueFalse(
            6, "Abstract class bisa diinstansiasi langsung.",
            "OOP", 10, false
        ));
        questionBank.push_back(new TrueFalse(
            7, "Linked List manual memberi nilai bonus pada tugas ini.",
            "Struktur Data", 10, true
        ));
        questionBank.push_back(new Essay(
            8, "Jelaskan apa yang dimaksud dengan Polimorfisme dalam OOP!",
            "OOP", 20, "override", 20
        ));
        questionBank.push_back(new Essay(
            9, "Mengapa Socket diperlukan dalam aplikasi client-server?",
            "Jaringan", 20, "komunikasi", 20
        ));

        QuestionSorter::sortById(questionBank);
        cout << "[Server] " << questionBank.size() << " soal dimuat." << endl;
    }

    void sendQuestions(SOCKET clientSock, Participant* p) {
        for (auto q : questionBank) {
            string soalText =
                "\n================================================\n"
                "No. " + to_string(q->getId()) +
                "  [" + q->getType() + " | " + q->getCategory() +
                " | " + to_string(q->getPoints()) + " poin]\n" +
                q->getText() + "\n" +
                q->getChoicesText() +
                "Jawaban: ";

            string packet = JsonParser::makeQuestion(
                q->getId(), q->getType(),
                soalText, q->getChoicesText(),
                to_string(q->getPoints())
            ) + "\n";

            send(clientSock, packet.c_str(), (int)packet.size(), 0);

            char buf[MAX_BUF] = {};
            int n = recv(clientSock, buf, MAX_BUF-1, 0);
            if (n <= 0) { p->setDone(true); return; }

            auto data    = JsonParser::deserialize(string(buf));
            string ans   = data["answer"];
            bool correct = q->checkAnswer(ans);

            // LOCK MENGGUNAKAN WINDOWS API
            EnterCriticalSection(&mtx);
            p->recordAnswer();
            if (correct) p->addScore(q->getPoints());
            LeaveCriticalSection(&mtx);

            string res = JsonParser::makeResult(
                correct ? "1" : "0",
                correct ? to_string(q->getPoints()) : "0",
                to_string(p->getScore())
            ) + "\n";
            send(clientSock, res.c_str(), (int)res.size(), 0);
        }
        p->setDone(true);
    }

    void broadcastLeaderboard() {
        vector<Participant*> pVec;
        PNode* cur = participants.getHead();
        while (cur) { pVec.push_back(cur->data); cur = cur->next; }

        LeaderboardSorter::sortLeaderboard(pVec);

        string board = "=== LEADERBOARD ===\n";
        for (int i = 0; i < (int)pVec.size(); i++)
            board += to_string(i+1) + ". " + pVec[i]->getResultSummary() + "\n";
        board += "===================\n";

        string packet = JsonParser::makeLeaderboard(board) + "\n";
        cur = participants.getHead();
        while (cur) {
            send(cur->data->getSocket(), packet.c_str(), (int)packet.size(), 0);
            cur = cur->next;
        }
    }

    void handleClient(SOCKET clientSock) {
        char buf[MAX_BUF] = {};
        int n = recv(clientSock, buf, MAX_BUF-1, 0);
        if (n <= 0) { closesocket(clientSock); return; }  

        auto data  = JsonParser::deserialize(string(buf));
        string name = data["name"];

        Participant* p = new Participant(name, (int)clientSock);
        
        // LOCK MENGGUNAKAN WINDOWS API
        EnterCriticalSection(&mtx);
        participants.addParticipant(p);
        LeaveCriticalSection(&mtx);

        cout << "[Server] " << name << " bergabung." << endl;

        string notif = JsonParser::makeNotif(
            "Selamat datang " + name + "! Ujian akan segera dimulai."
        ) + "\n";
        send(clientSock, notif.c_str(), (int)notif.size(), 0);

        sendQuestions(clientSock, p);

        string done = JsonParser::makeNotif(
            "Ujian selesai!\n"
            "Skor    : " + to_string(p->getScore()) + "\n"
            "Benar   : " + to_string(p->getCorrectCount()) +
            " / " + to_string(p->getTotalAnswered()) + " soal\n"
            "Akurasi : " + to_string((int)p->getAccuracy()) + "%"
        ) + "\n";
        send(clientSock, done.c_str(), (int)done.size(), 0);

        // LOCK MENGGUNAKAN WINDOWS API
        EnterCriticalSection(&mtx);
        broadcastLeaderboard();
        LeaveCriticalSection(&mtx);

        cout << "[Server] " << name << " selesai. Skor: "
             << p->getScore() << endl;

        closesocket(clientSock);   
    }

    // Bridge helper fungsi agar CreateThread Windows bisa memanggil fungsi class C++
    static DWORD WINAPI ThreadBridge(LPVOID lpParam) {
        pair<QuizServer*, SOCKET>* p = (pair<QuizServer*, SOCKET>*)lpParam;
        p->first->handleClient(p->second);
        delete p; // bersihkan memori parampair
        return 0;
    }

public:
    QuizServer() : serverSocket(INVALID_SOCKET) {
        // INISIALISASI MUTEX WINDOWS
        InitializeCriticalSection(&mtx);
    }

    void start() {
        WSADATA wsaData;
        int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (wsaResult != 0) {
            cerr << "[Error] WSAStartup gagal: " << wsaResult << endl;
            return;
        }
        cout << "[Server] Winsock2 diinisialisasi." << endl;

        initQuestions();

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            cerr << "[Error] Gagal membuat socket." << endl;
            WSACleanup();
            return;
        }

        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
                   (const char*)&opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port        = htons(PORT);

        if (::bind(serverSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            cerr << "[Error] bind gagal. Port " << PORT << " mungkin sudah dipakai." << endl;
            closesocket(serverSocket);
            WSACleanup();
            return;
        }

        listen(serverSocket, 10);
        cout << "[Server] Quiz Server berjalan di port " << PORT << endl;
        cout << "[Server] Menunggu peserta... (Ctrl+C untuk berhenti)" << endl;

        while (true) {
            sockaddr_in cAddr{};
            int cLen = sizeof(cAddr);
            SOCKET clientSock = accept(serverSocket, (sockaddr*)&cAddr, &cLen);
            if (clientSock == INVALID_SOCKET) continue;

            // MENGGANTI std::thread MENJADI CreateThread WINDOWS API
            auto* param = new pair<QuizServer*, SOCKET>(this, clientSock);
            HANDLE hThread = CreateThread(NULL, 0, ThreadBridge, param, 0, NULL);
            if (hThread) {
                CloseHandle(hThread); // Detach thread agar memori otomatis bersih saat selesai
            } else {
                delete param;
            }
        }
    }

    ~QuizServer() {
        for (auto q : questionBank) delete q;
        
        // HAPUS MUTEX WINDOWS
        DeleteCriticalSection(&mtx);
        
        if (serverSocket != INVALID_SOCKET)
            closesocket(serverSocket);
        WSACleanup();   
    }
};