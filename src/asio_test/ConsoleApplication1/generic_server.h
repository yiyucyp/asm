#pragma once

#include "stdafx.h"
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using namespace boost;

template<typename ConnectionHander>
class AsioGenericServer
{
    using shared_handler_t = std::shared_ptr<ConnectionHander>;
public:
    AsioGenericServer(int thread_count = 1)
        : thread_count_(thread_count)
        , accepter_(io_service_)
    {}

    void StartServer(uint16_t port)
    {
        auto handler = std::make_shared<ConnectionHander>(io_service_);

        system::error_code error_code;
        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
        accepter_.open(endpoint.protocol(), error_code);
        accepter_.set_option(asio::tcp::acceptor::reuse_address(true), error_code);
        accepter_.bind(endpoint, error_code);
        accepter_.listen();

        accepter_.async_accept(handler->socket(), [=](auto ec)
        {
            handle_new_connection(handler, ec);
        });

        for (int i = 0; i < thread_count_; i++)
        {
            thread_pool_.emplace_back([=]() { io_service_.run(); });
        }
    }

private:
    void handle_new_connection(shared_handler_t handler, system::error_code const & error)
    {
        if (error) { return; }

        handler->start();

        auto new_handler = std::make_shared<ConnectionHander>(io_service_);
        accepter_.async_accept(new_handler->socket()
            , [=](auto ec)
              {
                handle_new_connection(new_handler, ec);
              }
        )
    }

    int thread_count_;
    std::vector<std::thread> thread_pool_;
    asio::io_service io_service_;
    asio::ip::tcp::acceptor accepter_;
};