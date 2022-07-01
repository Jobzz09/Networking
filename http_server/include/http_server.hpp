#ifndef HTTP_SERVER
#define HTTP_SERVER

#include <string>
#include <memory>
#include <iostream>
#include <atomic>
#include <thread>
#include <boost/asio.hpp>

#include "http_acceptor.hpp"

class Http_server {
    public:
        Http_server();
        
        void Start(unsigned short, unsigned int);

        void Stop();
    private:
        boost::asio::io_context m_ioc;
        std::unique_ptr<boost::asio::io_context::work> m_work;
        std::unique_ptr<Http_acceptor> m_acceptor;
        std::vector<std::unique_ptr<std::thread>> m_threadPool;
};

#endif