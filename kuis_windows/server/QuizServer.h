#pragma once

// ============================================================
//  WINSOCK2 — Header khusus Windows (HARUS di paling atas)
// ============================================================
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")   // link otomatis di MSVC

#include "../common/QuestionTypes.h"
#include "../common/Participant.h"
#include "../common/LinkedList.h"
#include "../common/Algorithms.h"
#include "../common/JsonParser.h"
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>
#include <iostream>

#define PORT     9090
#define MAX_BUF  4096

// ============================================================
//  QuizServer — Versi Windows (Winsock2)
//  Perbedaan dari Linux:
//    - Tipe socket: SOCKET (bukan int)
//    - Tutup socket: closesocket() (bukan close())
//    - Inisialisasi: WSAStartup() di awal
//    - Cleanup: WSACleanup() di akhir
// ============================================================
class QuizServer {
private:
    SOCKET            serverSocket;   // SOCKET bukan int
    vector<Question*> questionBank;
    ParticipantList   participants;
    mutex             mtx;

    // ---- Isi bank soal ----
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

        // Sort bank soal by ID — Quick Sort O(n log n)
        QuestionSorter::sortById(questionBank);
        cout << "[Server] " << questionBank.size() << " soal dimuat." << endl;
    }

    // ---- Kirim semua soal ke satu client ----
    void sendQuestions(SOCKET clientSock, Participant* p) {
        for (auto q : questionBank) {
            // Bangun teks soal lengkap
            string soalText =
                "\n================================================\n"
                "No. " + to_string(q->getId()) +
                "  [" + q->getType() + " | " + q->getCategory() +
                " | " + to_string(q->getPoints()) + " poin]\n" +
                q->getText() + "\n" +
                q->getChoicesText();

            string packet = JsonParser::makeQuestion(
                q->getId(), q->getType(),
                soalText, q->getChoicesText(),
                to_string(q->getPoints())
            ) + "\n";

            send(clientSock, packet.c_str(), (int)packet.size(), 0);

            // Tunggu jawaban dari client
            char buf[MAX_BUF] = {};
            int n = recv(clientSock, buf, MAX_BUF-1, 0);
            if (n <= 0) { p->setDone(true); return; }

            auto data    = JsonParser::deserialize(string(buf));
            string ans   = data["answer"];
            bool correct = q->checkAnswer(ans);

            lock_guard<mutex> lock(mtx);
            p->recordAnswer();
            if (correct) p->addScore(q->getPoints());

            // Kirim hasil per soal
            string res = JsonParser::makeResult(
                correct ? "1" : "0",
                correct ? to_string(q->getPoints()) : "0",
                to_string(p->getScore())
            ) + "\n";
            send(clientSock, res.c_str(), (int)res.size(), 0);
        }
    }

    // ---- Broadcast leaderboard ke semua client ----
    void broadcastLeaderboard(SOCKET newDoneSock) {
    vector<Participant*> pVec;
    PNode* cur = participants.getHead();
    while (cur) { pVec.push_back(cur->data); cur = cur->next; }

    LeaderboardSorter::sortLeaderboard(pVec);

    string board = "LEADERBOARD\n";
    for (int i = 0; i < (int)pVec.size(); i++)
        board += to_string(i+1) + ". " + pVec[i]->getResultSummary() + "\n";
    board += "\n";

    string packet = JsonParser::makeLeaderboard(board) + "\n";

    // Kirim ke semua yang sudah done
    cur = participants.getHead();
    while (cur) {
        if (cur->data->getIsDone()) {
            send(cur->data->getSocket(),
                 packet.c_str(), (int)packet.size(), 0);
        }
        cur = cur->next;
    }
    }

    // ---- Handle satu client (dijalankan di thread terpisah) ----
    void handleClient(SOCKET clientSock) {
        char buf[MAX_BUF] = {};
        int n = recv(clientSock, buf, MAX_BUF-1, 0);
        if (n <= 0) { closesocket(clientSock); return; }  // closesocket di Windows

        auto data  = JsonParser::deserialize(string(buf));
        string name = data["name"];

        // Daftarkan peserta
        // Winsock2: SOCKET adalah unsigned int, cast ke int untuk disimpan
        Participant* p = new Participant(name, (int)clientSock);
        {
            lock_guard<mutex> lock(mtx);
            participants.addParticipant(p);
        }

        cout << "[Server] " << name << " bergabung." << endl;

        string notif = JsonParser::makeNotif(
            "Selamat datang " + name + "! Ujian akan segera dimulai."
        ) + "\n";
        send(clientSock, notif.c_str(), (int)notif.size(), 0);

        // Kirim semua soal
        sendQuestions(clientSock, p);
        
        {
            lock_guard<mutex> lock(mtx);
            p->setDone(true);   // ← baris baru
        }  

        // Kirim hasil akhir
        string done = JsonParser::makeNotif(
            "Ujian selesai!\n"
            "Skor    : " + to_string(p->getScore()) + "\n"
            "Benar   : " + to_string(p->getCorrectCount()) +
            " / " + to_string(p->getTotalAnswered()) + " soal\n"
            "Akurasi : " + to_string((int)p->getAccuracy()) + "%"
        ) + "\n";
        send(clientSock, done.c_str(), (int)done.size(), 0);

        {
            lock_guard<mutex> lock(mtx);
            broadcastLeaderboard(clientSock);
        }

        cout << "[Server] " << name << " selesai. Skor: "
             << p->getScore() << endl;

             char keepbuff[MAX_BUF];
             while (true){
                int n = recv(clientSock, keepbuff, MAX_BUF-1,0);
                if (n <= 0) break;
             }

        closesocket(clientSock);   // Windows: closesocket()
    }

public:
    QuizServer() : serverSocket(INVALID_SOCKET) {}

    void start() {
        // ---- Inisialisasi Winsock2 (WAJIB di Windows) ----
        WSADATA wsaData;
        int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (wsaResult != 0) {
            cerr << "[Error] WSAStartup gagal: " << wsaResult << endl;
            return;
        }
        cout << "[Server] Winsock2 diinisialisasi." << endl;

        initQuestions();

        // Buat socket TCP
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            cerr << "[Error] Gagal membuat socket." << endl;
            WSACleanup();
            return;
        }

        // Agar port bisa langsung dipakai ulang
        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
                   (const char*)&opt, sizeof(opt));

        // Bind ke port
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

            // BONUS MULTITHREADING: spawn thread per peserta
            thread t(&QuizServer::handleClient, this, clientSock);
            t.detach();
        }
    }

    ~QuizServer() {
        for (auto q : questionBank) delete q;
        if (serverSocket != INVALID_SOCKET)
            closesocket(serverSocket);
        WSACleanup();   // Wajib dipanggil di akhir program
    }
};
