#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <cstring>
#include <iomanip>
#include <limits>

using namespace std;

int getSafeInt(const char* prompt, int minVal) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            if (value >= minVal) {
                return value;
            }
            else {
                cout << " [!] Error: Number must be at least " << minVal << ".\n";
            }
        }
        else {
            cout << " [!] Error: Invalid input! Please enter an integer.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

class ElectoralDistrict {
private:
    char* name;
    int registeredVoters;
    int partyCount;
    int* votesPerParty;
    char** partyNames;

    void copyFrom(const ElectoralDistrict& other) {
        name = new char[strlen(other.name) + 1];
        strcpy(name, other.name);

        registeredVoters = other.registeredVoters;
        partyCount = other.partyCount;

        votesPerParty = new int[partyCount];
        for (int i = 0; i < partyCount; i++) {
            votesPerParty[i] = other.votesPerParty[i];
        }

        partyNames = new char* [partyCount];
        for (int i = 0; i < partyCount; i++) {
            partyNames[i] = new char[strlen(other.partyNames[i]) + 1];
            strcpy(partyNames[i], other.partyNames[i]);
        }
    }

    void freeMemory() {
        delete[] name;
        delete[] votesPerParty;
        for (int i = 0; i < partyCount; i++) {
            delete[] partyNames[i];
        }
        delete[] partyNames;
    }

public:
    ElectoralDistrict() {
        name = nullptr;
        registeredVoters = 0;
        partyCount = 0;
        votesPerParty = nullptr;
        partyNames = nullptr;
    }

    ElectoralDistrict(const char* _name, int _voters, int _pCount) {
        try {
            name = new char[strlen(_name) + 1];
            strcpy(name, _name);
            registeredVoters = _voters;
            partyCount = _pCount;

            votesPerParty = new int[partyCount];
            partyNames = new char* [partyCount];

            for (int i = 0; i < partyCount; i++) {
                votesPerParty[i] = 0;
                partyNames[i] = nullptr;
            }
        }
        catch (bad_alloc& e) {
            cout << " [!] Error allocating memory: " << e.what() << endl;
            exit(1);
        }
    }

    ElectoralDistrict(const ElectoralDistrict& other) {
        copyFrom(other);
    }

    //прави пълно копие на обекта без &, const не го променя read only
    ElectoralDistrict& operator=(const ElectoralDistrict& other) {
        if (this != &other) {
            freeMemory();
            copyFrom(other);
        }
        return *this;
    }

    ~ElectoralDistrict() {
        freeMemory();
    }

    void inputPartyData() {
        char buffer[100];
        int currentTotalVotes = 0;

        cout << " --- Input parties for district '" << name << "' ---\n";

        for (int i = 0; i < partyCount; i++) {
            cout << "  -> Party " << (i + 1) << " initials: ";
            cin >> buffer;

            partyNames[i] = new char[strlen(buffer) + 1];
            strcpy(partyNames[i], buffer);

            while (true) {
                int tempVotes = getSafeInt("     Votes: ", 0);
                int remainingVoters = registeredVoters - currentTotalVotes;

                if (tempVotes <= remainingVoters) {
                    votesPerParty[i] = tempVotes;
                    currentTotalVotes += tempVotes;
                    break;
                }
                else {
                    cout << " [!] Error! Only " << remainingVoters
                        << " voters remaining. You entered " << tempVotes << ".\n";
                }
            }
        }
    }

    const char* getName() const { return name; }

	void saveToFile(ofstream& out) { // Сериализация
        int nameLen = strlen(name);
        out.write((char*)&nameLen, sizeof(nameLen));
        out.write(name, nameLen);

        out.write((char*)&registeredVoters, sizeof(registeredVoters));
        out.write((char*)&partyCount, sizeof(partyCount));

        out.write((char*)votesPerParty, sizeof(int) * partyCount);

        for (int i = 0; i < partyCount; i++) {
            int pLen = strlen(partyNames[i]);
            out.write((char*)&pLen, sizeof(pLen));
            out.write(partyNames[i], pLen);
        }
    }

    void loadFromFile(ifstream& in) { // Десериализация
        freeMemory();

        int nameLen;
        in.read((char*)&nameLen, sizeof(nameLen));
        name = new char[nameLen + 1];
        in.read(name, nameLen);
        name[nameLen] = '\0';

        in.read((char*)&registeredVoters, sizeof(registeredVoters));
        in.read((char*)&partyCount, sizeof(partyCount));

        if (partyCount < 0 || partyCount > 100000) {
            cout << " [!] File error: invalid party count.\n";
            partyCount = 0;
            return;
        }

        votesPerParty = new int[partyCount];
        in.read((char*)votesPerParty, sizeof(int) * partyCount);

        partyNames = new char* [partyCount];
        for (int i = 0; i < partyCount; i++) {
            int pLen;
            in.read((char*)&pLen, sizeof(pLen));
            partyNames[i] = new char[pLen + 1];
            in.read(partyNames[i], pLen);
            partyNames[i][pLen] = '\0';
        }
    }

    void printStats() const {
        int totalVotesCast = 0;
        for (int i = 0; i < partyCount; i++)
            totalVotesCast += votesPerParty[i];

        cout << "--- District: " << name << " ---\n";
        for (int i = 0; i < partyCount; i++) {
            double percent = 0;
            if (totalVotesCast > 0)
                percent = ((double)votesPerParty[i] / totalVotesCast) * 100.0;

            cout << "  " << partyNames[i] << ": "
                << fixed << setprecision(2) << percent << "% (" << votesPerParty[i] << " votes)\n";
        }
    }

    bool hasWinner() const {
        int totalVotesCast = 0;
        for (int i = 0; i < partyCount; i++)
            totalVotesCast += votesPerParty[i];

        if (totalVotesCast == 0) return false;

        for (int i = 0; i < partyCount; i++) {
            if (votesPerParty[i] > totalVotesCast / 2) {
                return true;
            }
        }
        return false;
    }

    double getNonVotersPercent() const {
        int totalVotesCast = 0;
        for (int i = 0; i < partyCount; i++) totalVotesCast += votesPerParty[i];

        int nonVoters = registeredVoters - totalVotesCast;
        if (registeredVoters == 0) return 0.0;

        return ((double)nonVoters / registeredVoters) * 100.0;
    }
};

void sortDistricts(ElectoralDistrict* arr, int count) {
    for (int i = 0; i < count - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < count; j++) {
            if (strcmp(arr[j].getName(), arr[minIdx].getName()) < 0) {
                minIdx = j;
            }
        }
        if (minIdx != i) {
            ElectoralDistrict temp = arr[i];
            arr[i] = arr[minIdx];
            arr[minIdx] = temp;
        }
    }
}

int main() {
    int n;
    n = getSafeInt("Enter number of electoral districts (at least 1): ", 1);

    ofstream outFile("districts.dat", ios::binary);
    if (!outFile) { cerr << "Error: Cannot create file!\n"; return 1; }

    outFile.write((char*)&n, sizeof(n));

    for (int i = 0; i < n; i++) {
        char nameBuff[100];
        int voters, parties;

        cout << "\n=== INPUT FOR DISTRICT #" << (i + 1) << " ===\n";

        cout << "District name: ";
        cin >> nameBuff;

        voters = getSafeInt("Registered voters count: ", 1);
        parties = getSafeInt("Number of parties: ", 1);

        ElectoralDistrict district(nameBuff, voters, parties);
        district.inputPartyData();
        district.saveToFile(outFile);
    }
    outFile.close();
    cout << "\n[OK] File 'districts.dat' created successfully.\n";

    ifstream inFile("districts.dat", ios::binary);
    if (!inFile) { cerr << "Error reading file!\n"; return 1; }

    int countFromFile;
    inFile.read((char*)&countFromFile, sizeof(countFromFile));

    if (inFile.gcount() < sizeof(int) || countFromFile <= 0) {
        cerr << "Error: File is empty or contains invalid data.\n";
        return 1;
    }

    ElectoralDistrict* districts = nullptr;
    try {
        districts = new ElectoralDistrict[countFromFile];
    }
    catch (bad_alloc& e) {
        cerr << "Error: Not enough memory to load data.\n";
        return 1;
    }

    for (int i = 0; i < countFromFile; i++) {
        districts[i].loadFromFile(inFile);
    }
    inFile.close();

    sortDistricts(districts, countFromFile);

    cout << "\n=== RESULTS BY DISTRICT (ALPHABETICAL) ===\n";
    for (int i = 0; i < countFromFile; i++) {
        districts[i].printStats();
        cout << "-----------------------\n";
    }

    ofstream noWinnerFile("no_winner_stats.txt");

    cout << "\n=== DISTRICTS WITHOUT A WINNER ===\n";
    if (noWinnerFile.is_open()) {
        noWinnerFile << "District | % Non-Voters\n";
        noWinnerFile << "-----------------------\n";
    }

    bool found = false;
    for (int i = 0; i < countFromFile; i++) {
        if (!districts[i].hasWinner()) {
            found = true;
            double nonVoters = districts[i].getNonVotersPercent();

            cout << districts[i].getName() << " (Non-voters: "
                << fixed << setprecision(2) << nonVoters << "%)\n";

            if (noWinnerFile.is_open()) {
                noWinnerFile << districts[i].getName() << " | "
                    << fixed << setprecision(2) << nonVoters << "%\n";
            }
        }
    }

    if (!found) cout << "All districts have a clear winner.\n";

    noWinnerFile.close();
    cout << "\n[OK] Report saved to 'no_winner_stats.txt'.\n";

    delete[] districts;

    return 0;
}