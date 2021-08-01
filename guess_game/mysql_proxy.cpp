#include "mysql_proxy.h"
#include <sys/time.h>
#include <ostream>
#include <string.h>

MysqlProxy::MysqlProxy()
    :m_result_(NULL)
{
    //初始化连接
	mysql_init(&m_mysql_);
	m_bconn_ = false;
}

MysqlProxy::~MysqlProxy()
{
    //关闭数据库连接
	close();
}

MysqlProxy* MysqlProxy::GetInstance()
{
	static MysqlProxy instance;
	return &instance;
}
 
void MysqlProxy::setAddress(const char* host, const char* user, const char* password, const char* db_name, const unsigned int port)
{
	m_host_ = host;
	m_user_ = user;
	m_password_ = password;
	m_db_name_ = db_name;
	m_port_ = port;
}

int MysqlProxy::connect()
{
    //先判断是否已经连接了, 防止重复连接
    if (is_connected())
		return MYSQL_SUCC;

	mysql_options(&m_mysql_, MYSQL_SET_CHARSET_NAME, "utf8");

    //typedef my_bool char
	char reconnect = 1;
	mysql_options(&m_mysql_, MYSQL_OPT_RECONNECT, &reconnect);

	//timeout options
	int timeout = 1;
	mysql_options(&m_mysql_, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
	mysql_options(&m_mysql_, MYSQL_OPT_READ_TIMEOUT, &timeout);
	mysql_options(&m_mysql_, MYSQL_OPT_WRITE_TIMEOUT, &timeout);

    //连接数据库
	if (mysql_real_connect(&m_mysql_, m_host_, m_user_, m_password_, m_db_name_, m_port_, NULL, CLIENT_MULTI_STATEMENTS) == NULL)
	{
		//LOG
		return MYSQL_CONN_ERR;
    }

	//LOG
	m_bconn_ = true;
	return MYSQL_SUCC;
}
 
void MysqlProxy::close() 
{
	free_result();
	if (is_connected())
	{
		mysql_close(&m_mysql_);
	}
}
 
int MysqlProxy::selectQuery(const char* query_sql)
{
    //如果查询串是空指针,则返回
    int nlen = strlen(query_sql);
    if (NULL == query_sql || 0 == nlen) 
	{
        //LOG
		return MYSQL_SQLNULL_ERR;
    }

    //如果没有连接
	if (!is_connected() && connect() != MYSQL_SUCC)
	{
        //LOG
		return MYSQL_CONN_ERR;
	}
		
    //LOG
    try
    {
		//释放上一次的结果集
		free_result();

        //查询
        if (mysql_real_query(&m_mysql_, query_sql, nlen) != 0) 
		{
			//LOG
			return MYSQL_QUE_ERR;			
        }
      
        //取结果集
        m_result_ = mysql_store_result(&m_mysql_);
        if (m_result_ == NULL) 
		{
			//LOG
			return MYSQL_STORE_ERR;
        }
    } 
	catch (...) 
	{
        //LOG
		return MYSQL_UNKNOW_ERR;
    }

    //LOG
	return MYSQL_SUCC;
}
 
int MysqlProxy::updateQuery(const char* update_sql) 
{
    //如果查询串是空指针,则返回
    int nlen = strlen(update_sql);
	if (NULL == update_sql || 0 == nlen)
	{
        //LOG
		return MYSQL_SQLNULL_ERR;
    }

	//如果还没有连接
	if (!is_connected() && connect() != MYSQL_SUCC)
	{
		//LOG
		return MYSQL_CONN_ERR;
	}
		
	//LOG
    try
    {
        //释放上一次的结果集
		free_result();

        //查询, 实际上开始真正地修改数据库
		if (mysql_real_query(&m_mysql_, update_sql, nlen) != 0) 
		{
			//LOG
            return MYSQL_QUE_ERR;
		}

		do 
		{
			m_result_ = mysql_store_result(&m_mysql_);
			free_result(); 
		} while (!mysql_next_result(&m_mysql_));
    } 
	catch (...) 
	{
		//LOG
		return MYSQL_UNKNOW_ERR;
    }
	return MYSQL_SUCC;
}

char** MysqlProxy::fetch_row() 
{
    if (m_result_ == NULL)
        return NULL;

    //从结果集中取出一行
    m_row_ = mysql_fetch_row(m_result_);
    return m_row_;
}

void MysqlProxy::free_result()
{
    if (m_result_ != NULL)
	{
        mysql_free_result(m_result_);
        m_result_ = NULL;
    }
}

bool MysqlProxy::is_connected() 
{
    return m_bconn_;
}