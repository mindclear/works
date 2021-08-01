#include "server.h"
#include <iostream>
#include <functional>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>
#include "mysql_proxy.h"

static const int SERVER_BUCKET = 1024;
static const int BUCKET_MASK = 0x3ff;
static const char kCRLF[] = "\r\n";

char info[] = " 1.查看在线列表\r\n 2.创建战局\r\n 3.加入战局(3:昵称)\r\n 4:退出游戏\r\n";

//r:石头-0 s:剪刀-1 p:布-2
//0-平局 1-赢 -输
int game_rule[3][3] = 
{
    {0, 1, -1},
    {-1, 0, 1},
    {1, -1, 0},
};

int getCommandConde(char c)
{
    switch (c)
    {
    case 'r':
        return 0;
    case 's':
        return 1;
    case 'p':
        return 2;
    }
    return -1;
}

GuessServer::GuessServer(EventLoop* loop, uint16_t port)
    :loop_(loop), server_(loop, port), table_index_(0), free_tables_(NULL), out_file_(NULL),
    win_score_(0), lose_score_(0), tie_score_(0)
{
    //初始数据结构
    players_ = createHashTable(SERVER_BUCKET, nameHashCode, nameKeyCompare);
    tables_ = createHashTable(SERVER_BUCKET, NULL, NULL);

    //设置回调函数
    server_.setMessageCallback(std::bind(&GuessServer::onMessage, this, _1, _2));
    server_.setConnectionCallback(std::bind(&GuessServer::onConnection, this, _1));

    //对局记录文件
    out_file_ = fopen("record.txt", "a");
    assert(out_file_ != NULL);
}

GuessServer::~GuessServer()
{
    //FIXME:内存释放
    ListNode* cur = players_->Head();
    while (cur)
    {
        ListNode* next = cur->next;
        delete (Player*)cur->val;
        cur = next;
    }
    delete players_;

    cur = tables_->Head();
    while (cur)
    {
        ListNode* next = cur->next;
        delete (Table*)cur->val;
        cur = next;
    }
    delete tables_;
    
    //关闭文件
    fclose(out_file_);
}

void GuessServer::startGame()
{
    std::cout << "GUESS GAME START" << std::endl;
    server_.start();
    loop_->loop();
}

void GuessServer::setScoreCfg(int win_score, int lose_score, int tie_score)
{
    win_score_ = win_score;
    lose_score_ = lose_score;
    tie_score_ = tie_score;
}

void GuessServer::onConnection(TcpConnection* conn)
{
    if (conn->connected()) //新连接
    {
        //LOG
        char buf[128] = {0};
        int num = sprintf(buf, "请输入昵称\r\n");
        conn->send(buf, num);
    }
    else
    {
        //断开连接
        Player* player = (Player*)conn->getContext();
        if (player != NULL)
        {
            //LOG
            handleQuitGame(player);
            delete player;
        }
    }
}

void GuessServer::onMessage(TcpConnection* conn, Buffer* buf)
{
    //协议格式，数字表示操作类型，以\r\n结尾，例如加入战局 "3:user1\r\n"
    while (true)
    {
        int nread = buf->readableBytes();
        if (nread < 3)
            break;

        char* newline = (char*)memmem(buf->peek(), nread, kCRLF, 2);
        if (newline != NULL)
        {
            int body_size = newline - buf->peek();
            if (body_size > 0)
            {
                char* pdata = (char*)calloc(1, sizeof(char) * (body_size+1));
                memcpy(pdata, buf->peek(), body_size);
                processRequest(conn, pdata);
            }
            buf->retrieve(body_size + 2);
        }
    }
}

bool GuessServer::decodeRequest(char* pdata, int& argc, char** argv)
{
    argc = 0;
    argv[argc] = strtok(pdata,":");
    while (argv[argc] != NULL)
    {
        argc++;
        argv[argc] = strtok(NULL, ":");
        if (argc > 2)
            return false;
    }
    return true;
}

bool GuessServer::checkValidName(const char* name)
{
    //检查昵称有效性，昵称是字母和数字组合，不能以数字开头
    int nlen = strlen(name);
    if (nlen <= 0 || nlen > 16)
        return false;

    if (name[0] >= '0' && name[0] <= '9')
        return false;

    int digit_num = 0;
    int letter_num = 0;
    for (int i = 0; i < nlen; ++i)
    {
        if ((name[i] >= '0' && name[i] <= '9'))
            digit_num++;
        else if ((name[i] >= 'A' && name[i] <= 'Z') || (name[i] >= 'a' && name[i] <= 'z'))
            letter_num++;
    }
    if ((digit_num + letter_num) != nlen || letter_num == nlen)
        return false;
    return true;
}

bool GuessServer::handleLogin(TcpConnection* conn, int argc, char** argv)
{
    //只能包含昵称参数
    if (argc != 1)
    {
        //LOG
        std::cout << "handleLogin argc invalid!" << std::endl;
        return false;
    }

    //校验昵称有效性
    if (!checkValidName(argv[0]))
    {
        //LOG
        std::cout << "handleLogin checkValidName failed! name: " << argv[0] << std::endl;
        return false;
    }

    Player* cur = getPlayer(argv[0]);
    if (cur == NULL)
    {
        //新增玩家
        int nlen = strlen(argv[0]);
        cur = new Player();
        memcpy(cur->name, argv[0], nlen);
        cur->name_size = nlen;
        players_->Set(cur->name, cur);
        //保存数据库
        updatePlayerToDB(cur);
        std::cout << "create new player! name: " << argv[0]<< std::endl;
    }
    else
    {
        //LOG
        std::cout << "exists player! name: " << argv[0] << std::endl;
    }
    //记录tcp链接
    cur->conn = conn;
    //保存玩家指针在conn
    conn->setContext(cur);

    char buf[128] = {0};
    int num = sprintf(buf, "%s\t%d\t%d\r\n", cur->name, cur->score, cur->status);
    num += sprintf(buf, "%s", info);
    conn->send(buf, num);
    return true;
}

bool GuessServer::handleShowList(TcpConnection* conn)
{
    //FIXME:协议支持分页
    int total = 0;
    char buf[4096] = {0};
    total += sprintf(buf, "昵称\t总分\t状态（0-空闲 1-等待加入 2-游戏中）\r\n");

    int player_cnt = 0;
    ListNode* cur = players_->Head();
    while (cur != NULL) //遍历玩家列表
    {
        player_cnt++;
        Player* p = (Player*)cur->val;
        total += sprintf(buf + total, "%s\t%d\t%d\r\n", p->name, p->score, p->status);
        cur = cur->next;
        if (player_cnt >= 100)
        {
            conn->send(buf, total);
            player_cnt = 0;
            total = 0;
        }
    }
    if (total > 0)
        conn->send(buf, total);
    return true;
}

bool GuessServer::handleJoinGame(Player* player, int argc, char** argv)
{
    if (argc != 2)
    {
        //LOG
        std::cout << "invalid operation! user: " << player->name << std::endl;
        return false;
    }

    if (player->status == USER_STATUS_GAMEING)
    {
        //LOG
        std::cout << "can't join game! user: " << player->name << " status: " << player->status << std::endl; 
        return false;
    }

    int nlen = strlen(argv[1]);
    Player* join_player = getPlayer(argv[1]);
    if (join_player == NULL || join_player->status != USER_STATUS_WAIT)
    {
        //LOG
        char buf[64] = {0};
        int num = sprintf(buf, "can't join game! join_user:%s", argv[1]);
        std::cout << buf << std::endl;
        player->conn->send(buf, num);
        return false;
    }

    //先获取空闲桌子，没有再new
    Table* table = getIdleTable();
    if (NULL == table)
    {
        table = new Table();
        table->table_id = ++table_index_;
    }
    table->players[0] = join_player;
    table->players[1] = player;
    tables_->Set(&table->table_id, table);

    //设置玩家状态
    join_player->setTableInfo(table->table_id, 0, USER_STATUS_GAMEING);
    player->setTableInfo(table->table_id, 1, USER_STATUS_GAMEING);

    //rsp to client
    char buf[64] = {0};
    int num = sprintf(buf, "请出石头(r),剪刀(s),布(p)\r\n");
    for (int i = 0; i < 2; ++i)
        table->players[i]->conn->send(buf, num);
    return true;
}

bool GuessServer::handleGame(Player* player, int argc, char** argv)
{
    int comlen = strlen(argv[0]);
    if (1 != argc || 1 != comlen)
    {
        //LOG
        std::cout << "invalid operation! user: " << player->name << std::endl;
        return false;
    }

    //r:石头 s:剪刀 p:布
    char command = argv[0][0];
    int c_code = getCommandConde(command);
    if (-1 == c_code)
    {
        //LOG
        std::cout << "invalid command! user: " << player->name << " command: " << command << std::endl;
        return false;
    }
    
    if (player->status != USER_STATUS_GAMEING)
    {
        //LOG
        std::cout << "user not gameing! user: " << player->name << std::endl;
        return false;
    }

    int table_id = player->table_id;
    int seat = player->seat_id;
    Table* table = (Table*)tables_->Get(&table_id);
    if (NULL == table)
    {
        //LOG
        std::cout << "not found table! user: " << player->name << " table_id: " << table_id << std::endl;
        return false;
    }
    if (table->command[seat] != -1)
    {
        //LOG，重复输入
        std::cout << "user repeat input command! user: " << player->name << std::endl;
        return false;
    }

    table->command[seat] = c_code;

    int done = 0;
    for (int i = 0; i < 2; ++i)
    {
        if (table->command[i] != -1)
            done++;
    }

    //双方是否已输入完毕
    if (2 == done)
    {
        //计算输赢
        int game_ret = game_rule[table->command[0]][table->command[1]];
        switch (game_ret)
        {
        case 0:
            table->score_change[0] = tie_score_;
            table->score_change[1] = tie_score_;
            break;
        case 1:
            table->score_change[0] = win_score_;
            table->score_change[1] = lose_score_;
            break;
        case -1:
            table->score_change[0] = lose_score_;
            table->score_change[1] = win_score_;
            break;
        default:
            break;
        }
        for (int i = 0; i < 2; ++i)
        {
            //更新积分
            table->players[i]->score += table->score_change[i];
            updatePlayerToDB(table->players[i]);

            char buf[64] = {0};
            int num = sprintf(buf, "finish round! change:%d, total:%d\n", table->score_change[i], table->players[i]->score);
            table->players[i]->conn->send(buf, num);
        }
        table->end_time = time(NULL);
        //记录牌局
        recordGame(table);
        table->clearTable();
        recycleTable(table);
    }
    return true;
}

bool GuessServer::handleQuitGame(Player* player)
{
    if (NULL == player)
        return false;
    
    if (player->table_id != -1 && player->status == USER_STATUS_GAMEING) //游戏中
    {
        Table* table = (Table*)tables_->Get(&player->table_id);
        if (table != NULL)
        {
            //TODO:通知对手
            //LOG
            table->clearTable();
            recycleTable(table);
        }
    }
    players_->Del((void*)player->name);
    return true;
}

void GuessServer::processRequest(TcpConnection* conn, char* pdata)
{
    int argc = 0;
    char* argv[2] = {0};
    if (!decodeRequest(pdata, argc, argv) || 0 == argc)
    {
        //LOG
        std::cout << "decodeRequest failed!" << std::endl;
        return;
    }
    
    //LOG
    std::cout << "processRequest argc: " << argc << " argv: ";
    for (int i = 0; i < argc; ++i)
        std::cout << argv[i] << ",";
    std::cout << std::endl;

    Player* conn_player = (Player*)conn->getContext();
    if (conn_player == NULL) //未初始化信息
    {
        handleLogin(conn, argc, argv);
    }
    else
    {
        int op_type = -1;
        if (argv[0][0] >= '0' && argv[0][0] <= '9') //操作类型以数字开头
            op_type = atoi(argv[0]);

        char* user_name = conn_player->name;
        if (op_type > 0)
        {
            switch (op_type)
            {
            case GUESS_OP_TYPE_LIST:    //查看在线用户
            {
                handleShowList(conn);
                break;
            }
            case GUESS_OP_TYPE_CREATE:  //开设战局
            {
                if (argc != 1)
                {
                    //LOG
                    std::cout << "invalid operation! user: " << user_name << std::endl;
                    break;
                }
                if (conn_player->status != USER_STATUS_IDEL)
                {
                    //LOG
                    std::cout << "can't create game! user: " << user_name << " status: " << conn_player->status << std::endl;
                    break;
                }
                conn_player->status = USER_STATUS_WAIT;
                break;
            }
            case GUESS_OP_TYPE_JOIN:  //加入战局
            {
                handleJoinGame(conn_player, argc, argv);
                break;   
            }
            case GUESS_OP_TYPE_QUIT: //退出游戏
            {
                handleQuitGame(conn_player);
                conn->setContext(NULL);
                delete conn_player;
                break;
            }
            default:
            {
                //LOG
                std::cout << "invalid operation! user: " << user_name << " op_type: " << op_type << std::endl;
                break;
            }
            }
        }
        else
        {
            handleGame(conn_player, argc, argv);
        }  
    }
}

Player* GuessServer::getPlayer(const char* name)
{
    Player* cur = (Player*)players_->Get(name);
    if (cur != NULL)
        return cur;

    //从数据库读取
    cur = getPlayerFromDB(name);
    if (cur != NULL)
    {
        players_->Set(cur->name, cur);
        return cur;
    }
    return NULL;
}

Table* GuessServer::getIdleTable()
{
    if (NULL == free_tables_)
        return NULL;
    
    Table* cur = free_tables_;
    free_tables_ = free_tables_->idle_next;
    cur->idle_next = NULL;
    return cur;
}

void GuessServer::recycleTable(Table* table)
{
    if (NULL == free_tables_)
    {
        free_tables_ = table;
    }
    else
    {
        table->idle_next = free_tables_->idle_next;
        free_tables_ = table;
    }
}

void GuessServer::recordGame(const Table* table)
{
    int index = 0;
    char buf[1024] = {0};
    for (int i = 0; i < 2; ++i)
    {
        index += sprintf(buf + index, "%s|%d|%d|", table->players[i]->name, table->score_change[i], table->players[i]->score);
    }
    index += sprintf(buf + index, "%ld\n", table->end_time);
    int ret = fwrite(buf, index, 1, out_file_);
    if (ret < 0)
    {
        //LOG
        std::cout << "recordGame failed! " << buf;
        return;
    }
    fflush(out_file_);
}

Player* GuessServer::getPlayerFromDB(const char* name)
{
    //FIXME:escape
    char sql[128] = {0};
    sprintf(sql, "select name, score from user_basic where name='%s'", name);
    std::cout << sql << std::endl;

    int ret = MysqlProxy::GetInstance()->selectQuery(sql);
    if (ret != MYSQL_SUCC)
    {
        //LOG
        return NULL;
    }

    MYSQL_ROW m_row = MysqlProxy::GetInstance()->fetch_row();
    if (m_row == NULL)
    {
        //LOG
        return NULL;
    }

    //FIMEX:字段校验
    Player* player = new Player();
    player->name_size = strlen(m_row[0]);
    memcpy(player->name, m_row[0], player->name_size);
    player->score = atoi(m_row[1]);
    return player;
}

bool GuessServer::updatePlayerToDB(const Player* player)
{
    //FIXME:escape
    char sql[128] = {0};
    sprintf(sql, "REPLACE INTO user_basic (name, score) VALUES ('%s',%d)", player->name, player->score);
    std::cout << sql << std::endl;

    int ret = MysqlProxy::GetInstance()->updateQuery(sql);
    if (ret != MYSQL_SUCC)
    {
        //LOG
        std::cout << "updateQuery failed! " << sql << std::endl;
        return false;
    }
    return true;
}