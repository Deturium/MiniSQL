#include "stdafx.h"
#include "CatalogManager.h"
#include <fstream>

void CatalogManager::serialize(const std::string& table_name, Table& t) {
	std::ifstream fin(table_name+".cal");
	
	fin >> t.name >> t.record_size >> t.insert_offset;

	int attrs_size;
	fin >> attrs_size;

	for (int i = 0; i < attrs_size; i++) {
		Attribute tmp;
		fin >> tmp.name >> tmp.type >> tmp.size
			>> tmp.is_unique >> tmp.has_index 
			>> tmp.index_name >> tmp.is_primary_key;
		t.attrs.push_back(tmp);
	}

	fin.close();
}

void CatalogManager::deserialize(Table& t) {
	std::ofstream fout(t.name + ".cal");

	fout << t.name << ' ' << t.record_size << ' ' << t.insert_offset << ' ';

	fout << static_cast<int>(t.attrs.size()) << ' ';

	for (auto& walker : t.attrs) {
		fout << walker.name << ' ' << walker.type << ' ' << walker.size << ' '
			 << walker.is_unique << ' ' << walker.has_index << ' ' 
			 << walker.index_name << ' ' << walker.is_primary_key << ' ';
	}

	fout.close();
}

CatalogManager::CatalogManager() {
	
	std::ifstream fin("_index_map_");
	if (!fin)
		return;

	std::string index_name, table_name, attr_name;

	while (!fin.eof()) {
		fin >> index_name >> table_name >> attr_name;
		index_map.insert(std::make_pair(index_name, std::make_pair(table_name, attr_name)));
	}
}

CatalogManager::~CatalogManager() {
	for (auto& it : table_map) {
		deserialize(it.second);
	}

	//save index_map
	std::ofstream fout("_index_map_");
	for (auto& it : index_map) {
		fout << it.first << ' ' << it.second.first << ' '
			 << it.second.second << ' ';
	}
}

bool CatalogManager::hasTable(const std::string table_name) {
	if (table_map.count(table_name))
		return true;

	bool flag = true;
	std::ifstream f(table_name + ".cal");
	if (!f) {
		flag = false;
	}
	f.close();
	return flag;
}

Table & CatalogManager::getTable(const std::string table_name) {

	if (!table_map.count(table_name)) {
		// maybe move construct will be a better choose
		Table t;
		serialize(table_name, t);

		table_map.insert(std::make_pair(table_name, t));
	}

	return table_map.at(table_name);

	// table_map[table_name] // it will require a default constructor
}

Table & CatalogManager::createTable(const std::string table_name, std::vector<Attribute>& attrs) {
	//assert(!hasTable(table_name));

	//FILE* fp = fopen((table_name + ".cal").c_str(), "wb");
	//fclose(fp);
	
	table_map.insert(std::make_pair(table_name, Table(table_name, attrs)));
	return table_map.at(table_name);
}

void CatalogManager::dropTable(const std::string table_name) {
	auto it = table_map.find(table_name);
	if (it != table_map.end())
		table_map.erase(it);

	remove((table_name + ".cal").c_str());
}

bool CatalogManager::hasIndex(const std::string index_name) {
	return static_cast<bool>(index_map.count(index_name));
}

std::pair<std::string, std::string> CatalogManager::getIndex(const std::string index_name) {
	return index_map[index_name];
};

void CatalogManager::createIndex(const std::string index_name, const std::string table_name, const std::string attr_name) {
	Table& t = getTable(table_name);
	for (auto& walker : t.attrs) {
		if (walker.name == attr_name) {
			walker.has_index = true;
			walker.index_name = index_name;
			index_map.insert(std::make_pair(index_name, std::make_pair(table_name, attr_name)));
			break;
		}
	}
}

void CatalogManager::dropIndex(const std::string index_name) {
	auto it = index_map.find(index_name);
	
	std::string table_name = (*it).second.first;
	std::string attr_name = (*it).second.second;
	
	index_map.erase(it);

	Table& t = getTable(table_name);
	for (auto& walker : t.attrs) {
		if (walker.name == attr_name) {
			walker.has_index = false;
			walker.index_name.clear();
			break;
		}
	}
}