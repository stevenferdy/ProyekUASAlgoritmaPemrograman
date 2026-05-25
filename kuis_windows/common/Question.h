#pragma once
#include <string>
#include <vector>
using namespace std;

// ============================================================
//  ABSTRAKSI: Abstract Base Class — tidak bisa diinstansiasi
// ============================================================
class Question {
protected:
    // ENKAPSULASI: semua atribut private, akses via getter
    int    id;
    string questionText;
    string category;
    int    points;      // nilai jika benar

public:
    Question(int id, const string& text, const string& cat, int pts)
        : id(id), questionText(text), category(cat), points(pts) {}

    virtual ~Question() {}

    // Getter
    int    getId()       const { return id; }
    string getText()     const { return questionText; }
    string getCategory() const { return category; }
    int    getPoints()   const { return points; }

    // Pure virtual — WAJIB diimplementasikan subclass
    virtual string getType()                        const = 0;  // ABSTRAKSI
    virtual bool   checkAnswer(const string& ans)   const = 0;  // ABSTRAKSI
    virtual string getChoicesText()                 const = 0;  // untuk ditampilkan ke client

    // Virtual untuk Polimorfisme
    virtual string getSummary() const {
        return "[" + getType() + " | " + category + " | " +
               to_string(points) + " pts] " + questionText;
    }
};
