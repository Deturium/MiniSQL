#include "RecordManager.h"
#include "CatalogManager.h"
#include "IndexManager.h"
#include "DataStruct.h"
#include <sstream>

// flags: do not add const
static char IS_DELETED = 'Y';
static char NOT_DELETED = 'N';
static char END_OF_RECORD = 'E';

// Table:
// | Record ... | end_flag |

// Record:
// | flag | next_offset | Element ... |

void RecordManager::serialize(BufferStream & bs, Record & record) {
	for (Element& ele : record) {
		switch (ele.type) {
		case(INT):
			bs << ele.data._int;
			break;
		case(FLOAT):
			bs << ele.data._float;
			break;
		case(STRING):
			bs << ele._string;
			bs.seek(ele.data._str_size - ele._string.size() - 1);
			break;
		default:
			assert(0);
			break;
		}
	}
}

void RecordManager::deserialize(Table& table, BufferStream & bs, Record & record) {

	for (Attribute& attr : table.attrs) {
		Element ele;

		switch (attr.type) {
		case(INT):
			ele.type = INT;
			bs >> ele.data._int;
			break;
		case(FLOAT):
			ele.type = FLOAT;
			bs >> ele.data._float;
			break;
		case(STRING):
			ele.type = STRING;
			bs >> ele._string;
			ele.data._str_size = attr.size;
			bs.seek(attr.size - ele._string.size() - 1);
			break;
		default:
			assert(0);
			break;
		}

		record.push_back(ele);
	}
}

bool RecordManager::checkAttr(Element& ele, Condition& cond) {
	bool flag;
	switch (cond.operation) {
		case(OPERATOR_EQUAL):
			flag = cond.value == ele;
			break;
		case(OPERATOR_NOT_EQUAL):
			flag = !(cond.value == ele);
			break;
		case(OPERATOR_LESS):
			flag = ele < cond.value;
			break;
		case(OPERATOR_MORE):
			flag = !(cond.value == ele) && !(ele < cond.value);
			break;
		case(OPERATOR_LESS_EQUAL):
			flag = (cond.value == ele) || (ele < cond.value);
			break;
		case(OPERATOR_MORE_EQUAL):
			flag = !(ele < cond.value);
			break;
		default:
			assert(0);
			break;
	}
	return flag;
}

bool RecordManager::check(Table& table, Record& rec, std::vector<Condition>& conditions) {
	bool flag = true;

	for (auto& cond : conditions) {

		int i = 0;
		for (auto& attr : table.attrs) {
			if (attr.name == cond.attr_name) {
				flag = checkAttr(rec[i], cond);
				break;
			}
			i++;
		}

		if (!flag) {
			break;
		}

	}

	return flag;
}


RecordManager::RecordManager() {
}

RecordManager::~RecordManager() {
}

void RecordManager::createTableFile(const std::string table_name) {
	FILE *fp = fopen((table_name + ".rec").c_str(), "wb");
	if (fp == NULL) {
		std::cerr << "Error: creat file " << table_name << " fail when removeRecord" << std::endl;
	}
	fclose(fp);
	Block *block_ptr = BufferManager::instance().readBlock(table_name + ".rec", 0);
	BufferStream bs(block_ptr);

	bs << END_OF_RECORD;

	BufferManager::instance().makeDirty(block_ptr);
}

void RecordManager::dropTableFile(const std::string table_name) {
	BufferManager::instance().deleteFile(table_name + ".rec");
}

void RecordManager::insertRecord(const std::string table_name, Record& record) {

	Table& table = CatalogManager::instance().getTable(table_name);

	uint32_t insert_offset = table.insert_offset;
	uint32_t next_offset, index_offset;
	char flag;
	int block_cnt = insert_offset / BLOCK_SIZE;

	Block *block_ptr = BufferManager::instance().readBlock(table_name + ".rec", block_cnt*BLOCK_SIZE);
	BufferStream bs(block_ptr);
	bs.set(insert_offset - block_cnt*BLOCK_SIZE);

	bs.peek(flag);

	assert(flag != NOT_DELETED);

	if (flag == IS_DELETED) {

		bs << NOT_DELETED;
		bs >> next_offset;

		index_offset = block_cnt*BLOCK_SIZE + bs.tell();
		serialize(bs, record);

	}

	if (flag == END_OF_RECORD) {

		if (bs.remain() < table.record_size + sizeof(char) * 2 + sizeof(uint32_t)) {
			// if this block is full
			block_cnt++;
			block_ptr = BufferManager::instance().readBlock(table_name + ".rec", block_cnt * BLOCK_SIZE);
            bs << NOT_DELETED;
			bs = BufferStream(block_ptr);
		}

		bs << NOT_DELETED;
		next_offset = block_cnt*BLOCK_SIZE + bs.tell() + sizeof(uint32_t) + table.record_size;
		bs << next_offset;

		index_offset = block_cnt*BLOCK_SIZE + bs.tell();
		serialize(bs, record);

		bs << END_OF_RECORD;
	}

	table.insert_offset = next_offset;

	BufferManager::instance().makeDirty(block_ptr);

	int i = 0;
	for (auto& attr : table.attrs) {
		if (attr.has_index) {
			switch(attr.type) {
				case (INT):
					IndexManager::instance().insertIndex(attr.index_name, record[i].data._int, index_offset, attr.type);
					break;
				case (FLOAT):
					IndexManager::instance().insertIndex(attr.index_name, record[i].data._float, index_offset, attr.type);
					break;
				case (STRING):
					IndexManager::instance().insertIndex(attr.index_name, record[i]._string, index_offset, attr.type);
					break;
				default:
					assert(1);
			}
		}
		i++;
	}
}

int RecordManager::deleteRecord(const std::string table_name, std::vector<Condition>& conditions) {
	Table& table = CatalogManager::instance().getTable(table_name);

	int block_cnt = 0;
	int delete_num_cnt = 0;
	Block *block_ptr = BufferManager::instance().readBlock(table_name + ".rec", 0);
	BufferStream bs(block_ptr);

	Record record;
	char flag;

	while(1) {
		bs >> flag;
		if (flag == IS_DELETED) {
			bs.seek(sizeof(uint32_t) + table.record_size);
			continue;
		}

		if (flag == END_OF_RECORD) {
			break;
		}

		//bs >> offset;
		bs.seek(sizeof(uint32_t));

		if (static_cast<int>(bs.remain()) <= table.record_size) {
            block_cnt++;
			block_ptr = BufferManager::instance().readBlock(table_name + ".rec", block_cnt*BLOCK_SIZE);
			bs = BufferStream(block_ptr);
            continue;
		}

		deserialize(table, bs, record);

		if (check(table, record, conditions)) {
			// mark it is deleted
			bs.seek(-static_cast<int>(table.record_size + sizeof(char) + sizeof(uint32_t)));
			bs << IS_DELETED;
			bs << table.insert_offset;

			table.insert_offset = static_cast<uint32_t>(block_cnt*BLOCK_SIZE + bs.tell() - sizeof(char)-sizeof(uint32_t));

			bs.seek(table.record_size);
			delete_num_cnt++;

			int i = 0;
			for (auto& attr : table.attrs) {
				if (attr.has_index) {
					switch(attr.type) {
						case (INT):
							IndexManager::instance().deleteIndex(attr.index_name, record[i].data._int, attr.type);
							break;
						case (FLOAT):
							IndexManager::instance().deleteIndex(attr.index_name, record[i].data._float, attr.type);
							break;
						case (STRING):
							IndexManager::instance().deleteIndex(attr.index_name, record[i]._string, attr.type);
							break;
						default:
							assert(1);
					}
				}
				i++;
			}

		}

		record.clear();
	}

	return delete_num_cnt;

}

QueryResult RecordManager::selectRecord(const std::string table_name, std::vector<Condition>& conditions) {
	Table& table = CatalogManager::instance().getTable(table_name);

	int block_cnt = 0;
	Block *block_ptr = BufferManager::instance().readBlock(table_name + ".rec", 0);
	BufferStream bs(block_ptr);

	QueryResult res;
	Record record;
	char flag;

	while(1) {
		bs >> flag;

		if (flag == IS_DELETED) {
			bs.seek(sizeof(uint32_t) + table.record_size);
			continue;
		}

		if (flag == END_OF_RECORD) {
			break;
		}

		//bs >> offset;
		bs.seek(sizeof(uint32_t));

		if (static_cast<int>(bs.remain()) <= table.record_size) {
            block_cnt++;
			block_ptr = BufferManager::instance().readBlock(table_name + ".rec", block_cnt*BLOCK_SIZE);
			bs = BufferStream(block_ptr);
            continue;
		}

		deserialize(table, bs, record);

		if (check(table, record, conditions)) {
			res.push_back(record);
		}
		record.clear();
	}

	// rvalue
	return res;
}

QueryResult RecordManager::selectAllRecords(const std::string table_name) {
	Table& table = CatalogManager::instance().getTable(table_name);

	int block_cnt = 0;
	Block *block_ptr = BufferManager::instance().readBlock(table_name + ".rec", 0);
	BufferStream bs(block_ptr);

	QueryResult res;
	Record record;
	char flag;

	while(1) {
		bs >> flag;

		if (flag == IS_DELETED) {
			bs.seek(sizeof(uint32_t) + table.record_size);
			continue;
		}

		if (flag == END_OF_RECORD) {
			break;
		}

		//bs >> offset;
		bs.seek(sizeof(uint32_t));

		if (static_cast<int>(bs.remain()) <= table.record_size) {
            block_cnt++;
			block_ptr = BufferManager::instance().readBlock(table_name + ".rec", block_cnt*BLOCK_SIZE);
			bs = BufferStream(block_ptr);
            continue;
		}

		deserialize(table, bs, record);

		res.push_back(record);
		record.clear();
	}

	return res;
}
