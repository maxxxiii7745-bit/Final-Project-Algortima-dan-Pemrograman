#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <fstream> 
#include <algorithm> 
#include <sstream>   
#include <chrono>    
#include <thread>    
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// ==========================================
// 1. STRUKTUR DATA LOG
// ==========================================
struct LogEntry {
    int turn;
    string attacker;
    string action;
    int damage;
    string defender;
    int defenderHpRemaining;
    
    // Fitur baru: Status lengkap kedua karakter
    int swordSaintHP;
    int archmageHP;
    int swordSaintEnergy;
    int archmageEnergy;
};

// ==========================================
// 2. CLASS CHARACTERS
// ==========================================
class Character {
protected:
    string name;
    int HP;
    int cooldown;
    int energy;
    int damage;
public: 
    Character(string n, int h, int cd, int e) : name(n), HP(h), cooldown(cd), energy(e) {}
    virtual ~Character() {} 

    virtual void useWeapon() = 0; 
    virtual void displayOptions() = 0;
    virtual string getOptionsString() = 0;

    int getHP() const { return HP; }
    void setHP(int newHP) { HP = newHP; }
    string getName() const { return name; }
    int getCooldown() const { return cooldown; }
    void setCooldown(int newCooldown) { cooldown = newCooldown; }
    void reduceCooldown() {
        cooldown = (cooldown <= 0) ? 0 : cooldown - 1;
    }

    int getEnergy() const { return energy; }
    void setEnergy(int newEnergy) { energy = newEnergy; }
    int getDamage() const { return damage; }
    void takeDmg(int dmg) { HP -= dmg; }
};

class SwordSaint : public Character {
public:
    SwordSaint() : Character("Sword Saint", 140, 0, 0) {} 

    void useWeapon() override {
        damage = 15; energy += 10;
    }
    void iaiSlash() {
        if (cooldown <= 0) {
            damage = 30; cooldown = 2; energy += 15;
        } else {
            damage = 0; 
        }
    }
    void mountainSplitter() {
        if (energy >= 50) {
            damage = 40; energy -= 50;
        } else {
            damage = 0; 
        }       
    }

    void displayOptions() override {}
    
    string getOptionsString() override {
        stringstream ss;
        ss << "\n=== CURRENT TURN: SWORD SAINT ===\n"
           << "What will the Sword Saint do?\n1. Basic Attack\n2. Iai Slash\n3. Mountain Splitter (50 energy)\n"
           << "Skill cooldown: " << cooldown << "\nCurrent energy: " << energy << "\nCurrent HP: " << HP << "\nYour choice: ";
        return ss.str();
    }
};

class Archmage : public Character {
public:
    Archmage() : Character("Archmage", 130, 0, 0) {} 

    void useWeapon() override {
        damage = 12; energy += 15;
    }
    void beamBlast() {
        if (cooldown <= 0) { 
            damage = 24; energy += 25; cooldown = 2;
        } else {
            damage = 0; 
        }
    }
    void explosion() {
        if (energy >= 70) { 
            damage = 60; energy -= 70;
        } else {
            damage = 0; 
        }
    }

    void displayOptions() override {}

    string getOptionsString() override {
        stringstream ss;
        ss << "\n=== CURRENT TURN: ARCHMAGE ===\n"
           << "What will the Archmage do?\n1. Basic Attack\n2. Beam Blast\n3. Explosion (70 energy)\n"
           << "Skill cooldown: " << cooldown << "\nCurrent energy: " << energy << "\nCurrent HP: " << HP << "\nYour choice: ";
        return ss.str();
    }
};

// ==========================================
// 3. LOG MANAGEMENT SYSTEM
// ==========================================
class BattleLogger {
private:
    vector<LogEntry> logs;
    int currentTurn;

public:
    BattleLogger() : currentTurn(1) {}

    // Upgrade addLog dengan field baru
    void addLog(string attacker, string action, int damage, string defender, int defenderHpRemaining,
                int ssHp, int amHp, int ssEnergy, int amEnergy) {
        logs.push_back({currentTurn, attacker, action, damage, defender, defenderHpRemaining, 
                        ssHp, amHp, ssEnergy, amEnergy});
    }

    void advanceTurn() { currentTurn++; }
    vector<LogEntry>& getLogs() { return logs; }
};

// Fungsi Helper untuk Serialize 1 Log ke Format JSON
string logToJson(const LogEntry& log, bool isLast) {
    stringstream ss;
    ss << "  {\n"
       << "    \"turn\": " << log.turn << ",\n"
       << "    \"attacker\": \"" << log.attacker << "\",\n"
       << "    \"action\": \"" << log.action << "\",\n"
       << "    \"damage\": " << log.damage << ",\n"
       << "    \"defender\": \"" << log.defender << "\",\n"
       << "    \"defender_hp_remaining\": " << log.defenderHpRemaining << ",\n"
       << "    \"swordSaintHP\": " << log.swordSaintHP << ",\n"
       << "    \"archmageHP\": " << log.archmageHP << ",\n"
       << "    \"swordSaintEnergy\": " << log.swordSaintEnergy << ",\n"
       << "    \"archmageEnergy\": " << log.archmageEnergy << "\n"
       << "  }";
    if (!isLast) ss << ",";
    ss << "\n";
    return ss.str();
}

// Menampilkan 1 Log ke Terminal
void printSingleLog(const LogEntry& log) {
    cout << logToJson(log, true);
}

// Menampilkan Semua Log ke Terminal
void printAllLogs(const vector<LogEntry>& logs) {
    if (logs.empty()) {
        cout << "No logs available.\n";
        return;
    }
    cout << "[\n";
    for (size_t i = 0; i < logs.size(); ++i) {
        cout << logToJson(logs[i], i == logs.size() - 1);
    }
    cout << "]\n";
}

// Menyimpan Semua Log ke JSON File
void saveLogsToJSON(const vector<LogEntry>& logs, const string& filename = "battle_log.json") {
    ofstream filePointer(filename);
    if (!filePointer.is_open()) {
        cout << "Failed to save logs to " << filename << "!\n";
        return;
    }
    filePointer << "[\n";
    for (size_t i = 0; i < logs.size(); ++i) {
        filePointer << logToJson(logs[i], i == logs.size() - 1);
    }
    filePointer << "]\n";
    filePointer.close();
    cout << "Success! Logs saved to " << filename << "\n";
}

// Generic Selection Sort berdasarkan Field (Manual Sort)
void selectionSortLogs(vector<LogEntry>& logs, int fieldChoice, bool isAscending) {
    int n = logs.size();
    for (int i = 0; i < n - 1; i++) {
        int targetIdx = i;
        for (int j = i + 1; j < n; j++) {
            bool swapNeeded = false;
            
            int val1 = 0, val2 = 0;
            if (fieldChoice == 1) { val1 = logs[j].swordSaintHP; val2 = logs[targetIdx].swordSaintHP; }
            else if (fieldChoice == 2) { val1 = logs[j].archmageHP; val2 = logs[targetIdx].archmageHP; }
            else if (fieldChoice == 3) { val1 = logs[j].swordSaintEnergy; val2 = logs[targetIdx].swordSaintEnergy; }
            else if (fieldChoice == 4) { val1 = logs[j].archmageEnergy; val2 = logs[targetIdx].archmageEnergy; }

            if (isAscending) {
                swapNeeded = (val1 < val2);
            } else {
                swapNeeded = (val1 > val2);
            }

            if (swapNeeded) {
                targetIdx = j;
            }
        }
        if (targetIdx != i) {
            // Swap manual
            LogEntry temp = logs[i];
            logs[i] = logs[targetIdx];
            logs[targetIdx] = temp;
        }
    }
}

// Sub-menu Sorting
void handleSortingMenu(vector<LogEntry>& logs) {
    int field, order;
    cout << "\n=== SORT LOGS ===\n";
    cout << "1. HP Sword Saint\n2. HP Archmage\n3. Energy Sword Saint\n4. Energy Archmage\n";
    cout << "Choose field to sort: ";
    cin >> field;

    if (field < 1 || field > 4) {
        cout << "Invalid choice!\n";
        return;
    }

    cout << "\n1. Ascending\n2. Descending\nChoose order: ";
    cin >> order;

    bool isAscending = (order == 1);
    selectionSortLogs(logs, field, isAscending);
    cout << "\nLogs sorted successfully! Here are the sorted logs:\n";
    printAllLogs(logs);
}

// Sub-menu Searching (Linear Search)
void handleSearchingMenu(const vector<LogEntry>& logs) {
    cout << "\n=== SEARCH LOGS (Critical Condition) ===\n";
    bool found = false;
    
    // Linear search mencari kondisi kritis
    cout << "[\n";
    for (size_t i = 0; i < logs.size(); ++i) {
        if (logs[i].swordSaintHP < 70 || logs[i].archmageHP < 65) {
            cout << logToJson(logs[i], true);
            found = true;
        }
    }
    cout << "]\n";

    if (!found) {
        cout << "No critical condition found.\n";
    }
}

// Menu Utama Post-Battle Server
void displayMenu(vector<LogEntry>& logs) {
    int choice = 0;
    while (choice != 5) {
        cout << "\n=================================\n";
        cout << "      === BATTLE LOG MENU ===      \n";
        cout << "=================================\n";
        cout << "1. Show Logs\n";
        cout << "2. Sort Logs\n";
        cout << "3. Search Logs (Critical Condition)\n";
        cout << "4. Save Logs to JSON File\n";
        cout << "5. Exit\n";
        cout << "Your choice: ";
        
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
            case 1: printAllLogs(logs); break;
            case 2: handleSortingMenu(logs); break;
            case 3: handleSearchingMenu(logs); break;
            case 4: saveLogsToJSON(logs); break;
            case 5: cout << "Exiting log manager...\n"; break;
            default: cout << "Invalid choice!\n"; break;
        }
    }
}


// ==========================================
// 4. SOCKET & GAME LOGIC
// ==========================================
void sendData(SOCKET sock, string msg) {
    send(sock, msg.c_str(), msg.length(), 0);
    this_thread::sleep_for(chrono::milliseconds(100)); 
}

string receiveData(SOCKET sock) {
    char buffer[1024] = {0};
    recv(sock, buffer, 1024, 0);
    return string(buffer);
}

int handleRockPaperScissors(SOCKET p1, SOCKET p2) {
    while (true) {
        sendData(p1, "\n=== ROCK PAPER SCISSORS ===\n1. Rock\n2. Paper\n3. Scissors\nYour choice: ");
        sendData(p2, "\n=== ROCK PAPER SCISSORS ===\n1. Rock\n2. Paper\n3. Scissors\nYour choice: ");

        string move1 = receiveData(p1);
        string move2 = receiveData(p2);

        int m1 = atoi(move1.c_str());
        int m2 = atoi(move2.c_str());

        if (m1 == m2) {
            sendData(p1, "Draw! Retrying...\n");
            sendData(p2, "Draw! Retrying...\n");
            continue;
        }

        if ((m1 == 1 && m2 == 3) || (m1 == 2 && m2 == 1) || (m1 == 3 && m2 == 2)) {
            sendData(p1, "You earned the first turn!! You go first.\n");
            sendData(p2, "You lose the first turn!! You go second.\n");
            return 1; 
        } else {
            sendData(p1, "You lose the first turn!! You go second.\n");
            sendData(p2, "You earned the first turn!! You go first.\n");
            return 2; 
        }
    }
}

string executeCharacterTurn(Character* attacker, Character* defender, int choice, BattleLogger& logger) {
    stringstream report;
    string actionName = "";

    if (attacker->getName() == "Sword Saint") {
        SwordSaint* ss = static_cast<SwordSaint*>(attacker);
        if (choice == 1) { ss->useWeapon(); actionName = "Basic Attack"; report << "The sword saint swings their sword!\n"; }
        else if (choice == 2) { ss->iaiSlash(); actionName = "Iai Slash"; report << (ss->getDamage() > 0 ? "The sword saint performs their iai slash!\n" : "The sword saint cannot use their iai slash yet!\n"); }
        else if (choice == 3) { ss->mountainSplitter(); actionName = "Mountain Splitter"; report << (ss->getDamage() > 0 ? "The sword saint performs their mountain splitter!\n" : "The sword saint does not have enough energy!\n"); }
    } else {
        Archmage* am = static_cast<Archmage*>(attacker);
        if (choice == 1) { am->useWeapon(); actionName = "Basic Attack"; report << "The archmage casts their spell!\n"; }
        else if (choice == 2) { am->beamBlast(); actionName = "Beam Blast"; report << (am->getDamage() > 0 ? "The archmage casts beam blast!\n" : "The archmage cannot use their beam blast yet!\n"); }
        else if (choice == 3) { am->explosion(); actionName = "Explosion"; report << (am->getDamage() > 0 ? "The archmage casts explosion!\n" : "The archmage cannot cast explosion yet!\n"); }
    }

    if (actionName != "" && attacker->getDamage() >= 0) {
        defender->takeDmg(attacker->getDamage());
        report << "The " << defender->getName() << " took " << attacker->getDamage() << " damage!\n";
        
        // Ekstraksi stat untuk log
        int ssHp = (attacker->getName() == "Sword Saint") ? attacker->getHP() : defender->getHP();
        int amHp = (attacker->getName() == "Archmage") ? attacker->getHP() : defender->getHP();
        int ssE  = (attacker->getName() == "Sword Saint") ? attacker->getEnergy() : defender->getEnergy();
        int amE  = (attacker->getName() == "Archmage") ? attacker->getEnergy() : defender->getEnergy();

        logger.addLog(attacker->getName(), actionName, attacker->getDamage(), defender->getName(), defender->getHP(), ssHp, amHp, ssE, amE);
    } else {
        report << "The move failed or was invalid!\n";
        
        int ssHp = (attacker->getName() == "Sword Saint") ? attacker->getHP() : defender->getHP();
        int amHp = (attacker->getName() == "Archmage") ? attacker->getHP() : defender->getHP();
        int ssE  = (attacker->getName() == "Sword Saint") ? attacker->getEnergy() : defender->getEnergy();
        int amE  = (attacker->getName() == "Archmage") ? attacker->getEnergy() : defender->getEnergy();

        logger.addLog(attacker->getName(), "Invalid Move", 0, defender->getName(), defender->getHP(), ssHp, amHp, ssE, amE);
    }

    return report.str();
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 2);

    cout << "Server berjalan. Menunggu dua pemain bergabung..." << endl;

    SOCKET player1 = accept(server_fd, NULL, NULL);
    cout << "Pemain 1 Terkoneksi (Sword Saint)\n";
    sendData(player1, "Welcome Player 1! You are the Sword Saint.\nWaiting for Player 2...\n");

    SOCKET player2 = accept(server_fd, NULL, NULL);
    cout << "Pemain 2 Terkoneksi (Archmage)\n";
    sendData(player2, "Welcome Player 2! You are the Archmage.\nBattle is starting!\n");
    sendData(player1, "Player 2 has joined! Battle is starting!\n");

    SwordSaint* SS = new SwordSaint();
    Archmage* AM = new Archmage();
    BattleLogger logger;

    int suwitResult = handleRockPaperScissors(player1, player2);

    SOCKET firstSocket = (suwitResult == 1) ? player1 : player2;
    SOCKET secondSocket = (suwitResult == 1) ? player2 : player1;
    Character* firstChar = (suwitResult == 1) ? static_cast<Character*>(SS) : static_cast<Character*>(AM);
    Character* secondChar = (suwitResult == 1) ? static_cast<Character*>(AM) : static_cast<Character*>(SS);

    do {
        // --- GILIRAN PEMAIN PERTAMA ---
        sendData(firstSocket, firstChar->getOptionsString());
        sendData(secondSocket, "\nWaiting for " + firstChar->getName() + " to move...\n");
        
        int choiceFirst = atoi(receiveData(firstSocket).c_str());
        string report1 = executeCharacterTurn(firstChar, secondChar, choiceFirst, logger);
        
        sendData(firstSocket, report1);
        this_thread::sleep_for(chrono::milliseconds(300));
        sendData(secondSocket, report1);
        this_thread::sleep_for(chrono::milliseconds(300));

        if (secondChar->getHP() <= 0) break;

        // --- GILIRAN PEMAIN KEDUA ---
        sendData(secondSocket, secondChar->getOptionsString());
        sendData(firstSocket, "\nWaiting for " + secondChar->getName() + " to move...\n");
        
        int choiceSecond = atoi(receiveData(secondSocket).c_str());
        string report2 = executeCharacterTurn(secondChar, firstChar, choiceSecond, logger);
        
        sendData(secondSocket, report2);
        this_thread::sleep_for(chrono::milliseconds(300));
        sendData(firstSocket, report2);
        this_thread::sleep_for(chrono::milliseconds(300));

        firstChar->reduceCooldown();
        secondChar->reduceCooldown();
        logger.advanceTurn();

    } while (SS->getHP() > 0 && AM->getHP() > 0);

    // --- AKHIR PERTANDINGAN ---
    string finalResult = "";
    if (SS->getHP() > 0 && AM->getHP() <= 0) finalResult = "\nThe sword saint emerges victorious.\n";
    else if (AM->getHP() > 0 && SS->getHP() <= 0) finalResult = "\nThe archmage emerges victorious.\n";
    else finalResult = "\nBoth fighters have fallen.\n";

    sendData(player1, finalResult);
    sendData(player2, finalResult);
    
    // Memberitahu client untuk close
    sendData(player1, "Game Over. Disconnecting...\n");
    sendData(player2, "Game Over. Disconnecting...\n");

    // Tutup socket client
    closesocket(player1);
    closesocket(player2);
    closesocket(server_fd);
    WSACleanup();

    delete SS; 
    delete AM;

    // --- TAMPILKAN MENU BATTLE LOG DI SERVER ---
    cout << "\n[SERVER] Battle finished and clients disconnected.\n";
    displayMenu(logger.getLogs());

    return 0;
}