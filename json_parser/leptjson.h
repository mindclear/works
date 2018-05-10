#ifndef LEPTJSON_H_
#define LEPTJSON_H_

namespace LeptJson
{
#define NULL 0

// Type of JSON Value
enum Type
{
	TYPE_NULL,
	TYPE_FALSE,
	TYPE_TRUE,
	TYPE_NUMBER,
	TYPE_STRING,
	TYPE_ARRAY,
	TYPE_OBJECT
};

struct Value;

// String
struct SDString
{
	char* str;
	int len;

	SDString()
		:str(NULL), len(0)
	{}
};

typedef SDString MKey;
typedef Value MValue;
typedef double Number;

struct Member
{
	MKey key;
	MValue val;
};

// Object
struct Object
{
	Member* member;
	int size;
};

// Array
struct Array
{
	Value* arr;
	int size;
};

// JSON Value
struct Value
{
	union
	{
		Object obj;
		SDString sds;
		Array arr;
		Number num;
	};
	Type type;

	Value()
		:type(TYPE_NULL)
	{}
};

enum
{
	LEPT_PARSE_OK = 0,
	LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_INVALID_VALUE,
	LEPT_PARSE_ROOT_NOT_SINGULAR
};

class Document
{
public:
	int parse(Value* val, const char* json);

private:
	//skip \r \n \t ' '
	void skipWhitespace();
	//parse Json Value
	int parseValue(Value* val);

	// for parse true/false/null
	int parseLiteral(Value* val, const char* literal, Type type);
	int parseString(Value* val);
	int parseNumber(Value* val);
	int parseArray(Value* val);
	int parseObject(Value* val);

private:
	struct
	{
		const char* json;
		char* stack;
		size_t size, top;
	}context;
};

}

#endif // LEPTJSON_H_