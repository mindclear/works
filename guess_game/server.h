#ifndef __SERVER_H__
#define __SERVER_H__

#include "net.h"
#include <stdio.h>
#include <string.h>
#include "util.h"

//玩家操作
enum
{
    GUESS_OP_TYPE_LIST = 1,     //查看在线用户
    GUESS_OP_TYPE_CREATE = 2,   //开设战局
    GUESS_OP_TYPE_JOIN = 3,     //加入战局
    GUESS_OP_TYPE_QUIT = 4,     //退出游戏
};

//玩家游戏状态
enum
{
    USER_STATUS_IDEL = 0,       //空闲
    USER_STATUS_WAIT = 1,       //等待加入
    USER_STATUS_GAMEING = 2,    //正在游戏
};

//FIXME:使用share_ptr管理Player对象
struct Player
{
    int score;      //玩家积分
    int status;     //当前状态
    int name_size;  //昵称长度
    char name[16];  //昵称
    int hash_code;  
    int table_id;   //牌桌id
    int seat_id;    //座位索引
    TcpConnection* conn;

    Player()
        :status(USER_STATUS_IDEL), hash_code(0), name_size(0), score(0), table_id(-1), seat_id(-1),
        conn(NULL)
    {
        memset(name, 0, 16);
    }

    void setTableInfo(int t_id, int s_id, int st)
    {
        table_id = t_id;
        seat_id = s_id;
        status = st;
    }
};

struct Table
{
    int table_id;           //牌桌id
    Player* players[2];     //玩家指针
    int command[2];        //玩家指令
    int score_change[2];    //结算积分变化
    time_t end_time;        //对局结束时间
    Table* idle_next;       //空闲桌子链表

    Table()
        :table_id(0)
    {
        init();
    }

    void init()
    {
        end_time = 0;
        idle_next = NULL;
        for (int i = 0; i < 2; ++i)
            command[i] = -1;
        memset(score_change, 0, sizeof(score_change));
        memset(players, 0, sizeof(players));
    }

    void clearTable()
    {
        for (int i = 0; i < 2; ++i)
        {
            if (players[i] != NULL)
                players[i]->setTableInfo(-1, -1, USER_STATUS_IDEL);
        }
        init();
    }
};

class GuessServer
{
public:
    GuessServer(EventLoop* loop, uint16_t port);
    ~GuessServer();
    void startGame();
    void setScoreCfg(int win_score, int lose_score, int tie_score);

private:
    void onConnection(TcpConnection* conn);
    void onMessage(TcpConnection* conn, Buffer* buf);
    void processRequest(TcpConnection* conn, char* pdata);
    bool decodeRequest(char* pdata, int& argc, char** argv);

private:
    bool checkValidName(const char* name);
    bool handleLogin(TcpConnection* conn, int argc, char** argv);
    bool handleShowList(TcpConnection* conn);
    bool handleJoinGame(Player* player, int argc, char** argv);
    bool handleGame(Player* player, int argc, char** argv);
    bool handleQuitGame(Player* player);

private:
    Player* getPlayer(const char* name);
    Table* getIdleTable();
    void recycleTable(Table* player);
    void recordGame(const Table* table);

private:
    Player* getPlayerFromDB(const char* name);
    bool updatePlayerToDB(const Player* player);
 
private:
    //事件循环
    EventLoop* loop_;
    TcpServer server_;

    //结算配置
    int win_score_;
    int lose_score_;
    int tie_score_;

    //玩家管理相关
    HashTable* players_;

    //桌子管理相关
    int table_index_;
    HashTable* tables_;
    Table* free_tables_;

    //输出文件
    FILE* out_file_;
};

#endif //__SERVER_H__