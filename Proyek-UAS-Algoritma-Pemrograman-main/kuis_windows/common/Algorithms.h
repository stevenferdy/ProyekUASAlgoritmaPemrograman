#pragma once
#include "Participant.h"
#include "Question.h"
#include <vector>

class LeaderboardSorter {
private:
    static void merge(vector<Participant*>& arr, int l, int m, int r) {
        int n1 = m - l + 1, n2 = r - m;
        vector<Participant*> L(arr.begin()+l, arr.begin()+m+1);
        vector<Participant*> R(arr.begin()+m+1, arr.begin()+r+1);

        int i = 0, j = 0, k = l;
        while (i < n1 && j < n2) {
            if (L[i]->getScore() >= R[j]->getScore())
                arr[k++] = L[i++];
            else
                arr[k++] = R[j++];
        }
        while (i < n1) arr[k++] = L[i++];
        while (j < n2) arr[k++] = R[j++];
    }

public:
    static void mergeSort(vector<Participant*>& arr, int l, int r) {
        if (l < r) {
            int m = l + (r - l) / 2;
            mergeSort(arr, l, m);
            mergeSort(arr, m+1, r);
            merge(arr, l, m, r);
        }
    }

    static void sortLeaderboard(vector<Participant*>& participants) {
        if (participants.size() > 1)
            mergeSort(participants, 0, participants.size()-1);
    }
};


class QuestionSorter {
private:
    static int partition(vector<Question*>& arr, int low, int high) {
        int pivot = arr[high]->getId();
        int i = low - 1;
        for (int j = low; j < high; j++) {
            if (arr[j]->getId() <= pivot) {
                i++;
                swap(arr[i], arr[j]);
            }
        }
        swap(arr[i+1], arr[high]);
        return i + 1;
    }

public:
    static void quickSort(vector<Question*>& arr, int low, int high) {
        if (low < high) {
            int pi = partition(arr, low, high);
            quickSort(arr, low, pi-1);
            quickSort(arr, pi+1, high);
        }
    }

    static void sortById(vector<Question*>& questions) {
        if (questions.size() > 1)
            quickSort(questions, 0, questions.size()-1);
    }
};


class QuestionSearcher {
public:
    static int binarySearchById(const vector<Question*>& arr, int targetId) {
        int left = 0, right = arr.size() - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (arr[mid]->getId() == targetId)      return mid;
            else if (arr[mid]->getId() < targetId)  left = mid + 1;
            else                                     right = mid - 1;
        }
        return -1;
    }

    static vector<Question*> linearSearchByCategory(
            const vector<Question*>& arr, const string& category) {
        vector<Question*> results;
        for (auto q : arr) {
            if (q->getCategory() == category)
                results.push_back(q);
        }
        return results;
    }
};
