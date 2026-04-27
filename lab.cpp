#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

using std::cin;
using std::cout;
using std::vector;
using std::unordered_map;
using std::string;

void readNames(int &n, vector<string> &names) {
    cin >> n;
    names.resize(n);

    for (size_t i = 0; i < n; i++) {
        cin >> names[i];
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

void processTransactions(vector<string> &names,
                         unordered_map<string, double> &spent,
                         unordered_map<string, double> &owed) {
    cin.ignore();

    string line;
    while (std::getline(cin, line)) {
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

void printStats(vector<string> &names,
                unordered_map<string, double> &spent,
                unordered_map<string, double> &owed) {
    cout << std::fixed << std::setprecision(1);
    for (string &name : names) {
        cout << name << " " << spent[name] << " " << owed[name] << "\n";
    }
}

void settleDebts(vector<string> &names,
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

        cout << debtors[i].first << " "
             << amount << " "
             << payers[j].first << "\n";

        debtors[i].second -= amount;
        payers[j].second -= amount;

        if (debtors[i].second < epsilon) i++;
        if (payers[j].second < epsilon) j++;
    }
}

int main() {
    int n;
    vector<string> names;

    readNames(n, names);

    unordered_map<string, double> spent, owed;
    initMaps(names, spent, owed);

    processTransactions(names, spent, owed);

    printStats(names, spent, owed);

    settleDebts(names, spent, owed);

    return 0;
}
