#pragma once
#include "Question.h"
#include <algorithm>

// ============================================================
//  INHERITANCE + POLIMORFISME: MultipleChoice mewarisi Question
// ============================================================
class MultipleChoice : public Question {
private:
    vector<string> choices;     // pilihan jawaban A, B, C, D
    char           correctKey;  // 'A', 'B', 'C', atau 'D'

public:
    MultipleChoice(int id, const string& text, const string& cat,
                   int pts, vector<string> opts, char correct)
        : Question(id, text, cat, pts),
          choices(opts), correctKey(toupper(correct)) {}

    // POLIMORFISME: override getType
    string getType() const override { return "MultipleChoice"; }

    // POLIMORFISME: override checkAnswer
    bool checkAnswer(const string& ans) const override {
        if (ans.empty()) return false;
        return toupper(ans[0]) == correctKey;
    }

    // POLIMORFISME: override getChoicesText
    string getChoicesText() const override {
        string result = "";
        char label = 'A';
        for (const auto& c : choices)
            result += "  " + string(1, label++) + ". " + c + "\n";
        return result;
    }

    // POLIMORFISME: override getSummary
    string getSummary() const override {
        return "[MC | " + category + " | " + to_string(points) + " pts] " + questionText;
    }
};

// ============================================================
//  INHERITANCE: TrueFalse mewarisi Question
// ============================================================
class TrueFalse : public Question {
private:
    bool correctAnswer;   // true = Benar, false = Salah

public:
    TrueFalse(int id, const string& text, const string& cat,
              int pts, bool correct)
        : Question(id, text, cat, pts), correctAnswer(correct) {}

    string getType() const override { return "TrueFalse"; }

    bool checkAnswer(const string& ans) const override {
        // Terima: "true"/"false", "benar"/"salah", "1"/"0", "B"/"S"
        string a = ans;
        transform(a.begin(), a.end(), a.begin(), ::tolower);
        bool userAnswer = (a == "true" || a == "benar" ||
                           a == "1"    || a == "b");
        return userAnswer == correctAnswer;
    }

    string getChoicesText() const override {
        return "  Jawab: Benar / Salah\n";
    }
};

// ============================================================
//  INHERITANCE: Essay mewarisi Question (penilaian manual)
// ============================================================
class Essay : public Question {
private:
    string keyAnswer;   // kata kunci yang harus ada di jawaban
    int    minLength;   // panjang minimum jawaban

public:
    Essay(int id, const string& text, const string& cat,
          int pts, const string& key, int minLen = 20)
        : Question(id, text, cat, pts),
          keyAnswer(key), minLength(minLen) {}

    string getType() const override { return "Essay"; }

    // Pengecekan sederhana: jawaban mengandung kata kunci
    bool checkAnswer(const string& ans) const override {
        if ((int)ans.size() < minLength) return false;
        string lAns = ans, lKey = keyAnswer;
        transform(lAns.begin(), lAns.end(), lAns.begin(), ::tolower);
        transform(lKey.begin(), lKey.end(), lKey.begin(), ::tolower);
        return lAns.find(lKey) != string::npos;
    }

    string getChoicesText() const override {
        return "  [Soal Essay - tulis jawaban lengkap]\n";
    }
};
