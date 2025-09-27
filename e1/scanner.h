#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include "token.h"
using namespace std;

class Scanner {
private:
    string input;
    int first;
    int current;
    std::deque<Token*> la_;

public:
    // Constructor
    Scanner(const char* in_s);

    // Retorna el siguiente token
    Token* nextToken();

    Token* peekToken(int k = 1);

    void ensureLookahead(size_t k);
    
    ~Scanner();

};

// Ejecutar scanner
void ejecutar_scanner(Scanner* scanner,const string& InputFile);

#endif // SCANNER_H