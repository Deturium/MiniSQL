#include "stdafx.h"
#include "BufferManager.h"

//#include <utility>

Block* BlockPool::malloc(const std::string& file_name, uint32_t offset) {

	auto& item = _table.front();

	assert(item.second);

	item.second = false;

	Block* block_ptr = &_block_pool[item.first];
			
	// init Block
	block_ptr->file_name = file_name;
	block_ptr->offset = offset;
	block_ptr->pin_time = 0;
	block_ptr->is_dirty = false;

	_table.push_back(item);
	_table.pop_front();

	return block_ptr;

}

void BlockPool::free(Block* block_ptr) {
	int cnt = block_ptr - _block_pool;
	
	_table.remove(std::make_pair(cnt, false));
	_table.push_front(std::make_pair(cnt, true));
}


// the file must be existed
Block* BufferManager::readFile(const std::string& file_name, uint32_t offset) {

	// TODO: O_DIRECT
	FILE *fp = fopen(file_name.c_str(), "rb");
	if (fp == NULL) {
		std::cerr << "Error: open file " << file_name << " fail when read" << std::endl;
	}

	fseek(fp, offset, SEEK_SET);

	// will free in func writeFile
	Block *block_ptr = BlockPool::instance().malloc(file_name, offset);

	fread(block_ptr->buf, 1, BLOCK_SIZE, fp);
	fclose(fp);

	return block_ptr;
}

void BufferManager::writeFile(Block* block_ptr) {
	if (isDirty(block_ptr)) {

		FILE *fp = fopen(block_ptr->file_name.c_str(), "rb+");

		if (fp == NULL) {
			std::cerr << "Error: open file " << block_ptr->file_name << " fail when write" << std::endl;
		}

		fseek(fp, block_ptr->offset, SEEK_SET);
		fwrite(block_ptr->buf, 1, BLOCK_SIZE, fp);
		fclose(fp);

	}

	BlockPool::instance().free(block_ptr);
}


// the file must be exist
Block* BufferManager::readBlock(const std::string& file_name, uint32_t offset) {

    tag block_tag = make_pair(file_name, offset);
    auto t_it = table.find(block_tag);

    if (t_it != table.end()) {

        buffer.push_back(*(t_it->second));
		buffer.erase(t_it->second);

		bufferIter b_it = buffer.end();
		--b_it;

		table.find(block_tag)->second = b_it;

		return *b_it;

    } else {
        
		Block *block_ptr = readFile(file_name, offset);

		assert(block_ptr);

		buffer.push_back(block_ptr);

		bufferIter b_it = buffer.end();
		--b_it;

        table.insert(make_pair(block_tag, b_it));

		// LRU
        if (buffer.size() > BLOCK_MAX_NUM){ 
			bufferIter victim = buffer.begin();
            while ((*victim)->pin_time)
                victim++;
            writeBlock(*victim);
        }

		assert(buffer.size() <= BLOCK_MAX_NUM);

        return *b_it;
    }
}

void BufferManager::writeBlock(Block* block_ptr) {

	tag block_tag = make_pair(block_ptr->file_name, block_ptr->offset);

	writeFile(block_ptr);

	auto t_it = table.find(block_tag);
	if (t_it != table.end()) {
		buffer.erase(t_it->second);
		table.erase(t_it);
	}

}

void BufferManager::flushBlock() {
	for (auto& block_ptr : buffer) {
		writeFile(block_ptr);
	}

	buffer.clear();
	table.clear();
}

void BufferManager::deleteFile(const std::string& file_name) {
    auto t_it = table.lower_bound(make_pair(file_name, 0));

    while (t_it != table.end() && t_it->first.first == file_name) {
		auto tmp = t_it;
		t_it++;
		writeBlock(*(tmp->second));
    }

    remove(file_name.c_str());
}

