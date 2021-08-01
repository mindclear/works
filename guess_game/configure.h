#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__

#include <iostream>
#include <sstream>
#include <fstream>
#include "util.h"

class Configure
{
public:
    Configure(const char* file);
	~Configure();

	bool loadFile(const char* file);
	char* getString(const char* key);
	int getInt(const char* key);

private:
	bool parseConfig();

private:
	std::ifstream _ifs;
    HashTable* items_; //FIXME:use map/string
};

#endif // __CONFIGURE_H__