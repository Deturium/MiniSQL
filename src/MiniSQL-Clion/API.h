#ifndef API_H
#define API_H

#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>

#include "DataStruct.h"
#include "RecordManager.h"
#include "CatalogManager.h"
#include "IndexManager.h"

using namespace std;

class API{
public:
    API();
    
    void tableDrop(string table_name);
    void tableCreate(string table_name, vector<Attribute>* attribute_vector);

    
    void indexDrop(string index_name);
	void indexCreate(string index_name, string table_name, string attribute_name);
  
	void recordShow(string table_name,  vector<string>* attribute_name_vector, vector<Condition>* condition_vector);

	void recordInsert(string table_name,vector<string>* record_content);

	void recordDelete(string table_name);
	void recordDelete(string table_name, vector<Condition>* condition_vector);

    Attribute& getAttribute(string table_name, string attribute_name);
    
private:
    void printQueryResults(QueryResult res, vector<string>* attribute_name_vector);
    bool isInt(string s);
    bool isFloat(string s);
    bool matchUnique(const string table_name, const Attribute att, const Element ele);
};

#endif /* API_h */
