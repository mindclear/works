#include "leptjson.h"
#include <assert.h>
#include <stdlib.h>

using namespace LeptJson;

#define EXPECT(c, ch) do { assert(*c->json == (ch)); c->json++; } while(0)

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
}

int Document::parseValue(Value * val)
{
	return LEPT_PARSE_OK;
}

int Document::parseLiteral(Value * val, const char * literal, Type type)
{
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
