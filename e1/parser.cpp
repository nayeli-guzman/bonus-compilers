#include <iostream>
#include <stdexcept>
#include <list>
#include "token.h"
#include "scanner.h"
#include "ast.h"
#include "parser.h"

using namespace std;

// =============================
// Métodos de la clase Parser
// =============================

Parser::Parser(Scanner* sc) : scanner(sc) {
    previous = nullptr;
    current = scanner->nextToken();
    if (current->type == Token::ERR) {
        throw runtime_error("Error léxico");
    }
}

bool Parser::match(Token::Type ttype) {
    if (check(ttype)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(Token::Type ttype) {
    if (isAtEnd()) return false;
    return current->type == ttype;
}

bool Parser::advance() {
    if (!isAtEnd()) {
        Token* temp = current;
        if (previous) delete previous;
        current = scanner->nextToken();
        previous = temp;

        if (check(Token::ERR)) {
            throw runtime_error("Error lexico");
        }
        return true;
    }
    return false;
}

bool Parser::isAtEnd() {
    return (current->type == Token::END);
}


// =============================
// Reglas gramaticales
// =============================

Program* Parser::parseProgram() { // lista de intrucciones separadas por ;, cada [] es una instrucción
    Program * programa = new Program();
    programa->slist.push_back(parseStm());

    while(match(Token::SEMICOL)) {
        programa->slist.push_back(parseStm());
    }

    if (!isAtEnd()) {
        throw runtime_error("Error sintáctico");
    }
    
    cout << "Parseo exitoso" << endl;
    return programa;
}

Stm* Parser::parseStm() {
    Stm* stm;
    Exp* e;
    string nombre;
    if(match(Token::PRINT)) {
        match(Token::LPAREN);
        e = parseCE();
        match(Token::RPAREN);
        return new PrintStm(e);
    }
    else if(match(Token::ID)) {
        nombre = previous->text;
        match(Token::ASSIGN);
        e = parseCE();
        return new AssignStm(nombre, e);
    }
    return stm;
}

Exp* Parser::parseCE() {

    if (check(Token::LBRACE) || nextIsSetOpAfterPrimary()) {
        return parseSetE();
    }

    return parseE();
}


Exp* Parser::parseE() {
    
    Exp* l = parseT();

    while (match(Token::PLUS) || match(Token::MINUS)) {
        BinaryOp op;
        if (previous->type == Token::PLUS){
            op = PLUS_OP;
        }
        else{
            op = MINUS_OP;
        }
        Exp* r = parseT();
        l = new BinaryExp(l, r, op);
    }
    
    return l;
}


Exp* Parser::parseSet() {
    list<Exp*> elements;

    // Puede ser vacío
    if (!check(Token::RBRACE)) {
        elements.push_back(parseCE());
        while (match(Token::COMMA)) {       
            
            elements.push_back(parseCE());
        }
    }
    return new SetExp(elements);
}

Exp* Parser::parseSetE() {
    Exp* l = parseSetFactor();                
    while (match(Token::CUP) || match(Token::CAP) || match(Token::BACK)) {
        BinaryOp op = (previous->type == Token::CUP) ? CUP_OP
                     : (previous->type == Token::CAP) ? CAP_OP
                     : BACK_OP;
        Exp* r = parseSetFactor();
        l = new BinaryExp(l, r, op);
    }
    return l;
}

Exp* Parser::parseSetFactor() {
    if (match(Token::LBRACE)) {
        Exp* s = parseSet();
        match(Token::RBRACE);
        return s;
    }
    if (match(Token::LPAREN)) {
        Exp* e = parseSetE();                 
        match(Token::RPAREN);
        return e;
    }
    if (match(Token::ID)) {
        return new IdExp(previous->text);
    }
    throw runtime_error("Se esperaba un conjunto o '(...)' en SetExpr");
}

Exp* Parser::parseT() {
    Exp* l = parseF();
    while (match(Token::MUL) || match(Token::DIV)) {
        BinaryOp op;
        if (previous->type == Token::MUL){
            op = MUL_OP;
        }
        else{
            op = DIV_OP;
        }
        Exp* r = parseF();
        l = new BinaryExp(l, r, op);
    }
    return l;
}

Exp* Parser::parseF() {
    Exp* e; 
    if (match(Token::NUM)) {
        return new NumberExp(stoi(previous->text));
    } 
    else if (match(Token::LPAREN))
    {
        e = parseCE();
        match(Token::RPAREN);
        return e;
    }
    else if (match(Token::SQRT))
    {   
        match(Token::LPAREN);
        e = parseE();
        match(Token::RPAREN);
        return new SqrtExp(e);
    }
    else if (match(Token::ID))
    {
        return new IdExp(previous->text);
    }
    else {
        throw runtime_error("Error sintáctico");
    }
}


bool Parser::isSetOp(Token::Type t) {
    return t == Token::CUP || t == Token::CAP || t == Token::BACK;
}

bool Parser::nextIsSetOpAfterPrimary() {
    if (check(Token::ID)) {
        Token* t1 = scanner->peekToken(1);
        return t1 && isSetOp(t1->type);
    }
    if (check(Token::LPAREN)) {
        // saltar paréntesis balanceados con peeks
        int depth = 0;
        int k = 0;
        Token* tk = nullptr;
        do {
            tk = scanner->peekToken(++k);
            if (!tk) return false;
            if (tk->type == Token::LPAREN) depth++;
            else if (tk->type == Token::RPAREN) depth--;
        } while (depth >= 0 && tk->type != Token::RPAREN);
        Token* after = scanner->peekToken(k+1);
        return after && isSetOp(after->type);
    }
    return check(Token::LBRACE); // ya es SetExpr
}