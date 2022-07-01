#include "http_service.hpp"

using namespace boost;

// Status codes
std::unordered_map<unsigned int, std::string> HttpStatusTable =
    {
        {101, "Switching Protocols"},
        {201, "Created"},
        {202, "Accepted"},
        {200, "200 OK"},
        {400, "Bad Request"},
        {401, "Unauthorized"},
        {404, "404 Not Found"},
        {408, "Request Timeout"},
        {413, "413 Request Entity Too Large"},
        {500, "500 Internal Server Error"},
        {501, "501 Not Implemented"},
        {502, "Bad Gateway"},
        {503, "Service Unavailable"},
        {505, "505 HTTP Version Not Supported"}};

http_request_parser::http_request_parser(std::string &request)
{
    m_http_request = request;
}

std::shared_ptr<http_request> http_request_parser::GetHttpRequest()
{
    if (m_http_request.empty())
    {
        std::cerr << "Request is empty! " << std::endl;
        return nullptr;
    }

    std::string request_method, resource, http_version;
    std::istringstream request_line_stream(m_http_request);

    request_line_stream >> request_method;

    request_line_stream >> resource;

    request_line_stream >> http_version;

    std::shared_ptr<http_request> request(new http_request);

    request->resource = std::move(resource);
    request->status = 0;

    if (!request_method.compare("GET"))
    {
        request->method = Method::GET;
    }
    else if (request_method.compare("HEAD") == 0)
    {
        request->method = Method::HEAD;
    }
    else if (request_method.compare("POST") == 0)
    {
        request->method = Method::POST;
    }
    else if (request_method.compare("PUT") == 0)
    {
        request->method = Method::PUT;
    }
    else if (request_method.compare("DELETE") == 0)
    {
        request->method = Method::DELETE;
    }
    else if (request_method.compare("CONNECT") == 0)
    {
        request->method = Method::CONNECT;
    }
    else if (request_method.compare("OPTIONS") == 0)
    {
        request->method = Method::OPTIONS;
    }
    else if (request_method.compare("TRACE") == 0)
    {
        request->method = Method::TRACE;
    }
    else
    {
        request->status = 400;
    }

    if (!http_version.compare("HTTP/1.1")) {
        request->http_version = "1.1";
    } else {
        request->status = 505;
    }

    request->request = std::move(m_http_request);

    return request;
}

extern std::string RESOURCE_DIRECTORY_PATH;

http_service::http_service(std::shared_ptr<asio::ip::tcp::socket> socket)
    : m_socket(socket), 
    m_request(4096),
    m_is_response_sent(false) {}

void http_service::Handle_request() {
    // Read the request from client
    asio::async_read_until(*m_socket.get(),
        m_request, '\r',
        [this](const boost::system::error_code& ec,
            std::size_t bytes_transfered) {

                std::string request_line;
                std::istream request_stream(&m_request);
                std::getline(request_stream, request_line, '\0');

                http_request_parser parser(request_line);
                std::shared_ptr<http_request> http_request = parser.GetHttpRequest();

                std::cout << "Handling client: " << get_ip() << std::endl;
                std::cout << "Request: " << std::endl;
                std::cout << http_request->request << std::endl;

                std::istringstream istrstream(http_request->request);
                while (std::getline(istrstream, m_script_data)){}
                std::cout << "Script data: " << m_script_data << std::endl;

                if (http_request->status == 0) {
                    m_requested_resource = http_request->resource;

                    switch(http_request->method) {
                        case Method::GET :
                            process_get_request();
                            break;
                        case Method::POST :
                            //process_post_request();
                            break;
                        case Method::HEAD :
                            //process_head_request();
                            break;
                        case Method::DELETE :
                            //process_delete_request();
                            break;
                        case Method::OPTIONS :
                           // process_options_request();
                            break;
                        default :
                            break;
                    }
                } else {
                    m_response_status_code = http_request->status;
                    if (!m_is_response_sent) {
                        this->send_response();
                        return;
                    }
                }
            });
}

std::string http_service::get_ip() {
    asio::ip::tcp::endpoint ep = m_socket->remote_endpoint();
    asio::ip::address addr = ep.address();
    return std::move(addr.to_string());
}

void http_service::process_get_request() {
    if(m_requested_resource.empty()) {
        m_response_status_code = 400;
        return;
    }

    if (boost::contains(m_requested_resource, "/show/")) {
        // Get data from redis
        // Save it to json
        // Send json as response
        std::string tmp = default_get_response1 + m_requested_resource + default_get_response2;
        std::size_t total_size = tmp.size();

        m_resource_buffer.reset(new char[total_size]);

        tmp.copy(m_resource_buffer.get(), total_size);
        m_resource_byte_size = total_size;
        this->send_response();
    }
}

std::string http_service::get_response_status() {
    std::string response_status;

    auto end = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::string timestr(std::ctime(&end_time));

    m_socket->shutdown(asio::ip::tcp::socket::shutdown_receive);
    auto status_line = HttpStatusTable[m_response_status_code];

    response_status = std::string("HTTP/1.1 ") + status_line + "\n";
  if(m_resource_byte_size > 0){
    response_status += std::string("Content-Length: ") +
                       std::to_string(m_resource_byte_size) + "\n";
  }
  if(!m_content_type.empty()){
    response_status += m_content_type + "\n";
  }else{
    response_status += std::string("Content-Type: text/html") + "\n";
  }
  if(!m_server_options.empty()){
    response_status += std::string("Allow: ") + std::move(m_server_options) + "\n";
  }
  response_status += std::string("Server: FastWebServer/0.0.1") + "\n";
  response_status += std::string("AcceptRanges: bytes") + "\n";
  response_status += std::string("Connection: Closed") + "\n";
  response_status += std::string("Date: ") + timestr + "\n";

  return std::move(response_status);
}

void http_service::send_response() {
    std::vector<asio::const_buffer> response_buffers;

    m_is_response_sent = true;

    std::string response_status = get_response_status();
    response_buffers.push_back(asio::buffer(std::move(response_status)));

    if (m_resource_byte_size > 0) {
        response_buffers.push_back(asio::buffer(m_resource_buffer.get(), m_resource_byte_size));
    }

    //send response to client with data
  asio::async_write(*m_socket.get(),
           response_buffers,
           [this](const boost::system::error_code& ec,
                 std::size_t bytes_transferred)
           {
             if(ec.value() != 0){
               std::cout<<"Error occured, Error code = "<<ec.value()
                    <<" Message: "<<ec.message();
             }
             finish();
           });
}

void http_service::finish()
{
  delete this;
}