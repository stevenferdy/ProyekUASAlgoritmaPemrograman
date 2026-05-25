#pragma once
#include <string>
#include <vector>
using namespace std;

class Question {
protected:
    int    id;
    string questionText;
    string category;
    int    points;      


public:
    Question(int id, const string& text, const string& cat, int pts)
        : id(id), questionText(text), category(cat), points(pts) {}

    virtual ~Question() {}


    int    getId()       const { return id; }
    string getText()     const { return questionText; }
    string getCategory() const { return category; }
    int    getPoints()   const { return points; }

    virtual string getType()                        const = 0;  
    virtual bool   checkAnswer(const string& ans)   const = 0;  
    virtual string getChoicesText()                 const = 0;  

    virtual string getSummary() const {
        return "[" + getType() + " | " + category + " | " +
               to_string(points) + " pts] " + questionText;
    }
};
