#include "B.h"

void B::run() {
    while (true)
    {
        Sleep(1);
    }
}

B::B() {
    entity.init("cmd","cmd");
    entity.listenersub_.setData = [&](unsigned long  index,unsigned long  triggerId,unsigned long  workerId,const std::vector<char> & payload)->void{
        if(workerId != id)
            return;
        data = payload;
        std::cout<<"b recv "<<index<<"\n";
        entity.sendCommand(index,1,2);
    };
}
