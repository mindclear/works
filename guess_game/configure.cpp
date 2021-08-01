
#include "configure.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <string>

Configure::Configure(const char* file)
{
    items_  = createHashTable(1024, nameHashCode, nameKeyCompare);
    loadFile(file);
}

Configure::~Configure()
{
	if(_ifs.is_open())
	{
		_ifs.close();
	}
    //FIXME:释放内存
	ListNode* cur = items_->Head();
    while (cur)
    {
        ListNode* next = cur->next;
		delete (char*)cur->key;
        delete (char*)cur->val;
        cur = next;
    }
    delete items_;
}

bool Configure::loadFile(const char* file)
{
	if (_ifs.is_open())
		_ifs.close();
	
	_ifs.open(file, std::ios::binary | std::ios::in);
	if (_ifs.fail() | !_ifs.is_open())
	{
        //LOG
		return false;
	}
	return parseConfig();
}

char* Configure::getString(const char* key)
{
    int nlen = strlen(key);
	if (0 == nlen)
	{
        //LOG
		return NULL;
	}

	char* get_val = (char*)items_->Get(key);
	if (get_val == NULL)
	{
        //LOG
		return NULL;
	}

    //FIMXE:拷贝
    return get_val;
}

int Configure::getInt(const char* key)
{
    char* buf = getString(key);
	if (buf == NULL)
        return 0;

	return atoi(buf);
}

bool Configure::parseConfig()
{
	if (_ifs.fail())
	{
        //LOG
		return false;
	}

	std::string line;
	while (getline(_ifs, line))
	{
		if (line.empty())
			continue;
        
		size_t pos = line.find("=");
		if(pos == std::string::npos)
			continue;

		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1);
        //FIXME:去除首尾空格
		if (!key.empty())
		{
            //FIXME:use map/string
            std::cout << line << " " << key << " " << value << std::endl;
            char* str_key = (char*)calloc(1, key.size() + 1);
            memcpy(str_key, key.c_str(), key.size());
            char* str_val = (char*)calloc(1, value.size() + 1);
            memcpy(str_val, value.c_str(), value.size());
            items_->Set(str_key, str_val);
        }
	}

	if (_ifs.is_open())
	{
		_ifs.close();
	}
	return true;
}