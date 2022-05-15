#ifndef CLIENT_H
#define CLIENT_H
#include <array>
#include <fstream>
#include <string>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/asio.hpp>

class Logger
{
    Logger() {}
public:
    static Logger& instance();
    static void setOptions(std::string const& t_fileName, unsigned t_rotationSize,
        unsigned t_maxSize);
};


class Client
{
public:
    Client(boost::asio::io_context &t_context, boost::asio::ip::tcp::resolver::iterator t_endpointIterator,
           std::string const& t_path);
private:
    void openFile(std::string const& t_path);
    void doConnect();
    void doWriteFile(const boost::system::error_code& t_ec);
    template <class Buffer>
    void writeBuffer(Buffer& t_buffer);

    boost::asio::ip::tcp::resolver m_ioContext;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::resolver::iterator m_endpointIterator;
    enum {
        MessageSize = 1024
    };
    std::array<char, MessageSize> m_buf;
    boost::asio::streambuf m_request;
    std::ifstream m_sourceFile;
    std::string m_path;
};

template <typename Buffer>
void Client::writeBuffer(Buffer& t_buffer) {
    boost::asio::async_write(m_socket, t_buffer,
                             [this](boost::system::error_code ec, std::size_t length) -> void {
        doWriteFile(ec);
    });
}
#endif // CLIENT_H
