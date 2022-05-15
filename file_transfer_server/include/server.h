#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <array>
#include <iostream>
#include <fstream>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/utility/setup/file.hpp>

class Logger
{
    Logger() {}
public:
    static Logger& instance();
    static void setOptions(std::string const& t_fileName, unsigned t_rotationSize,
        unsigned t_maxSize);
};


class Session
        : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::ip::tcp::socket t_socket);

    void start() {
        doRead();
    }

private :
    void doRead();
    void processRead(std::size_t t_bytesTransfered);
    void createFile();
    void readData(std::istream &t_stream);
    void doReadFileContent(std::size_t t_bytesTransfered);
    void handleError(std::string const& t_functionName, boost::system::error_code const& t_ec);

    boost::asio::ip::tcp::socket m_socket;
    enum {
        MaxLength = 4096
    };
    std::array<char, MaxLength> m_buf;
    boost::asio::streambuf m_requestBuf;
    std::ofstream m_outputFile;
    std::size_t m_fileSize;
    std::string m_fileName;
};


class Server
{
public:
    Server(boost::asio::io_context& io_context, short t_port,
           std::string const& t_workDirectory = "/tmp/btftcs");
private:
    void doAccept();
    void createWorkDirectory();

    boost::asio::ip::tcp::socket m_socket;
    boost::asio::ip::tcp::acceptor m_acceptor;

    std::string m_workDirectory;
};

#endif // SERVER_H
