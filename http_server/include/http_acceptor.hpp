#ifndef HTTP_ACCEPTOR
#define HTTP_ACCEPTOR

#include <string>
#include <memory>
#include <atomic>
#include <boost/asio.hpp>

class Http_acceptor {
    public:
        Http_acceptor(boost::asio::io_context&, unsigned short);
        void Start();
        void Stop();
    
    private:
        void AcceptConnection();

        boost::asio::io_context& m_ioc;
        boost::asio::ip::tcp::acceptor m_acceptor;
        std::atomic<bool> m_isStopped;
};

#endif