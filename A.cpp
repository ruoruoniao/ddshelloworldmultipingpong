#include "A.h"

void A::run() {
    char ch = ' ';
    do{
        if(ch == 'y'){
            int i = 0;
            while (true)
            {
                if(recvFlag == true){
                    entity.sendCommand(i,0,1);
                    recvFlag = false;
                    i++;
                } else
                {
                    Sleep(1);
                }
            }

        }
    } while (std::cin>>ch);
}

A::A() {
    entity.init("cmd","cmd");
    entity.listenersub_.setData = [&](unsigned long  index,unsigned long  triggerId,unsigned long  workerId,const std::vector<char> & payload)->void{
        if(workerId != id)
            return;
        data = payload;
        std::cout<<"a recv "<<index<<"\n";
        recvFlag = true;
    };
}
