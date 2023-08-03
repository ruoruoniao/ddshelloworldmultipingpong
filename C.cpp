#include "C.h"

void C::run() {
    while (true)
    {
        Sleep(1);
    }
}

C::C() {
   entity.init("cmd","cmd");
    entity.listenersub_.setData = [&](unsigned long  index,unsigned long  triggerId,unsigned long  workerId,const std::vector<char> & payload)->void{
        if(workerId != id)
            return;
        data = payload;
        std::cout<<"c recv "<<index<<"\n";
        entity.sendCommand(index,2,0);
    };
}
