// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file HelloWorldPubSubMain.cpp
 * This file acts as a main entry point to the application.
 *
 * This file was generated by the tool fastcdrgen.
 */


#include "A.h"
#include "B.h"
#include "C.h"

int main(int argc,char** argv)
{
    int type = 0;

    if (argc == 2)
    {
        if (strcmp(argv[1], "a") == 0)
        {
            type = 1;
        }
        else if (strcmp(argv[1], "b") == 0)
        {
            type = 2;
        }
        else if (strcmp(argv[1], "c") == 0)
        {
            type = 3;
        }
    }

    // Register the type being used

    switch (type)
    {
        case 1:
        {
            A a;
            a.run();
            break;
        }
        case 2:
        {
            B b;
            b.run();
            break;
        }
        case 3:
        {
            C c;
            c.run();
            break;
        }
    }
    return 0;
}