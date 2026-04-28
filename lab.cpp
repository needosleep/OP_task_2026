#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>

using std::string;
using std::unordered_map;
using std::vector;

void readNames(std::ifstream &fin, int &n, vector<string> &names) {
  fin >> n;
  names.resize(n);

  for (size_t i = 0; i < n; i++) {
    fin >> names[i];
  }
}

void initMaps(vector<string> &names, unordered_map<string, double> &spent,
              unordered_map<string, double> &owed) {
  for (string &name : names) {
    spent[name] = 0.0;
    owed[name] = 0.0;
  }
}

bool processTransactions(std::ifstream &fin, vector<string> &names,
                         unordered_map<string, double> &spent,
                         unordered_map<string, double> &owed) {
  auto start_time = std::chrono::steady_clock::now();

  string line;
  while (std::getline(fin, line)) {
    auto current_time = std::chrono::steady_clock::now();
    if (current_time - start_time > std::chrono::minutes(5)) {
        return false;
    }

    string normalized;
    bool in_space = false;
    for (char c : line) {
      if (std::isspace(static_cast<unsigned char>(c))) {
        if (!in_space) {
          normalized += ' ';
          in_space = true;
        }
      } else {
        normalized += c;
        in_space = false;
      }
    }

    if (!normalized.empty() && normalized.front() == ' ')
      normalized.erase(0, 1);
    if (!normalized.empty() && normalized.back() == ' ')
      normalized.pop_back();

    if (normalized.empty())
      continue;

    std::stringstream sstream(normalized);

    string name;
    double amount;

    sstream >> name >> amount;

    spent[name] += amount;

    string slash;
    sstream >> slash;

    vector<string> excluded;
    string rest;

    if (std::getline(sstream, rest)) {
      string ex_name;
      std::stringstream sstream2(rest);

      while (std::getline(sstream2, ex_name, ',')) {
        ex_name.erase(std::remove_if(ex_name.begin(), ex_name.end(), ::isspace),
                      ex_name.end());
        if (!ex_name.empty())
          excluded.push_back(ex_name);
      }
    }

    vector<string> consumers;

    for (string &n : names) {
      bool isExcluded = false;
      for (string &ex : excluded) {
        if (n == ex) {
          isExcluded = true;
          break;
        }
      }
      if (!isExcluded)
        consumers.push_back(n);
    }

    if (!consumers.empty()) {
        double shared = amount / consumers.size();

        for (string &p : consumers) {
          owed[p] += shared;
        }
    }
  }
  return true;
}

void printStats(std::ofstream &fout, vector<string> &names,
                unordered_map<string, double> &spent,
                unordered_map<string, double> &owed) {
  fout << std::fixed << std::setprecision(1);
  for (string &name : names) {
    fout << name << " " << spent[name] << " " << owed[name] << "\n";
  }
}

void settleDebts(std::ofstream &fout, vector<string> &names,
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

    fout << debtors[i].first << " " << amount << " " << payers[j].first << "\n";

    debtors[i].second -= amount;
    payers[j].second -= amount;

    if (debtors[i].second < epsilon)
      i++;
    if (payers[j].second < epsilon)
      j++;
  }
}

int main() {
  std::string inputFile;
  std::string outputFile = "output.txt";

  std::cout << "Write the input file name\n";
  std::cin >> inputFile;

  std::ifstream fin(inputFile);

  if (!fin)
    return 1;

  int n;
  vector<string> names;

  readNames(fin, n, names);

  unordered_map<string, double> spent, owed;
  initMaps(names, spent, owed);

  if (!processTransactions(fin, names, spent, owed)) {
      return 1;
  }

  std::ofstream fout(outputFile);
  if (!fout) {
      return 1;
  }

  printStats(fout, names, spent, owed);
  settleDebts(fout, names, spent, owed);

  std::cout << "The result is written to " << outputFile << "\n";

  return 0;
}
