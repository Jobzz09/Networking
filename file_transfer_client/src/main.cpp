#include "../include/client.h"


int main(int argc, char** argv)
{
    if (argc != 4) {
        std::cerr << "Usage:\n client <address> <port> <filePath>\n";
        exit(1);
    }


    Logger::instance().setOptions("client_%3N.log", 1* 1024 * 1024, 10 * 1024 * 1024);

    auto address = argv[1];
    auto port = argv[2];
    auto file = argv[3];

    try {
        boost::asio::io_context ioContext;
        boost::asio::ip::tcp::resolver resolver(ioContext);
        auto endpointIterator = resolver.resolve({address, port});
        Client cl(ioContext, endpointIterator, file);

        ioContext.run();
    }

    catch (std::fstream::failure& e) {
        std::cerr << e.what() << std::endl;
    }

    catch (std::exception& e ) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
