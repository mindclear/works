#include "leptjson.h"
#include <assert.h>
#include <stdlib.h>

using namespace LeptJson;

#define EXPECT(c, ch) do { assert(*c == (ch)); ++c; } while(0)

int Document::parse(Value * val, const char * json)
{
	assert(val != NULL);
	assert(json != NULL);
	context.json = json;
	context.stack = NULL;
	context.size = context.top = 0;

	skipWhitespace();
	return 0;
}

void Document::skipWhitespace()
{
	while (*context.json == ' ' ||
		*context.json == '\t' ||
		*context.json == '\r' ||
		*context.json == '\n')
		++context.json;
}

int Document::parseValue(Value * val)
{
	switch (*context.json)
	{
	case 'n':
		return parseLiteral(val, "null", TYPE_NULL);
	case 'f':
		return parseLiteral(val, "false", TYPE_FALSE);
	case 't':
		return parseLiteral(val, "true", TYPE_TRUE);
	case '\"':
		return parseString(val);
	default:
		break;
	}
	return LEPT_PARSE_INVALID_VALUE;
}

int Document::parseLiteral(Value * val, const char * literal, Type type)
{
	EXPECT(context.json, literal[0]);
	size_t i = 0;
	while (literal[i + 1])
	{
		if (context.json[i] != literal[i + 1])
			return LEPT_PARSE_INVALID_VALUE;
		++i;
	}
	context.json += i;
	val->type = type;
	return LEPT_PARSE_OK;
}

int Document::parseString(Value * val)
{
	return LEPT_PARSE_OK;
}

int Document::parseNumber(Value * val)
{
	return LEPT_PARSE_OK;
}

int Document::parseArray(Value * val)
{
	return LEPT_PARSE_OK;
}

int Document::parseObject(Value * val)
{
	return LEPT_PARSE_OK;
}
