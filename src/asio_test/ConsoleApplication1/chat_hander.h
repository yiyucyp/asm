#pragma once
#include "stdafx.h"
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <deque>

using namespace boost;
class chat_handler
    : public std::enable_shared_from_this<chat_handler>
{
public:
    chat_handler(asio::io_service& service)
        : service_(service)
        , socket_(service)
        , write_strand_(service)
    {}

    boost::asio::ip::tcp::socket& socket()//不能复制的
    {
        return socket_;
    }

    void start()
    {
        read_packet();
    }

    void send(std::string msg)
    {
        service_.post(write_strand_.wrap([me = shared_from_this(), msg]()
        {
            me->queue_message(msg);
        } ));
    }

    void queue_message(std::string msg)
    {
        bool write_in_progress = !send_packet_queue_.empty();
        send_packet_queue_.push_back(std::move(msg));

        if (!write_in_progress)
        {
            start_packet_send();
        }

    }

private:

    void start_packet_send()
    {
        send_packet_queue_.front() += "\0";
        boost::asio::async_write(socket_, asio::buffer(send_packet_queue_.front())
            , write_strand_.wrap([me = shared_from_this()](system::error_code const & ec, std::size_t) 
        {
            me->packet_send_done(ec);
        } ));
    }

    void packet_send_done(system::error_code const & error)
    {
        if (error) { return; }
        send_packet_queue_.pop_front();
        if (!send_packet_queue_.empty()) { start_packet_send(); }

    }

    void read_packet()
    {
        asio::async_read_until(socket_, in_packet_, '\0',
            [me =shared_from_this()](system::error_code const & ec, std::size_t byte)
        {
            me->read_pack_done(ec, byte);
        } );
    }
    void read_pack_done(system::error_code const& ec, std::size_t byte)
    {
        if (ec) { return; }

        std::istream stream(&in_packet_);
        std::string packet_string;
        stream >> packet_string;

        // show 

        read_packet();
    }


private:
    asio::io_service& service_;
    asio::ip::tcp::socket socket_;
    asio::io_service::strand write_strand_;
    asio::streambuf in_packet_;
    std::deque<std::string> send_packet_queue_;
};