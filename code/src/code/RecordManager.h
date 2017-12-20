#pragma once

#include "BufferManager.h"
#include "BufferStream.h"

class RecordManager : Uncopyable {

private:
	void serialize(BufferStream& bs, Record& record);

	void deserialize(Table& table, BufferStream& bs, Record& record);

	bool checkAttr(Element& ele, Condition& cond);

	bool check(Table& table, Record& rec, std::vector<Condition>& conditions);

public:
	RecordManager();
	~RecordManager();

	static RecordManager& instance() {
		static RecordManager instance;
		return instance;
	}

	void createTableFile(const std::string table_name);

	void dropTableFile(const std::string table_name);

	void insertRecord(const std::string table_name, Record& record);

	int deleteRecord(const std::string table_name, std::vector<Condition>& conditions);

	QueryResult selectRecord(const std::string table_name, std::vector<Condition>& conditions);

	// IndexManager::getIndexOffset()
	// QueryResult selectRecord(Table& table, std::set<uint32_t>& offsets, std::vector<Condition>& conditions);

	QueryResult selectAllRecords(const std::string table_name);

};
