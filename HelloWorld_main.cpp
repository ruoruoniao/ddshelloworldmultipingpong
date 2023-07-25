// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file HelloWorld_main.cpp
 *
 */

#include <limits>
#include <sstream>

#include "HelloWorldPublisher.h"
#include "HelloWorldSubscriber.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastrtps/log/Log.h>

#include <optionparser.hpp>

using eprosima::fastdds::dds::Log;

namespace option = eprosima::option;

struct Arg : public option::Arg
{
    static void print_error(
            const char* msg1,
            const option::Option& opt,
            const char* msg2)
    {
        fprintf(stderr, "%s", msg1);
        fwrite(opt.name, opt.namelen, 1, stderr);
        fprintf(stderr, "%s", msg2);
    }

    static option::ArgStatus Unknown(
            const option::Option& option,
            bool msg)
    {
        if (msg)
        {
            print_error("Unknown option '", option, "'\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Required(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0 && option.arg[0] != 0)
        {
            return option::ARG_OK;
        }

        if (msg)
        {
            print_error("Option '", option, "' requires an argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Numeric(
            const option::Option& option,
            bool msg)
    {
        char* endptr = 0;
        if ( option.arg != nullptr )
        {
            strtol(option.arg, &endptr, 10);
            if (endptr != option.arg && *endptr == 0)
            {
                return option::ARG_OK;
            }
        }

        if (msg)
        {
            print_error("Option '", option, "' requires a numeric argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    template<long min = 0, long max = std::numeric_limits<long>::max()>
    static option::ArgStatus NumericRange(
            const option::Option& option,
            bool msg)
    {
        static_assert(min <= max, "NumericRange: invalid range provided.");

        char* endptr = 0;
        if ( option.arg != nullptr )
        {
            long value = strtol(option.arg, &endptr, 10);
            if ( endptr != option.arg && *endptr == 0 &&
                    value >= min && value <= max)
            {
                return option::ARG_OK;
            }
        }

        if (msg)
        {
            std::ostringstream os;
            os << "' requires a numeric argument in range ["
               << min << ", " << max << "]" << std::endl;
            print_error("Option '", option, os.str().c_str());
        }

        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus String(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0)
        {
            return option::ARG_OK;
        }
        if (msg)
        {
            print_error("Option '", option, "' requires an argument\n");
        }
        return option::ARG_ILLEGAL;
    }

};

enum  optionIndex
{
    UNKNOWN_OPT,
    HELP,
    SAMPLES,
    INTERVAL,
    ENVIRONMENT
};

const option::Descriptor usage[] = {
    { UNKNOWN_OPT, 0, "", "",                Arg::None,
      "Usage: HelloWorldExample <publisher|subscriber>\n\nGeneral options:" },
    { HELP,    0, "h", "help",               Arg::None,      "  -h \t--help  \tProduce help message." },
    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nPublisher options:"},
    { SAMPLES, 0, "s", "samples",            Arg::NumericRange<>,
      "  -s <num>, \t--samples=<num>  \tNumber of samples (0, default, infinite)." },
    { INTERVAL, 0, "i", "interval",          Arg::NumericRange<>,
      "  -i <num>, \t--interval=<num>  \tTime between samples in milliseconds (Default: 100)." },
    { ENVIRONMENT, 0, "e", "env",            Arg::None,       "  -e \t--env   \tLoad QoS from environment." },
    { 0, 0, 0, 0, 0, 0 }
};

int main(
        int argc,
        char** argv)
{
    std::cout << "Starting " << std::endl;

    int type = 1;
    uint32_t count = 0;
    uint32_t sleep = 1;


    HelloWorldSubscriber mysub;
    mysub.init(false);
    HelloWorldPublisher mypub;
    if (mypub.init(false))
    {
        mysub.run([&mypub](uint16_t from, uint16_t to, uint64_t index, const std::vector<char>& data){
            mypub.listen(from, to, index, data);
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(4000));
        mypub.run(count, sleep);
    }
    Log::Reset();
    return 0;
}
