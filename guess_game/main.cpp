#include <iostream>
#include <string>
#include <time.h>
#include "server.h"
#include "mysql_proxy.h"
#include "configure.h"

int main()
{
    //随机种子
    srand(time(NULL));
    //初始化配置
    Configure cfg("config.ini");

    //初始化数据库
    char* db_host = cfg.getString("db_host");
    char* db_user = cfg.getString("db_user");
    char* db_password = cfg.getString("db_password");
    char* db_name = cfg.getString("db_name");
    int db_port = cfg.getInt("db_port");
    MysqlProxy::GetInstance()->setAddress(db_host, db_user, db_password, db_name, db_port);
    int ret = MysqlProxy::GetInstance()->connect();
    // if (MYSQL_SUCC != ret)
    // {
    //     //LOG
    //     return -1;
    // }

    int win_score = cfg.getInt("win_score");
    int lose_score = cfg.getInt("lose_score");
    int tie_score = cfg.getInt("tie_score");

    EventLoop loop;
    GuessServer guess_server(&loop, 8999);
    guess_server.setScoreCfg(win_score, lose_score, tie_score);
    guess_server.startGame();
    std::cout << "GUESS GAME END" << std::endl;
    return 0;
}