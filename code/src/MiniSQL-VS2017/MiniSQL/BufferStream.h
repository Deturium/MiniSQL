#pragma once

#include "BufferManager.h"

class BufferStream {

private:
	byte* _buf;
	size_t _cur;
	size_t _size;

public:
	BufferStream(byte* buf, size_t size)
		: _buf(buf)
		, _cur(0)
		, _size(size) {
	}

	BufferStream(Block* block)
		: _buf(block->buf)
		, _cur(0)
		, _size(BLOCK_SIZE) {

        assert(block);
	}

	~BufferStream() {
	}

	void set(uint32_t offset) {
		_cur = offset;
	}

	void seek(int offset) {
		_cur += offset;
	}

	size_t tell() {
		return _cur;
	}

	size_t size() {
		return _size;
	}

	size_t remain() {
		return _size - _cur;
	}

	template<typename T>
	//template<typename T, typename = std::enable_if_t<std::is_pod<std::decay_t<T>>::value>>
	void peek(T& obj) {
		assert(remain() >= sizeof(T));

		memcpy(reinterpret_cast<char*>(&obj), _buf + _cur, sizeof(T));
	}

	template<typename T>
	//template<typename T, typename = std::enable_if_t<std::is_pod<std::decay_t<T>>::value>>
	BufferStream& operator >> (T& obj) {
		assert(remain() >= sizeof(T));

		memcpy(reinterpret_cast<char*>(&obj), _buf + _cur, sizeof(T));
		_cur += sizeof(T);

		return *this;
	}

	template<typename T>
	//template<typename T, typename = std::enable_if_t<std::is_pod<std::decay_t<T>>::value>>
	BufferStream& operator << (T& obj) {
		assert(remain() >= sizeof(T));

		memcpy(_buf + _cur, reinterpret_cast<char*>(&obj), sizeof(T));
		_cur += sizeof(T);

		return *this;
	}

	 BufferStream& operator << (std::string& str) {
	 	assert(remain() >= str.size()+1);

	 	memcpy(_buf + _cur, str.c_str(), str.size()+1);
	 	_cur += str.size()+1;

	 	return *this;
	 }

	 BufferStream& operator >> (std::string& str) {

		 str.clear();
		 while (_buf[_cur] != '\0') {
			 assert(_cur < _size);
			 str.push_back(_buf[_cur]);
			 _cur++;
		 }
		 _cur++;

		 return *this;
	 }

};