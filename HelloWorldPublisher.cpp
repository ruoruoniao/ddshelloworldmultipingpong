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

/**
 * @file HelloWorldPublisher.cpp
 *
 */

#include "HelloWorldPublisher.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

#include <thread>
#include "unordered_map"

#if defined(Server)
uint16_t current = 3;
uint16_t forward = 0;
#elif defined(Hub)
uint16_t current = 2;
uint16_t forward = 3;
#elif defined(Client)
uint16_t current = 1;
uint16_t forward = 2;
#endif

std::unordered_map<uint64_t, std::atomic<uint16_t>> listenedCount = {};

using namespace eprosima::fastdds::dds;

HelloWorldPublisher::HelloWorldPublisher()
        : participant_(nullptr), publisher_(nullptr), topic_(nullptr), writer_(nullptr),
          type_(new HelloWorldPubSubType()) {
}

bool HelloWorldPublisher::init(
        bool use_env) {
    hello_.index(0);
    hello_.from(current);
    hello_.to(forward);
    for (int i = 0; i < 1024 * 1024; ++i) {
        hello_.message().emplace_back(i % 255);
    }
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.transport().use_builtin_transports = true;
    pqos.transport().send_socket_buffer_size = 20 * 1024 * 1024;
    pqos.transport().listen_socket_buffer_size = 20 * 1024 * 1024;
    pqos.name("Participant_pub" + std::to_string(current));
    auto factory = DomainParticipantFactory::get_instance();

    if (use_env) {
        factory->load_profiles();
        factory->get_default_participant_qos(pqos);
    }

    participant_ = factory->create_participant(0, pqos);

    if (participant_ == nullptr) {
        return false;
    }

    //REGISTER THE TYPE
    type_.register_type(participant_);

    //CREATE THE PUBLISHER
    PublisherQos pubqos = PUBLISHER_QOS_DEFAULT;

    if (use_env) {
        participant_->get_default_publisher_qos(pubqos);
    }

    publisher_ = participant_->create_publisher(
            pubqos,
            nullptr);

    if (publisher_ == nullptr) {
        return false;
    }

    //CREATE THE TOPIC
    TopicQos tqos = TOPIC_QOS_DEFAULT;

    if (use_env) {
        participant_->get_default_topic_qos(tqos);
    }

    topic_ = participant_->create_topic(
            "HelloWorldTopic",
            "HelloWorld",
            tqos);

    if (topic_ == nullptr) {
        return false;
    }

    // CREATE THE WRITER
    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;
    wqos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
    wqos.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::DYNAMIC_REUSABLE_MEMORY_MODE;
    if (use_env) {
        publisher_->get_default_datawriter_qos(wqos);
    }

    writer_ = publisher_->create_datawriter(
            topic_,
            wqos,
            &listener_);

    if (writer_ == nullptr) {
        return false;
    }

    return true;
}

HelloWorldPublisher::~HelloWorldPublisher() {
    if (writer_ != nullptr) {
        publisher_->delete_datawriter(writer_);
    }
    if (publisher_ != nullptr) {
        participant_->delete_publisher(publisher_);
    }
    if (topic_ != nullptr) {
        participant_->delete_topic(topic_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void HelloWorldPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter *,
        const eprosima::fastdds::dds::PublicationMatchedStatus &info) {
    if (info.current_count_change == 1) {
        matched_ = info.total_count;
        firstConnected_ = true;
        std::cout << "Publisher matched." << std::endl;
    } else if (info.current_count_change == -1) {
        matched_ = info.total_count;
        std::cout << "Publisher unmatched." << std::endl;
    } else {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void HelloWorldPublisher::runThread(
        uint32_t samples,
        uint32_t sleep) {
    if (samples == 0) {
        while (!stop_) {
            if (current == 1) {
                if (publish(false)) {
                    std::cout << "Message with index: " << hello_.index()
                              << " SENT" << std::endl;
                }
                while (listenedCount[hello_.index()] != 2) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
                }
                listenedCount.erase(hello_.index());
            }
        }
    } else {
        for (uint32_t i = 0; i < samples; ++i) {
            if (!publish()) {
                --i;
            } else {
                std::cout << "Message with index: " << hello_.index()
                          << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    }
}

void HelloWorldPublisher::run(
        uint32_t samples,
        uint32_t sleep) {
    stop_ = false;
    std::thread thread(&HelloWorldPublisher::runThread, this, samples, sleep);
    if (samples == 0) {
        std::cout << "Publisher running. Please press enter to stop the Publisher at any time." << std::endl;
        std::cin.ignore();
        stop_ = true;
    } else {
        std::cout << "Publisher running " << samples << " samples." << std::endl;
    }
    thread.join();
}

void HelloWorldPublisher::listen(uint16_t from, uint16_t to, uint64_t index, const std::vector<char> &data) {
    std::string s = "Receive";
    if (from != current) {
        std::string type = "data";
        if (data.size() == 7){
            type = "receipt";
        }
        std::cout << "Message " << index << ", from " << from << ", to " << to << " " << type << " RECEIVED" << std::endl;
        if (data.size() != 7) {
            std::thread thread([this](uint64_t index, uint16_t from, const std::string &s) {
                HelloWorld helloWorld;
                helloWorld.index() = index;
                helloWorld.to() = from;
                helloWorld.from() = current;
                helloWorld.message() = {s.begin(), s.end()};
                writer_->write(&helloWorld);
                std::cout << "Message with index: " << index << " receipt SENT" << std::endl;
            }, index, from, s);
            thread.detach();
        } else if (to == current){
            listenedCount[index]++;
        }
    }

    if (from == 1 && current != 1 && forward != 0) {
        std::thread thread([this](uint64_t index, const std::vector<char> &data) {
            HelloWorld helloWorld;
            helloWorld.index() = index;
            helloWorld.to() = forward;
            helloWorld.from() = current;
            helloWorld.message() = data;
            writer_->write(&helloWorld);
            std::cout << "Message with index: " << index << " data SENT" << std::endl;
            listenedCount[index] = 0;
            while (listenedCount[index] != 2) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            listenedCount.erase(index);
        }, index, data);
        thread.detach();
    }
}

bool HelloWorldPublisher::publish(
        bool waitForListener) {
    if (listener_.firstConnected_ || !waitForListener || listener_.matched_ > 0) {
        hello_.index(hello_.index() + 1);
        listenedCount[hello_.index()] = 0;
        writer_->write(&hello_);
        return true;
    }
    return false;
}
