#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <list>
#include <map>
#include <cassert>
#include "Uncopyable.h"
#include "DataStruct.h"

// replace #define
const int BLOCK_SIZE = 4096;
const int BLOCK_MAX_NUM = 512;

using byte = unsigned char;

struct Block : Uncopyable {
	std::string file_name;
	uint32_t offset;
	int pin_time;
	bool is_dirty;

    int using_size; // for IndexManager

	byte buf[BLOCK_SIZE];
};

class BlockPool {

private:
	Block* _block_pool;

	// true  -> free 
	// false -> be_used
	std::list <std::pair<int, bool>> _table;

public:
	static BlockPool& instance() {
		static BlockPool instance;
		return instance;
	}

	BlockPool() {
		try {
			_block_pool = new Block[BLOCK_MAX_NUM + 1];
		} catch (const std::bad_alloc&) {
			std::cerr << "Error: bad_alloc when creat BlockPool" << std::endl;
		}
		for (int i = 0; i < BLOCK_MAX_NUM + 1; i++) {
			_table.push_back(std::make_pair(i, true));
		}
	}

	Block* malloc(const std::string& file_name, uint32_t offset);
	void free(Block* block_ptr);
};


using tag = std::pair <std::string, uint32_t>;
using bufferIter = std::list <Block*>::iterator;

class BufferManager {

private:
    std::map <tag, bufferIter> table;
    std::list <Block*> buffer;

	Block* readFile(const std::string& file_name, uint32_t offset);

	void writeFile(Block* block_ptr);

public:

	static BufferManager& instance() {
		static BufferManager instance;
		return instance;
	}

	Block* readBlock(const std::string& file_name, uint32_t offset);

	void writeBlock(Block* block_ptr);

	void flushBlock();

	void deleteFile(const std::string& file_name);


	//inline func
	bool isDirty(const Block* block_ptr) {
		return block_ptr->is_dirty;
	}

	void makeDirty(Block* block_ptr) {
		block_ptr->is_dirty = true;
	}

	bool isPin(const Block* block_ptr) {
		return block_ptr->pin_time != 0;
	}

	void pinBlock(Block* block_ptr) {
		++(block_ptr->pin_time);
	}

	void unPinBlock(Block* block_ptr) {
		--(block_ptr->pin_time);
		assert(block_ptr->pin_time >= 0);
	}

};
