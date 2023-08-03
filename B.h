//
// Created by MikuSoft on 2023/8/2.
// Copyright (c) 2023 SiYuanHongRui All rights reserved.
//
#ifndef generated_code__B_H
#define generated_code__B_H
#include "PubSubEntity.h"
class B{
public:
    B();
    void run();
private:
    int id=1;
    PubSubEntity entity;
    std::vector<char> data;
};
#undef Dll_Export_Import
#endif //generated_code__B_H