#pragma once
#include "Participant.h"
#include <iostream>

// ============================================================
//  BONUS: Manual Linked List untuk daftar peserta
//  (tanpa std::list / std::vector)
//
//  Big O:
//    addParticipant    : O(1)
//    removeParticipant : O(n)
//    findByName        : O(n) — Linear Search
//    findBySocket      : O(n) — Linear Search
// ============================================================
struct PNode {
    Participant* data;
    PNode*       next;
    PNode(Participant* p) : data(p), next(nullptr) {}
};

class ParticipantList {
private:
    PNode* head;
    int    size;

public:
    ParticipantList() : head(nullptr), size(0) {}

    ~ParticipantList() {
        PNode* cur = head;
        while (cur) {
            PNode* tmp = cur->next;
            delete cur->data;
            delete cur;
            cur = tmp;
        }
    }

    // O(1) — tambah di depan list
    void addParticipant(Participant* p) {
        PNode* node = new PNode(p);
        node->next = head;
        head = node;
        size++;
    }

    // O(n) — hapus berdasarkan nama
    bool removeParticipant(const string& name) {
        PNode* cur = head, *prev = nullptr;
        while (cur) {
            if (cur->data->getName() == name) {
                if (prev) prev->next = cur->next;
                else      head = cur->next;
                delete cur;
                size--;
                return true;
            }
            prev = cur; cur = cur->next;
        }
        return false;
    }

    // O(n) — LINEAR SEARCH by nama
    Participant* findByName(const string& name) const {
        PNode* cur = head;
        while (cur) {
            if (cur->data->getName() == name)
                return cur->data;
            cur = cur->next;
        }
        return nullptr;
    }

    // O(n) — LINEAR SEARCH by socket fd
    Participant* findBySocket(int sock) const {
        PNode* cur = head;
        while (cur) {
            if (cur->data->getSocket() == sock)
                return cur->data;
            cur = cur->next;
        }
        return nullptr;
    }

    int    getSize() const { return size; }
    PNode* getHead() const { return head; }

    void printAll() const {
        PNode* cur = head;
        int rank = 1;
        while (cur) {
            cout << "  " << rank++ << ". "
                 << cur->data->getResultSummary() << endl;
            cur = cur->next;
        }
    }
};
