#include "http_server.hpp"

using namespace boost;

//create new work of io_service
Http_server::Http_server()
{
  m_work.reset(new asio::io_context::work(m_ioc));
}

//start the http server
void Http_server::Start(unsigned short port, unsigned int thread_pool_size)
{
  if(thread_pool_size <= 0)  return;

  //create and start HttpAcceptor for accepting connection requests
  m_acceptor.reset(new Http_acceptor(m_ioc, port));
  m_acceptor->Start();

  std::cout<<"Server started at address: 127.0.0.1, port: 1234"<<std::endl;
  std::cout<<"Goto http://127.0.0.1:1234/"<<std::endl;

  //create specified number of threads and add them to the pool
  for(unsigned int i = 0; i < thread_pool_size; i++){
      std::unique_ptr<std::thread> 
             th(new std::thread([this]()
               {
                 //run the socket io service
                 m_ioc.run();
               })
             );

          m_threadPool.push_back(std::move(th));
    }
}

//stop the server.
void Http_server::Stop()
{
  m_acceptor->Stop();
  m_ioc.stop();
  for(auto& th : m_threadPool){
    th->join();
  }
}


