//
//  Interpreter.hpp
//  minisql
//
//  Created by 张倬豪 on 2017/5/20.
//  Copyright © 2017年 Icarus. All rights reserved.
//

#ifndef Interpreter_hpp
#define Interpreter_hpp

#include <iostream>
#include "API.h"
//#include "MiniSQL.h"
#include "DataStruct.h"

using namespace std;

#define welcomeInfo "Welcome to the MiniSQL Monitor\nCopyright 2017 saltfishpool@ZJU. All rights reserved.\nType \"help\" for help \n"
#define prompt "MiniSQL> "
#define unfinishedPrompt "      -> "
#define helpWord "Author: saltfishzzh, Hydrogen\n\nList of all MiniSQL commands\ncreate, select, drop, delete, insert, exec, quit, help\n"
#define quitWord "\nBYE\n"

class Interpreter {
public:
    
    Interpreter();
    virtual ~Interpreter();
    
    void mainLoop();

    void destruct();
    
    int interprete(string sql); 
    
    string getWord(string s, int *tmp);
    
private:
    API *ap;
    string fileName;
    int tmp = 0;
    void create(string s);
    void select(string s);
    void drop(string s);
    void deleteCmd(string s); // delete is the preserved keyword
    void insert(string s);
    bool isInt(string s);
    bool isFloat(string s);
};

class syntaxErrorException{
public:
    syntaxErrorException(string word):badWord(word) {}
    ~syntaxErrorException(){}
    void diagnostic() {cout << "Syntax Error on:" << (badWord == "" ? "Empty Command or Name" : badWord) << endl;}
private:
    string badWord;
};

class fileOpenException{
public:
    fileOpenException(string name): badFileName(name) {}
    ~fileOpenException(){}
    void diagnostic() {cout << "Fail to open file:" << badFileName << endl;}
private:
    string badFileName;
};

#endif /* Interpreter_hpp */
