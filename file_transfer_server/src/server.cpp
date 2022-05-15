#include "../include/server.h"

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}


void Logger::setOptions(std::string const& t_fileName, unsigned t_rotationSize,
    unsigned t_maxSize)
{
    boost::log::add_file_log(
        boost::log::keywords::file_name = t_fileName,
        boost::log::keywords::rotation_size = t_rotationSize,
        boost::log::keywords::max_size = t_maxSize,
        boost::log::keywords::time_based_rotation
            = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
        boost::log::keywords::format = "[%TimeStamp%]: %Message%",
        boost::log::keywords::auto_flush = true
        );

    boost::log::add_common_attributes();
}

Session::Session(boost::asio::ip::tcp::socket t_socket) : m_socket(std::move(t_socket)) {}

void Session::doRead() {
    auto self = shared_from_this();
    boost::asio::async_read_until(m_socket, m_requestBuf, "\n\n",
                                  [this, self](boost::system::error_code ec, std::size_t bytes) -> void {
        if (!ec)
            processRead(bytes);
        else
            handleError(__FUNCTION__, ec);
    });
}

void Session::processRead(std::size_t t_bytesTransfered) {
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << "(" << t_bytesTransfered << ")"
                             << ", in_avail = " << m_requestBuf.in_avail() << ", size = "
                             << m_requestBuf.size() << ", max_size = " << m_requestBuf.max_size() << ".";

    std::istream requestStream(&m_requestBuf);
    readData(requestStream);

    auto pos = m_fileName.find_last_of('\\');
    if (pos != std::string::npos)
        m_fileName = m_fileName.substr(pos + 1);
    createFile();

    while (requestStream.gcount() > 0) {
        requestStream.read(m_buf.data(), m_buf.size());
        BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << " write " << requestStream.gcount() << " bytes.";
        m_outputFile.write(m_buf.data(), requestStream.gcount());
    }

    auto self = shared_from_this();
    m_socket.async_read_some(boost::asio::buffer(m_buf.data(), m_buf.size()),
                             [this, self] (boost::system::error_code ec, std::size_t bytes) -> void {
        if (!ec)
            doReadFileContent(bytes);
        else
            handleError(__FUNCTION__, ec);
    });
}

void Session::readData(std::istream &stream) {
    stream >> m_fileName;
    stream >> m_fileSize;
    stream.read(m_buf.data(), 2);

    BOOST_LOG_TRIVIAL(trace) << m_fileName << " size is " << m_fileSize
                             << ", tellg = " << stream.tellg();
}

void Session::createFile() {
    m_outputFile.open(m_fileName, std::ios_base::binary);
    if (!m_outputFile) {
        BOOST_LOG_TRIVIAL(error) << __LINE__ << ": Failed to create " << m_fileName;
        return;
    }
}

void Session::doReadFileContent(std::size_t t_bytesTransfered) {
    if (t_bytesTransfered > 0) {
        m_outputFile.write(m_buf.data(), static_cast<std::streamsize>(t_bytesTransfered));
        BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << " recv " << m_outputFile.tellp() << " bytes";

        if (m_outputFile.tellp() >= static_cast<std::streamsize>(m_fileSize)) {
            std::cout << "Received file: " << m_fileName << std::endl;
            return;
        }
    }

    auto self = shared_from_this();
    m_socket.async_read_some(boost::asio::buffer(m_buf.data(), m_buf.size()),
                             [this, self](boost::system::error_code ec, std::size_t bytes) -> void {
        doReadFileContent(bytes);
    });
}

void Session::handleError(std::string const& t_functionName, boost::system::error_code const& ec) {
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " in " << t_functionName << " due to "
                             << ec << " " << ec.message() << std::endl;
}



Server::Server(boost::asio::io_context &io_context, short t_port,
       std::string const& t_workDirectory)
    : m_socket(io_context),
    m_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), t_port)),
    m_workDirectory(t_workDirectory)
{
    std::cout << "Server started! "<< std::endl;
    createWorkDirectory();
    doAccept();
}

void Server::doAccept() {
    m_acceptor.async_accept(m_socket,
                            [this](boost::system::error_code ec) {
        if (!ec)
            std::make_shared<Session>(std::move(m_socket))->start();
        doAccept();
    });
}

void Server::createWorkDirectory() {
    auto currentPath = boost::filesystem::path(m_workDirectory);
    if (!boost::filesystem::exists(currentPath) && !boost::filesystem::create_directory(currentPath))
        BOOST_LOG_TRIVIAL(error) << "Couldn't create working directory: " << m_workDirectory;
    boost::filesystem::current_path(currentPath);
}
