#pragma once
#include <string>
using namespace std;

class Participant {
private:
    string name;
    int    socket_fd;
    int    score;          
    int    totalAnswered;   
    int    correctCount;    
    bool   isDone;          

public:
    Participant(const string& n, int sock)
        : name(n), socket_fd(sock), score(0),
          totalAnswered(0), correctCount(0), isDone(false) {}

    string getName()         const { return name; }
    int    getSocket()       const { return socket_fd; }
    int    getScore()        const { return score; }
    int    getTotalAnswered() const { return totalAnswered; }
    int    getCorrectCount() const { return correctCount; }
    bool   getIsDone()       const { return isDone; }

    void setSocket(int s)  { socket_fd = s; }
    void setDone(bool d)   { isDone = d; }

    void addScore(int pts) {
        score += pts;
        correctCount++;
    }

    void recordAnswer() { totalAnswered++; }

    double getAccuracy() const {
        if (totalAnswered == 0) return 0.0;
        return (correctCount * 100.0) / totalAnswered;
    }

    string getResultSummary() const {
        return name + " | Skor: " + to_string(score) +
               " | Benar: " + to_string(correctCount) +
               "/" + to_string(totalAnswered);
    }
};
