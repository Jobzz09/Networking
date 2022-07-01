#include "http_service.hpp"
#include "http_acceptor.hpp"

using namespace boost;

//initialize TCP endpoint with IPv4
Http_acceptor::Http_acceptor(asio::io_context& ioc, unsigned short port_num):
        m_ioc(ioc),
        m_acceptor(m_ioc, 
            asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port_num)),
        m_isStopped(false)
{}


void Http_acceptor::Start()
{
  m_acceptor.listen();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  AcceptConnection();
}

void Http_acceptor::Stop()
{
  m_isStopped.store(true);
}

//accept the client connection request
void Http_acceptor::AcceptConnection()
{
  std::shared_ptr<asio::ip::tcp::socket> sock(new asio::ip::tcp::socket(m_ioc));

  m_acceptor.async_accept(*sock.get(),
      [this, sock](const boost::system::error_code& ec)
      {
        if(ec.value() == 0){
          (new http_service(sock))->Handle_request();
        }else{
          std::cout<<"Error occured, Error code = "<<ec.value()
                   <<" Message: "<<ec.message();
        }

        // accept next request if not stopped yet
        if(!m_isStopped.load()){
          AcceptConnection();
        }else{
          //stop accepting incoming connection requests
          m_acceptor.close();
        }
      });
}



