#include <iostream>
#include "net.h"

int main()
{
    std::cout << "GUESS GAME START" << std::endl;
    EventLoop loop;
    loop.loop();
    std::cout << "GUESS GAME END" << std::endl;
    return 0;
}