#pragma once

#include "BPlusTree.h"
#include "BufferStream.h"
#include "Uncopyable.h"
#include "DataStruct.h"
#include "CatalogManager.h"
#include <iostream>
#include <sstream>

class IndexManager : Uncopyable {

private:
//	void serialize(BufferStream& bs, Record& record);

//	void deserialize(BufferStream& bs, Record& record);
    
    typedef std::map<string,BPlusTree<int> *> intMap;
    typedef std::map<string,BPlusTree<string> *> stringMap;
    typedef std::map<string,BPlusTree<float> *> floatMap;
    intMap indexIntMap;
    stringMap indexStringMap;
    floatMap indexFloatMap;

public:
    IndexManager();
    void destruct();

	static IndexManager& instance() {
		static IndexManager instance;
		return instance;
	}

    void construct();

	int createIndexFile(const std::string index_name);


    void createIndex(const std::string index_name, int type, int size);

    void dropIndex(const std::string index_name, int type);

    void insertIndex(const std::string index_name, int key, offsetNumber blockOffset, int type);
	void insertIndex(const std::string index_name, float key, offsetNumber blockOffset, int type);
	void insertIndex(const std::string index_name, string key, offsetNumber blockOffset, int type);

	void deleteIndex(const std::string index_name, int key, int type);
	void deleteIndex(const std::string index_name, float key, int type);
	void deleteIndex(const std::string index_name, string key, int type);

	offsetNumber searchIndex(string filePath,int key,int type);
	offsetNumber searchIndex(string filePath,float key,int type);
	offsetNumber searchIndex(string filePath,string key,int type);

    //getIndexOffset();

};
