#include "../include/client.h"
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


Client::Client(boost::asio::io_context &t_context, boost::asio::ip::tcp::resolver::iterator t_endpointIterator,
               std::string const& t_path)
    : m_ioContext(t_context), m_socket(t_context),
      m_endpointIterator(t_endpointIterator), m_path(t_path)
{
    doConnect();
    openFile(m_path);
}

void Client::openFile(std::string const& t_path) {
    m_sourceFile.open(t_path, std::ios_base::binary | std::ios_base::ate);
    if (m_sourceFile.fail())
        throw std::fstream::failure("Failed while openning file " + t_path);
    m_sourceFile.seekg(0, m_sourceFile.end);
    auto fileSize = m_sourceFile.tellg();
    m_sourceFile.seekg(0, m_sourceFile.beg);

    std::ostream requestStream(&m_request);
    boost::filesystem::path p(t_path);
    requestStream << p.filename().string() << "\n" << fileSize << "\n\n";
    BOOST_LOG_TRIVIAL(trace) << "Request size: " << m_request.size();
}

void Client::doConnect() {
    boost::asio::async_connect(m_socket, m_endpointIterator,
                               [this] (boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator it) -> void {
       if (!ec)
           writeBuffer(m_request);
       else {
           std::cout << "Couldn't connect to host. Please run server or check network connection\n";
           BOOST_LOG_TRIVIAL(error) << "Error: " << ec.message();
       }
    });
}

void Client::doWriteFile(const boost::system::error_code& t_ec) {
    if (!t_ec) {
        if (m_sourceFile) {
            m_sourceFile.read(m_buf.data(), m_buf.size());
            if (m_sourceFile.fail() && !m_sourceFile.eof()) {
                auto msg = "Failed while reading file";
                BOOST_LOG_TRIVIAL(error) << msg;
                throw std::fstream::failure(msg);
            }
            std::stringstream ss;
            ss << "Send " << m_sourceFile.gcount() << "bytes of  "
               << m_sourceFile.tellg() << "bytes";
            BOOST_LOG_TRIVIAL(trace) << ss.str() << std::endl;

            auto buf = boost::asio::buffer(m_buf.data(), static_cast<std::size_t>(m_sourceFile.gcount()));
            writeBuffer(buf);
        }
    } else
        BOOST_LOG_TRIVIAL(trace) << "Error: " << t_ec.message();
}
