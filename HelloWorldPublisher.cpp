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

using namespace eprosima::fastdds::dds;

HelloWorldPublisher::HelloWorldPublisher()
        : participant_(nullptr), publisher_(nullptr), topic_(nullptr), writer_(nullptr),
          type_(new HelloWorldPubSubType()) {
}

bool HelloWorldPublisher::init(
        bool use_env) {
    hello_.index(0);
    for (int i = 0; i < 100000; ++i) {
        hello_.message().append("HelloWorld");
    }
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("Participant_pub");
    pqos.transport().listen_socket_buffer_size = 20 * 1024 * 1024;
    pqos.transport().send_socket_buffer_size = 20 * 1024 * 1024;
    static const char *flow_controller_name = "example_flow_controller";
    auto flow_control_300k_per_sec = std::make_shared<eprosima::fastdds::rtps::FlowControllerDescriptor>();
    flow_control_300k_per_sec->name = flow_controller_name;
    flow_control_300k_per_sec->scheduler = eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO;
    flow_control_300k_per_sec->max_bytes_per_period = 62.5 * 1000 * 100;
    flow_control_300k_per_sec->period_ms = 100;
    pqos.flow_controllers().push_back(flow_control_300k_per_sec);
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

//    topic1_ = participant_->create_topic(
//            "HelloWorldTopic",
//            "HelloWorld",
//            tqos);

    if (topic_ == nullptr) {
        return false;
    }

    // CREATE THE WRITER
    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;
    wqos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    wqos.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::DYNAMIC_REUSABLE_MEMORY_MODE;
    eprosima::fastrtps::rtps::Locator_t new_multicast_locator;
    eprosima::fastrtps::rtps::IPLocator::setIPv4(new_multicast_locator, "239.255.0.4");
    new_multicast_locator.port = 8012;
    new_multicast_locator.kind = LOCATOR_KIND_UDPv4;
    wqos.endpoint().multicast_locator_list.push_back(new_multicast_locator);
    wqos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;
    wqos.publish_mode().flow_controller_name = "example_flow_controller";
    if (use_env) {
        publisher_->get_default_datawriter_qos(wqos);
    }

    writer_ = publisher_->create_datawriter(
            topic_,
            wqos,
            &listener_);

    wqos = DATAWRITER_QOS_DEFAULT;
    wqos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    wqos.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::DYNAMIC_REUSABLE_MEMORY_MODE;
    wqos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;
    wqos.publish_mode().flow_controller_name = "example_flow_controller";
    new_multicast_locator = {};
    eprosima::fastrtps::rtps::IPLocator::setIPv4(new_multicast_locator, "239.255.0.4");
    new_multicast_locator.port = 8012;
    new_multicast_locator.kind = LOCATOR_KIND_UDPv4;
    wqos.endpoint().multicast_locator_list.push_back(new_multicast_locator);
    wqos.publish_mode().kind = ASYNCHRONOUS_PUBLISH_MODE;
    wqos.publish_mode().flow_controller_name = "example_flow_controller";

    writer1_ = publisher_->create_datawriter(
            topic_,
            wqos,
            &listener1_);

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
            if (publish(false)) {
                std::cout << "Message: " << " with index: " << hello_.index()
                          << " SENT" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
        }
    } else {
        for (uint32_t i = 0; i < samples; ++i) {
            if (!publish()) {
                --i;
            } else {
                std::cout << "Message: " << " with index: " << hello_.index()
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

bool HelloWorldPublisher::publish(
        bool waitForListener) {
    if (listener_.firstConnected_ || !waitForListener || listener_.matched_ > 0) {
        hello_.index(hello_.index() + 1);
        writer_->write(&hello_);
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
        writer1_->write(&hello_);
        return true;
    }
    return false;
}
