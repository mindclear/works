#ifndef __MYSQL_PROXY_H__
#define __MYSQL_PROXY_H__

#include <mysql/mysql.h>
#include <stdint.h>
#include <sstream>

enum MYSQL_ERR
{
	MYSQL_SUCC = 0,           //成功
	MYSQL_UNKNOW_ERR = -1,    //未知异常
	MYSQL_CONN_ERR = -2,      //连接错误
	MYSQL_SQLNULL_ERR = -3,   //查询语句为空
	MYSQL_QUE_ERR = -4,       //执行查询语句错误
	MYSQL_STORE_ERR = -5,     //存储查询结果错误
	MYSQL_PARAM_ERR = -6,     //参数错误
	MYSQL_QUEUE_ERR = -7,     //异步队列异常
	MYSQL_NOTSUPPORT_OPER = -8, //不支持的操作
	MYSQL_DUPLICATE_KEY = -9,   //主键冲突

};

//数据库操作类
class MysqlProxy
{
public:
	MysqlProxy();
	~MysqlProxy();
	//获取一个连接，单例模式
	static MysqlProxy* GetInstance();

public:
	//设置地址信息
	void setAddress(const char* host, const char* user, const char* password, const char* db_name, const unsigned int port);

	//连接mysql
	int connect();
 
	//关闭连接
    void close();
    
	//查询
    int selectQuery(const char* query_sql);
 
	//更新
	int updateQuery(const char* szSQL);
 
	//读取一个结果
    char** fetch_row();
 
	//释放结果集
	void free_result();

private:
    //是否已连接
	bool is_connected();
 
private:
	MYSQL m_mysql_;
	MYSQL_RES* m_result_; //结果集指针
	MYSQL_ROW m_row_; //一行,  typedef char **MYSQL_ROW;	

	bool        m_bconn_;  //是否连接
    const char* m_host_; //数据库服务器IP
	const char* m_user_; //用户名
	const char* m_password_; //口令
	const char* m_db_name_; //数据库名
	unsigned int m_port_; //端口
};

#endif //__MYSQL_PROXY_H__