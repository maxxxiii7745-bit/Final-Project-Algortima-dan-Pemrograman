#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>

#pragma comment(lib, "ws2_32.lib") // Buat nge-link library Winsock secara otomatis di Windows

using namespace std;

int main() {
    WSADATA wsaData;
    // Start Winsock dulu, ini wajib hukumnya di Windows sebelum mainan socket
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Failed to initialize Winsock." << endl;
        return 1;
    }

    // Bikin socket TCP buat jalur koneksinya
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cout << "Failed to create socket." << endl;
        WSACleanup();
        return 1;
    }

    string serverIP;
    // Minta IP server, kalau cuma tes di satu laptop tinggal ketik 127.0.0.1
    cout << "Enter Server IP Address (Type '127.0.0.1' if on the same PC): ";
    cin >> serverIP;

    // Setting alamat server tujuan (kita samakan port-nya di 8080)
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr);

    cout << "Connecting to the battlefield..." << endl;
    // Mulai coba colok/hubungi server yang ditarget
    if (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        cout << "Failed to connect to the server. Make sure the server is already running." << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    char buffer[4096];
    int bytesReceived;

    // Loop utama biar client terus-menerus dengerin kiriman data dari server
    while (true) {
        memset(buffer, 0, sizeof(buffer)); // Kosongin buffer biar gak ada teks sisa sebelumnya
        bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0); // Terima paket dari server

        // Kalau server mati atau koneksi mendadak putus, loop berhenti
        if (bytesReceived <= 0) {
            cout << "\nConnection lost or the battle has ended." << endl;
            break;
        }

        string msgFromServer(buffer);
        cout << msgFromServer; // Cetak pesan/menu dari server langsung ke layar player

        // Kalau terdeteksi server minta input ("Your choice:"), baru kita ketik dan kirim balik
        if (msgFromServer.find("Your choice:") != string::npos) {
            string userChoice;
            cin >> userChoice;
            send(sock, userChoice.c_str(), userChoice.length(), 0); // Kirim jawaban kita ke server
        }
    }

    // Beres-beres socket dan matikan fungsi network sebelum program ditutup
    closesocket(sock);
    WSACleanup();
    return 0;
}