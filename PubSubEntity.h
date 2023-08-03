//
// Created by MikuSoft on 2023/8/2.
// Copyright (c) 2023 SiYuanHongRui All rights reserved.
//
#ifndef generated_code__o_H
#define generated_code__o_H

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

#include <thread>
#include <chrono>
#include<windows.h>
#include "HelloWorld.h"

class PubSubEntity
{
public:

    PubSubEntity();

    virtual ~PubSubEntity();

    bool init(std::string pubtopic,std::string subTopic);

    void sendCommand(int i,int triggerId,int workerId);

private:

    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::dds::Publisher* publisher_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataWriter* writer_;
    eprosima::fastdds::dds::TypeSupport type_;


    eprosima::fastdds::dds::Subscriber* subscriber_;
    eprosima::fastdds::dds::DataReader* reader_;

    HelloWorld st;


public:
    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        SubListener() = default;

        ~SubListener() override = default;

        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

        int matched = 0;
        uint32_t samples = 0;
        std::function<void(unsigned long  index,unsigned long  triggerId,unsigned long  workerId,const std::vector<char> & payload)> setData;
    } listenersub_;

    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        PubListener() = default;

        ~PubListener() override = default;

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        int matched = 0;
    }listenerpub_;


};
#endif //generated_code__o_H