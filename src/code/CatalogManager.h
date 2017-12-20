#pragma once

#include <map>
#include <cassert>
#include "DataStruct.h"
#include "Uncopyable.h"

class CatalogManager : Uncopyable {

private:
	std::map <std::string, Table> table_map;
	std::map<std::string, std::pair<std::string, std::string>> index_map;

	void serialize(const std::string& table_name, Table& t);

	void deserialize(Table& t);

public:
	CatalogManager();
	~CatalogManager();

	static CatalogManager& instance() {
		static CatalogManager instance;
		return instance;
	}

	bool hasTable(const std::string table_name);
	
	// call hasTable() first
	Table& getTable(const std::string table_name);
	
	Table& createTable(const std::string table_name, std::vector<Attribute>& attrs);
	
	void dropTable(const std::string table_name);

	bool hasIndex(const std::string index_name);

	std::pair<std::string, std::string> getIndex(const std::string index_name);

	void createIndex(const std::string index_name, const std::string table_name, const std::string attr_name);

	void dropIndex(const std::string index_name);
};