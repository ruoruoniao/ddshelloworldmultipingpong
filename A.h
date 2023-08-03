//
// Created by MikuSoft on 2023/8/2.
// Copyright (c) 2023 SiYuanHongRui All rights reserved.
//
#ifndef generated_code__A_H
#define generated_code__A_H
#include "PubSubEntity.h"
class A{
public:
    A();
    void run();
private:
    int id=0;
    PubSubEntity entity;
    std::vector<char> data;
    bool recvFlag = true;
};
#undef Dll_Export_Import
#endif //generated_code__A_H