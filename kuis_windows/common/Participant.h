#pragma once
#include <string>
using namespace std;

// ============================================================
//  Participant — menyimpan data peserta dan skor ujian
//  ENKAPSULASI: semua atribut private
// ============================================================
class Participant {
private:
    string name;
    int    socket_fd;
    int    score;           // total skor terkumpul
    int    totalAnswered;   // jumlah soal yang sudah dijawab
    int    correctCount;    // jumlah jawaban benar
    bool   isDone;          // sudah selesai ujian?

public:
    Participant(const string& n, int sock)
        : name(n), socket_fd(sock), score(0),
          totalAnswered(0), correctCount(0), isDone(false) {}

    // Getter
    string getName()         const { return name; }
    int    getSocket()       const { return socket_fd; }
    int    getScore()        const { return score; }
    int    getTotalAnswered() const { return totalAnswered; }
    int    getCorrectCount() const { return correctCount; }
    bool   getIsDone()       const { return isDone; }

    // Setter
    void setSocket(int s)  { socket_fd = s; }
    void setDone(bool d)   { isDone = d; }

    // Tambah skor jika jawaban benar
    void addScore(int pts) {
        score += pts;
        correctCount++;
    }

    // Catat bahwa satu soal sudah dijawab
    void recordAnswer() { totalAnswered++; }

    // Hitung akurasi dalam persen
    double getAccuracy() const {
        if (totalAnswered == 0) return 0.0;
        return (correctCount * 100.0) / totalAnswered;
    }

    // Ringkasan hasil ujian peserta
    string getResultSummary() const {
        return name + " | Skor: " + to_string(score) +
               " | Benar: " + to_string(correctCount) +
               "/" + to_string(totalAnswered);
    }
};
