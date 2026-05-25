# Platform Kuis Online 
Tugas Akhir Algoritma dan Pemrograman | Teknik Elektro UI

## Kelompok AAP
1. Steven Ferdy Naibaho - 2506584281
2. Aditya Yohannes Peacero Sitinjak - 2506644103
3. Andreas Putra Nugraha Siagian - 2506644463

## Prasyarat
Install **MinGW** atau **MSYS2** agar punya g++ di Windows:
- Download: https://www.msys2.org
- Setelah install MSYS2, buka MSYS2 terminal dan jalankan:
  ```
  pacman -S mingw-w64-x86_64-gcc
  ```
- Tambahkan `C:\msys64\mingw64\bin` ke PATH Windows

## Cara Menjalankan
1. Double klik `run_server.bat` → tunggu server jalan
2. Double klik `run_client.bat` di jendela baru → ikuti kuis
3. Untuk peserta ke-2, 3, dst → double klik `run_client.bat` lagi

## Struktur File
```
quizapp_windows/
├── common/
│   ├── Question.h        ← Abstract Base Class (Abstraksi)
│   ├── QuestionTypes.h   ← MC, TrueFalse, Essay (Inheritance)
│   ├── Participant.h     ← Data peserta + skor (Enkapsulasi)
│   ├── LinkedList.h      ← Manual Linked List (Bonus)
│   ├── Algorithms.h      ← Merge Sort, Quick Sort, Binary/Linear Search
│   └── JsonParser.h      ← JSON serializer
├── server/
│   ├── QuizServer.h      ← Server Winsock2 + Multithreading
│   └── main_server.cpp
├── client/
│   └── main_client.cpp   ← Client Winsock2
├── run_server.bat         ← Double klik untuk jalankan server
└── run_client.bat         ← Double klik untuk jalankan client
```

## Akun & Soal Default
- 9 soal: 4 Multiple Choice, 3 True/False, 2 Essay
