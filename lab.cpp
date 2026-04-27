#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <fstream>

using std::vector;
using std::unordered_map;
using std::string;

void readNames(std::ifstream &fin, int &n, vector<string> &names) {
    fin >> n;
    names.resize(n);

    for (size_t i = 0; i < n; i++) {
        fin >> names[i];
    }
}

void initMaps(vector<string> &names,
              unordered_map<string, double> &spent,
              unordered_map<string, double> &owed) {
    for (string &name : names) {
        spent[name] = 0.0;
        owed[name] = 0.0;
    }
}

void processTransactions(std::ifstream &fin, 
                         vector<string> &names,
                         unordered_map<string, double> &spent,
                         unordered_map<string, double> &owed) {
    fin.ignore();

    string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;

        std::stringstream sstream(line);

        string name;
        double amount;

        sstream >> name >> amount;

        spent[name] += amount;

        string slash;
        sstream >> slash;

        vector<string> excluded;
        string rest;

        if (std::getline(sstream, rest)) {
            string name;
            std::stringstream sstream2(rest);

            while (std::getline(sstream2, name, ',')) {
                name.erase(std::remove_if(name.begin(), name.end(), ::isspace), name.end());
                if (!name.empty())
                    excluded.push_back(name);
            }
        }

        vector<string> consumers;

        for (string &name : names) {
            bool isExcluded = false;
            for (string &ex : excluded) {
                if (name == ex) {
                    isExcluded = true;
                    break;
                }
            }
            if (!isExcluded)
                consumers.push_back(name);
        }

        double shared = amount / consumers.size();

        for (string &p : consumers) {
            owed[p] += shared;
        }
    }
}

void printStats(std::ofstream &fout,
                vector<string> &names,
                unordered_map<string, double> &spent,
                unordered_map<string, double> &owed) {
    fout << std::fixed << std::setprecision(1);
    for (string &name : names) {
        fout << name << " " << spent[name] << " " << owed[name] << "\n";
    }
}

void settleDebts(std::ofstream &fout, 
                 vector<string> &names,
                 unordered_map<string, double> &spent,
                 unordered_map<string, double> &owed) {
    vector<std::pair<string, double>> payers, debtors;
    const double epsilon = 1e-4;

    for (string &name : names) {
        double balance = spent[name] - owed[name];

        if (balance > epsilon)
            payers.push_back({name, balance});
        else if (balance < -epsilon)
            debtors.push_back({name, -balance});
    }

    int i = 0, j = 0;

    while (i < debtors.size() && j < payers.size()) {
        double amount = std::min(debtors[i].second, payers[j].second);

        fout << debtors[i].first << " "
             << amount << " "
             << payers[j].first << "\n";

        debtors[i].second -= amount;
        payers[j].second -= amount;

        if (debtors[i].second < epsilon) i++;
        if (payers[j].second < epsilon) j++;
    }
}

int main() {
    std::string inputFile;
    std::string outputFile = "output.txt";

    std::cout << "Write the input file name\n";
    std::cin >> inputFile;

    std::ifstream fin(inputFile);
    std::ofstream fout(outputFile);

    if (!fin) return 1;

    int n;
    vector<string> names;

    readNames(fin, n, names);

    unordered_map<string, double> spent, owed;
    initMaps(names, spent, owed);

    processTransactions(fin, names, spent, owed);

    printStats(fout, names, spent, owed);

    settleDebts(fout, names, spent, owed);

    std::cout << "The result is written to output.txt\n";

    return 0;
}
