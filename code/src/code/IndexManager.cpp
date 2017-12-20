//
//  IndexManager.cpp
//  minisql_debug
//
//  Created by 张倬豪 on 2017/6/5.
//  Copyright © 2017年 Icarus. All rights reserved.
//

#include "IndexManager.h"

IndexManager::IndexManager() {

}

void IndexManager::destruct() {
    //write back to the disk
    for(intMap::iterator itInt = indexIntMap.begin();itInt != indexIntMap.end(); itInt ++) {
        if(itInt->second) {
            itInt -> second->writeBackToDiskAll();
            delete itInt->second;
        }
    }
    for(stringMap::iterator itString = indexStringMap.begin();itString != indexStringMap.end(); itString ++) {
        if(itString->second) {
            itString ->second-> writeBackToDiskAll();
            delete itString->second;
        }
    }
    for(floatMap::iterator itFloat = indexFloatMap.begin();itFloat != indexFloatMap.end(); itFloat ++) {
        if(itFloat->second) {
            itFloat ->second-> writeBackToDiskAll();
            delete itFloat->second;
        }
    }
    BufferManager::instance().flushBlock();
}

int IndexManager::createIndexFile(const std::string index_name)  {
    string indexFileName = "Ind_" + index_name + ".idx";

    FILE *fp;
    fp = fopen(indexFileName.c_str(), "w+");
    if (fp == NULL) {
        return 0;
    }
    fclose(fp);
    return 1;
}

void IndexManager::createIndex(const std::string index_name, int type, int size) {
    std::string fileName = "Ind_" + index_name + ".idx";
    int getKeySize(int type, int size);
    int getDegree(int size);
    int key_size = getKeySize(type, size);
    int degree = getDegree(key_size);
    switch (type) {
        case INT: {
            BPlusTree<int> *tree = new BPlusTree<int>(fileName, key_size, degree);
            indexIntMap.insert(intMap::value_type(fileName, tree));
            break;
        }
        case FLOAT: {
            BPlusTree<float> * tree = new BPlusTree<float>(fileName, key_size, degree);
            indexFloatMap.insert(floatMap::value_type(fileName, tree));
            break;
        }
        case STRING: {
            BPlusTree<string> * tree = new BPlusTree<string>(fileName, key_size, degree);
            indexStringMap.insert(stringMap::value_type(fileName, tree));
            break;
        }
        default:
            break;
    }
    return;
}

void IndexManager::dropIndex(const std::string index_name, int type) {
    
    std::string fileName = "Ind_" + index_name + ".idx";
    
    switch (type) {
        case INT: {
            intMap::iterator it = indexIntMap.find(fileName);
            delete it->second;
            indexIntMap.erase(it);
            // remove file
            break;
        }
        case FLOAT: {
            floatMap::iterator it = indexFloatMap.find(fileName);
            delete it->second;
            indexFloatMap.erase(it);
            // remove file
            break;
        }
        case STRING: {
            stringMap::iterator it = indexStringMap.find(fileName);
            delete it->second;
            indexStringMap.erase(it);
            // remove file
            break;
        }
        default:
            break;
    }
    return;
}

int getKeySize(int type, int size) {
    if(type == INT) return sizeof(int);
    else if (type == FLOAT) return sizeof(float);
    else if (type == STRING) return size;//size or size+1?
    return 0;
}

int getDegree(int key_size) {
    int degree = (BLOCK_SIZE - sizeof(uint32_t)) / (key_size + sizeof(offsetNumber));
    if (degree % 2 == 0) return degree - 1;
    return degree;
}

void IndexManager::insertIndex(const std::string index_name, int key, offsetNumber blockOffset, int type) {
    string filePath = "Ind_" + index_name + ".idx";
    intMap::iterator itInt = indexIntMap.find(filePath);
    if(itInt == indexIntMap.end()) {
        cout << "Error:in search index, no index " << filePath <<" exits" << endl;
        return;
    }
    else {
        itInt->second->insertKey(key, blockOffset);
    }
}

void IndexManager::insertIndex(const std::string index_name, float key, offsetNumber blockOffset, int type) {
    string filePath = "Ind_" + index_name + ".idx";
    floatMap::iterator itFloat = indexFloatMap.find(filePath);
    if(itFloat == indexFloatMap.end()) {
        cout << "Error:in search index, no index " << filePath <<" exits" << endl;
        return;
    }
    else {
        itFloat->second->insertKey(key, blockOffset);
    }
}

void IndexManager::insertIndex(const std::string index_name, string key, offsetNumber blockOffset, int type) {
    string filePath = "Ind_" + index_name + ".idx";
    stringMap::iterator itString = indexStringMap.find(filePath);
    if(itString == indexStringMap.end()) {
        cout << "Error:in search index, no index " << filePath <<" exits" << endl;
        return;
    }
    else {
        itString->second->insertKey(key, blockOffset);
    }
}
void IndexManager::deleteIndex(const std::string index_name, int key, int type) {
    string filePath = "Ind_" + index_name + ".idx";
    intMap::iterator itInt = indexIntMap.find(filePath);
    if(itInt == indexIntMap.end()) {
        cout << "Error:in search index, no index " << filePath <<" exits" << endl;
        return;
    }
    else {
        itInt->second->deleteKey(key);
    }
}

void IndexManager::deleteIndex(const std::string index_name, float key, int type) {
    string filePath = "Ind_" + index_name + ".idx";
    floatMap::iterator itFloat = indexFloatMap.find(filePath);
    if(itFloat == indexFloatMap.end()) {
        cout << "Error:in search index, no index " << filePath <<" exits" << endl;
        return;
    }
    else {
        itFloat->second->deleteKey(key);
    }
}

void IndexManager::deleteIndex(const std::string index_name, string key, int type) {
    string filePath = "Ind_" + index_name + ".idx";

    stringMap::iterator itString = indexStringMap.find(filePath);
    if(itString == indexStringMap.end()) {
        cout << "Error:in search index, no index " << filePath <<" exits" << endl;
        return;
    }
    else {
        itString->second->deleteKey(key);
    }
}

offsetNumber IndexManager::searchIndex(string index_name, int key,int type) {
    string filePath = "Ind_" + index_name + ".idx";
    intMap::iterator itInt = indexIntMap.find(filePath);
    if(itInt == indexIntMap.end()) {
        cout << "Error:in search index, no index " << filePath <<" exits" << endl;
        return -1;
    }
    else {
        return itInt->second->search(key);
    }
}

offsetNumber IndexManager::searchIndex(string index_name, float key,int type) {
    string filePath = "Ind_" + index_name + ".idx";
    floatMap::iterator itFloat = indexFloatMap.find(filePath);
    if(itFloat == indexFloatMap.end()) {
        cout << "Error:in search index, no index " << filePath <<" exits" << endl;
        return -1;
    }
    else {
        return itFloat->second->search(key);
    }
}

offsetNumber IndexManager::searchIndex(string index_name, string key,int type) {
    string filePath = "Ind_" + index_name + ".idx";
    stringMap::iterator itString = indexStringMap.find(filePath);
    if(itString == indexStringMap.end()) {
        cout << "Error:in search index, no index " << filePath <<" exits" << endl;
        return -1;
    }
    else {
        return itString->second->search(key);
    }
}
