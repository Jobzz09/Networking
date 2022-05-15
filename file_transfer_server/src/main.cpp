#include <iostream>
#include <boost/asio/io_context.hpp>

#include "../include/server.h"

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::cerr << "Usage:\n server <port> <workDirectory>\n" << std::endl;
        exit(1);
    }

    Logger::instance().setOptions("server_%3N.log", 1 * 1024 * 1024, 10 * 1024 * 1024);
    try {
        boost::asio::io_context context;
        Server server(context, std::stoi(argv[1]), argv[2]);

        context.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
