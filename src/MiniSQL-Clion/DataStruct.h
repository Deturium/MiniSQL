#pragma once

#include <string>
#include <vector>
#include <map>
#include <cassert>

const int INT = 0;
const int FLOAT = 1;
const int STRING = 3;

struct Element {
	int type;
	union {
		int _int;
		float _float;
		int _str_size;
	} data;
	std::string _string;

	Element() {

	}

	Element(int i)
		: type(INT) {
		data._int = i;
	}

	Element(float i)
		: type(FLOAT) {
		data._float = i;
	}

	Element(std::string str, int str_size)
		: type(STRING)
		, _string(str) {
		data._str_size = str_size;
	}

	bool operator == (const Element& ele) {
		assert(type == ele.type);

		bool flag;
		switch (type) {
		case(INT):
			flag = data._int == ele.data._int;
			break;
		case(FLOAT):
			flag = data._float == ele.data._float;
			break;
		case(STRING):
			flag = _string == ele._string;
			break;
		default:
			assert(0);
			break;
		}

		return flag;
	}

	bool operator < (const Element& ele) {
		assert(type == ele.type);

		bool flag;
		switch (type) {
		case(INT):
			flag = data._int < ele.data._int;
			break;
		case(FLOAT):
			flag = data._float < ele.data._float;
			break;
		case(STRING):
			flag = _string < ele._string;
			break;
		default:
			assert(0);
			break;
		}

		return flag;
	}

};

const int OPERATOR_EQUAL = 0; // "="
const int OPERATOR_NOT_EQUAL = 1; // "<>"
const int OPERATOR_LESS = 2; // "<"
const int OPERATOR_MORE = 3; // ">"
const int OPERATOR_LESS_EQUAL = 4; // "<="
const int OPERATOR_MORE_EQUAL = 5; // ">="

struct Condition {
	std::string attr_name;
	Element value;
	int operation;

	Condition(std::string _attr_name, Element _value, int _operation)
	: attr_name(_attr_name)
	, value(_value)
	, operation(_operation) {
	}
};

struct Attribute {
	std::string name;
	int type;
	int size;
	bool is_unique;
	bool has_index;
    std::string index_name;
	bool is_primary_key;

	Attribute() {
	}

	Attribute(std::string _name, int _type, int _size, 
              bool _is_unique, bool _has_index, bool _is_primary_key)
		: name(_name)
		, type(_type)
		, size(_size)
		, is_unique(_is_unique)
		, has_index(_has_index)
		, index_name("_")
		, is_primary_key(_is_primary_key) {
	}
};

struct Table {
	std::string name;
	int record_size;
	std::vector<Attribute> attrs;
	uint32_t insert_offset;

	Table() {
	}

	Table(const std::string& table_name, std::vector<Attribute>& attributes)
		: name(table_name)
		, attrs(attributes)
		, record_size(0)
		, insert_offset(0) {
		for (auto& walker : attributes) {
			record_size += walker.size;
		}
	}
};

using Record = std::vector<Element>;
using QueryResult = std::vector<Record>;

