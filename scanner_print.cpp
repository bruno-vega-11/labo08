#include "scanner.h"
#include <fstream>
#include <iostream>

int main(int argc, const char* argv[]) {
    if (argc != 2) return 1;
    std::ifstream infile(argv[1]);
    if (!infile.is_open()) return 1;
    std::string s,line;
    while (std::getline(infile,line)) s += line + '\n';
    infile.close();
    Scanner sc(s.c_str());
    return ejecutar_scanner(&sc, std::string(argv[1]));
}
