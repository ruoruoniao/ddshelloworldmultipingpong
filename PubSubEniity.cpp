
//
// Created by MikuSoft on 2023/8/2.
// Copyright (c) 2023 SiYuanHongRui All rights reserved.
#include "PubSubEntity.h"
#include "HelloWorldPubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

using namespace eprosima::fastdds::dds;


PubSubEntity::PubSubEntity(): participant_(nullptr)
        , publisher_(nullptr)
        , topic_(nullptr)
        , writer_(nullptr)
        , reader_(nullptr)
        , type_(new HelloWorldPubSubType()) {
        for(int aa=0;aa<60000;aa++)
            st.message().emplace_back('1');
}

PubSubEntity::~PubSubEntity() {
    if (writer_ != nullptr)
    {
        publisher_->delete_datawriter(writer_);
    }
    if(reader_ != nullptr){
        subscriber_->delete_datareader(reader_);
    }
    if (publisher_ != nullptr)
    {
        participant_->delete_publisher(publisher_);
    }
    if(subscriber_ != nullptr){
        participant_->delete_subscriber(subscriber_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

bool PubSubEntity::init(std::string pubtopic, std::string subTopic) {
    /* Initialize data_ here */
    // Limit to 300kb per second.
    static const char* flow_controller_name = "example_flow_controller";
    auto flow_control_300k_per_sec = std::make_shared<eprosima::fastdds::rtps::FlowControllerDescriptor>();
    flow_control_300k_per_sec->name = flow_controller_name;
    flow_control_300k_per_sec->scheduler = eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO;
    flow_control_300k_per_sec->max_bytes_per_period = 300 * 1000;
    flow_control_300k_per_sec->period_ms = 1000;


    //CREATE THE PARTICIPANT
    DomainParticipantQos pqos;
    pqos.name("Participant_pub");
    // Increase the sending buffer size
    pqos.transport().send_socket_buffer_size = 60*1024*1024;
    // Increase the receiving buffer size
    pqos.transport().listen_socket_buffer_size = 60*1024*1024;

    pqos.flow_controllers().push_back(flow_control_300k_per_sec);

    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (participant_ == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    type_.register_type(participant_);

    //CREATE THE PUBLISHER
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (publisher_ == nullptr)
    {
        return false;
    }

    //CREATE THE TOPIC
    topic_ = participant_->create_topic(
            pubtopic,
            type_.get_type_name(),
            TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE WRITER
    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;
    wqos.publish_mode().kind = PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE;
    wqos.publish_mode().flow_controller_name = flow_controller_name;
    wqos.reliable_writer_qos().times.heartbeatPeriod.seconds = 0;
    wqos.reliable_writer_qos().times.heartbeatPeriod.nanosec = 1000000; //1 ms
    writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &listenerpub_);
    if (writer_ == nullptr)
    {
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //CREATE THE SUBSCRIBER
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        return false;
    }

    if(pubtopic!=subTopic){
        //CREATE THE TOPIC
        topic_ = participant_->create_topic(
                subTopic,
                type_.get_type_name(),
                TOPIC_QOS_DEFAULT);
        if (topic_ == nullptr)
        {
            return false;
        }
    }


    //CREATE THE READER
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    reader_ = subscriber_->create_datareader(topic_, rqos, &listenersub_);
    if (reader_ == nullptr)
    {
        return false;
    }
    return true;
}

void PubSubEntity::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched = info.total_count;
        std::cout << "DataWriter matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched = info.total_count;
        std::cout << "DataWriter unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void PubSubEntity::SubListener::on_data_available(eprosima::fastdds::dds::DataReader *reader) {
    // Take data
    HelloWorld st;
    SampleInfo info;

    if (reader->take_next_sample(&st, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.valid_data)
        {
            // Print your structure data here.
            ++samples;
           if(setData!= nullptr){
               setData(st.index(),st.triggerId(),st.workerId(),st.message());
           }
        }
    }
}

void PubSubEntity::SubListener::on_subscription_matched(eprosima::fastdds::dds::DataReader *reader,
                                                        const SubscriptionMatchedStatus &info) {
    if (info.current_count_change == 1)
    {
        matched = info.total_count;
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched = info.total_count;
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}


void PubSubEntity::sendCommand(int i,int triggerId,int workerId) {
    st.index() = i;
    st.triggerId()=triggerId;
    st.workerId()=workerId;
    writer_->write(&st);
}


