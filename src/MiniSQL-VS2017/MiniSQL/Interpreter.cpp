//
//  Interpreter.cpp
//  minisql
//
//  Created by 张倬豪 on 2017/5/20.
//  Copyright © 2017年 Icarus. All rights reserved.
//
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <vector>
#include "Interpreter.h"

using namespace std;


Interpreter::Interpreter() {
    cout << welcomeInfo;
}

Interpreter::~Interpreter() {
    cout << quitWord;
}

void Interpreter::mainLoop() {
    string sql = "", line = "";
    string::size_type semi_pos; // position of the first semicolon
    ifstream file;
    bool file_read = false; // whether there is a file reading now
    int status = 0;
    while (true) {
        double dur;
        clock_t start,end;
        start = clock();
        if (file_read) { // read the file
            
            try { // try to open the sql file
                file.open(fileName.c_str());
                if (!file.is_open()) throw fileOpenException(fileName);
            }
            catch (fileOpenException &f) {
                f.diagnostic();
                file_read = false;
                continue;
            }
            
            while(getline(file, sql, ';')) { // process sql
//                if (sql.find("\n") != string::npos) {
//                    sql = sql.substr(1, sql.size() - 1);
//                }
                tmp = 0;
                interprete(sql);
            }
            file.close();
            file_read = false;
        }
        else { // command line input
            sql = line + " ";
            
            if (line == "") cout << prompt; // the last sql did not leave some characters
            else cout << unfinishedPrompt; // the last sql left some characters
            
            while (getline(cin, line)) { // input line by line
                semi_pos = line.find_first_of(';');
                
                if (semi_pos == string::npos) {
                    sql += " " + line;
                    cout << unfinishedPrompt;
                }
                else {
                    sql += " " + line.substr(0, semi_pos);
                    line = line.substr(semi_pos + 1, line.size());
                    break;
                }
            }
            tmp = 0;
            status =  interprete(sql);
            
            if(status == 2) { // execute file
                file_read = true;
            }
            else if(status == 3) { // quit
                break;
            }
        }
        end = clock();
        dur = (double)(end - start);
        cout << "The duration time is " << dur / CLOCKS_PER_SEC << " seconds\n";
        if (cin.eof()) break;
    }
}

string Interpreter::getWord(string s, int *tmp) {
    string word;
    int start,end;
    
    while ((s[*tmp] == ' ' || s[*tmp] == '\n' || s[*tmp] == '\t') && s[*tmp] != 0) {
        (*tmp)++;
    }
    start = *tmp;
    
    if (s[*tmp] == '(' || s[*tmp] == ',' || s[*tmp] == ')') {
        (*tmp)++;
        end = *tmp;
        word = s.substr(start, end - start);
        return word;
    }
    else if (s[*tmp] == '"') {
        (*tmp)++;
        while (s[*tmp] != '"' && s[*tmp] != 0)
            (*tmp)++;
        if (s[*tmp] == '"') {
            start++;
            end = *tmp;
            (*tmp)++;
            word = s.substr(start, end - start);
            return word;
        }
        else {
            word = "";
            return word;
        }
    }
    else {
        while (s[*tmp] != ' ' &&s[*tmp] != '(' && s[*tmp] != 10 && s[*tmp] != 0 && s[*tmp] != ')' && s[*tmp] != ',')
            (*tmp)++;
        end = *tmp;
        if (start != end)
            word = s.substr(start, end - start);
        else
            word = "";
        
        return word;
    }
}

void Interpreter::create(string s) {
    int type, size;
    string word = "", table_name = "", attribute_name = "", index_name = "";
    bool if_unique;
    
    try {
        word = getWord(s, &tmp);
        if (word == "table") { // create table
            
            word = getWord(s, &tmp);
            if (!word.empty()) table_name = word; //create table tablename
            else throw syntaxErrorException(word);
            
            word = getWord(s, &tmp); // get left bracket
            if (word.empty() || word != "(") throw syntaxErrorException(word);
            
            
             // get attributes and primary keys
            word = getWord(s, &tmp);
            std::vector<Attribute> attribute_vector;
            
            while (!word.empty() && word != "primary" && word != ")") {
                attribute_name = word;
                type = 0;
                size = 0;
                if_unique = false;
                // deal with the data type
                word = getWord(s, &tmp);
                
                if (word == "int") {
                    type = INT;
                    size = sizeof(int);
                }
                else if(word == "float") {
                    type = FLOAT;
                    size = sizeof(float);
                }
                else if(word == "char") {
                    type = STRING;
                    word = getWord(s, &tmp);
                    if(word != "(") throw syntaxErrorException("Unknown Data Type");
                    word = getWord(s, &tmp);
                    
                    if (!isInt(word)) throw syntaxErrorException("Illegal number");
                    size = atoi(word.c_str());
                    word = getWord(s, &tmp);
                    if(word != ")") throw syntaxErrorException("Unknown Data Type");
                }
                else throw syntaxErrorException("Unknown or Missing Data Type");
                
                word = getWord(s, &tmp);
                if(word == "unique") {
                    if_unique = true;
                    word = getWord(s, &tmp);
                }
                Attribute attr(attribute_name, type, size, if_unique, false, false);
                attribute_vector.push_back(attr);
                if(word != ",") {
                    if(word != ")") throw syntaxErrorException(")");
                    else break;
                }
                word = getWord(s, &tmp);
            }
            if (word == "primary") {	// deal with primary key
                word = getWord(s, &tmp);
                if (word != "key") throw syntaxErrorException(word);
                word = getWord(s, &tmp);
                if (word == "(") {
                    word = getWord(s, &tmp);
                    int i = 0;
                    for(i = 0; i < attribute_vector.size(); i++) {
                        if(word == attribute_vector[i].name) {
                            attribute_vector[i].is_primary_key = true;
                            attribute_vector[i].is_unique = true;
                            attribute_vector[i].has_index = true;
                            attribute_vector[i].index_name = "PRIMARY_" + table_name;
                            break;
                        }
                    }
                    if (i == attribute_vector.size()) throw syntaxErrorException(word);
                    word = getWord(s, &tmp);
                    if (word != ")") throw syntaxErrorException(word);
                }
                else throw syntaxErrorException(word);
                word = getWord(s, &tmp);
                if (word != ")") throw syntaxErrorException(word);
            }
            ap->tableCreate(table_name,&attribute_vector);
            return;
        }
        else if (word == "index") {
            word = getWord(s, &tmp);
            if (!word.empty())			//create index indexname
                index_name = word;
            else throw syntaxErrorException(word);
            
            word = getWord(s, &tmp);
            if (word != "on") throw syntaxErrorException(word);
            
            word = getWord(s, &tmp);
            if (word.empty()) throw syntaxErrorException(word);
            
            table_name = word;
            
            word = getWord(s, &tmp);
            if (word != "(") throw syntaxErrorException(word);
            
            word = getWord(s, &tmp);
            if (word.empty()) throw syntaxErrorException(word);
            
            attribute_name = word;
            
            word = getWord(s, &tmp);
            if (word != ")") throw syntaxErrorException(word);
            
            ap->indexCreate(index_name, table_name, attribute_name);
        }
        else throw syntaxErrorException(word);
    }
    catch(syntaxErrorException &s) {
        s.diagnostic();
        return;
    }

    return;
}

void Interpreter::select(string s) {
    string word = "", table_name = "", attribute_name = "", value = "";
    vector<string> attr_selected;
    
    try {
        word = getWord(s, &tmp);
        
        if (word != "*") {	// select * only
            while(word != "from") {
                attr_selected.push_back(word);
                word = getWord(s, &tmp);
                if (word == ",") word = getWord(s, &tmp);
            }
        }
        else word = getWord(s, &tmp);
        
        if (word != "from") throw syntaxErrorException(word);
        
        word = getWord(s, &tmp);
        if (!word.empty())
            table_name = word;
        else throw syntaxErrorException(word);
        
        // condition extricate
        word = getWord(s, &tmp);
        if (word.empty()) {	// without condition
            if(attr_selected.size()==0){
                ap->recordShow(table_name, NULL, NULL);
            }
            else
                ap->recordShow(table_name,&attr_selected, NULL);
            return;
        }
        else if (word == "where") {
            Element value;
            int operate = OPERATOR_EQUAL;
            vector<Condition> condition_vector;
            word = getWord(s, &tmp);		//col1
            while(1){
                if(word.empty())
                    throw syntaxErrorException(word);
                attribute_name = word ;
                word = getWord(s, &tmp);
                if (word == "<=")
                    operate = OPERATOR_LESS_EQUAL;
                else if (word == ">=")
                    operate = OPERATOR_MORE_EQUAL;
                else if (word == "<")
                    operate = OPERATOR_LESS;
                else if (word == ">")
                    operate = OPERATOR_MORE;
                else if (word == "=")
                    operate = OPERATOR_EQUAL;
                else if (word == "<>")
                    operate = OPERATOR_NOT_EQUAL;
                else
                    throw syntaxErrorException(word);
                word = getWord(s, &tmp);
                if(word.empty()) // no condition
                    throw syntaxErrorException(word);
                if (isInt(word)) value = Element(atoi(word.c_str()));
                else if (isFloat(word)) value = Element((float)atof(word.c_str()));
                else {
                    Attribute att = ap->getAttribute(table_name, attribute_name);
                    if (att.name == "") throw syntaxErrorException("No such attribute_name");
                    value = Element(word, att.size);
                }
                
                Condition cond(attribute_name, value, operate);
                condition_vector.push_back(cond);
                word = getWord(s, &tmp);
                if(word.empty()) // no condition
                    break;
                if (word != "and")
                    throw syntaxErrorException(word);
                word = getWord(s, &tmp);
            }
            if(attr_selected.size()==0)
                ap->recordShow(table_name, NULL, &condition_vector);
            else
                ap->recordShow(table_name, &attr_selected, &condition_vector);
            
            return;
        }
    }
    catch(syntaxErrorException &s) {
        s.diagnostic();
        return;
    }
}

void Interpreter::drop(string s) {
    string word = "";
    
    try {
        word = getWord(s, &tmp);
        if (word == "table") {
            word = getWord(s, &tmp);
            if (!word.empty()) {
                ap->tableDrop(word);
                return;
            }
            else throw syntaxErrorException(word);
        }
        else if (word == "index") {
            word = getWord(s, &tmp);
            if (!word.empty()) {
                ap->indexDrop(word);
                return;
            }
            else throw syntaxErrorException(word);
        }
        else throw syntaxErrorException(word);
    }
    catch(syntaxErrorException &s) {
        s.diagnostic();
        return;
    }
}

void Interpreter::deleteCmd(string s) {
    string word = "", table_name = "", attribute_name = "", value = "";
    try {
        word = getWord(s, &tmp);
        if (word != "from") throw syntaxErrorException(word);
        
        word = getWord(s, &tmp);
        if (!word.empty()) table_name = word;
        else throw syntaxErrorException(word);
        
        // condition extricate
        word = getWord(s, &tmp);
        if (word.empty()) {	// without condition
            ap->recordDelete(table_name);
            return;
        }
        else if (word == "where") {
            Element value;
            int operate = OPERATOR_EQUAL;
            vector<Condition> condition_vector;
            word = getWord(s, &tmp);		//col1
            while(1){
                if(word.empty())
                    throw syntaxErrorException(word);
                attribute_name = word ;
                word = getWord(s, &tmp);
                if (word == "<=")
                    operate = OPERATOR_LESS_EQUAL;
                else if (word == ">=")
                    operate = OPERATOR_MORE_EQUAL;
                else if (word == "<")
                    operate = OPERATOR_LESS;
                else if (word == ">")
                    operate = OPERATOR_MORE;
                else if (word == "=")
                    operate = OPERATOR_EQUAL;
                else if (word == "<>")
                    operate = OPERATOR_NOT_EQUAL;
                else
                    throw syntaxErrorException(word);
                word = getWord(s, &tmp);
                if(word.empty()) // no condition
                    throw syntaxErrorException(word);
                if (isInt(word)) value = Element(atoi(word.c_str()));
                else if (isFloat(word)) value = Element((float)atof(word.c_str()));
                else {
                    Attribute att = ap->getAttribute(table_name, attribute_name);
                    if (att.name == "") throw syntaxErrorException("No such attribute_name");
                    value = Element(word, att.size);
                }
                
                Condition cond(attribute_name, value, operate);
                condition_vector.push_back(cond);
                word = getWord(s, &tmp);
                if(word.empty()) // no condition
                    break;
                if (word != "and")
                    throw syntaxErrorException(word);
                word = getWord(s, &tmp);
            }
            ap->recordDelete(table_name, &condition_vector);
            return;
        }
    }
    catch(syntaxErrorException &s) {
        s.diagnostic();
        return;
    }
}

void Interpreter::insert(string s) {
    string word = "", table_name = "";
    try {
        std::vector<string> value_vector;
        word = getWord(s, &tmp);
            if (word != "into")
                throw syntaxErrorException(word);
            word = getWord(s, &tmp);
            if (word.empty())
                throw syntaxErrorException(word);
            table_name = word;
            word = getWord(s, &tmp);
            if (word != "values")
                throw syntaxErrorException(word);
            word = getWord(s, &tmp);
            if (word != "(")
                throw syntaxErrorException(word);
            word = getWord(s, &tmp);
            while (!word.empty() && word != ")") {
                value_vector.push_back(word);
                word = getWord(s, &tmp);
                if (word == ",")  // bug here
                    word = getWord(s, &tmp);
            }
            if (word != ")") throw syntaxErrorException(word);
        ap->recordInsert(table_name,&value_vector);
        return;
    }
    catch(syntaxErrorException &s) {
        s.diagnostic();
        return;
    }
}

int Interpreter::interprete(string s) {
    string word;
    
    try {
        word = getWord(s, &tmp);
        if (word == "create") {
            create(s);
        }
        else if (word == "select") {
            select(s);
        }
        else if (word == "drop") {
            drop(s);
        }
        else if (word == "delete") {
            deleteCmd(s);
        }
        else if (word == "insert") {
            insert(s);
        }
        else if (word == "execfile") {
            fileName = getWord(s, &tmp);
            return 2;
        }
        else if (word == "quit") {
            return 3;
        }
        else if (word == "help") {
            cout << helpWord;
            return 0;
        }
        else throw syntaxErrorException(word);
    }
    catch (syntaxErrorException &s) {
        if (word != "") s.diagnostic();
        return 0;
    }
    
    return 0;
}

bool Interpreter::isInt(string str) {
    std::istringstream iss(str);
    int f;
    iss >> noskipws >> f;
    return iss.eof() && !iss.fail();
}

bool Interpreter::isFloat(string str) {
    std::istringstream iss(str);
    float f;
    iss >> noskipws >> f;
    return iss.eof() && !iss.fail();
}

void Interpreter::destruct() {
    IndexManager::instance().destruct();
    BufferManager::instance().flushBlock();
}