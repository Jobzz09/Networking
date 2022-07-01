#include <fstream>
#include <atomic>
#include <thread>
#include <memory>
#include <chrono>
#include <ctime>
#include <iostream>
#include <string>
#include <unordered_map>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sw/redis++/redis++.h>

class RedisClient {
    public:
        RedisClient(std::string endpoint = "tcp://127.0.0.1:6379") :
            m_redis(endpoint.c_str()) {}

        bool Set(std::string key, std::string val) {
            return m_redis.set(key, val);
        }

        std::string GetVal(std::string key) {
            return *m_redis.get(key);
        }

        bool Delete (std::string key) {
            return m_redis.del(key);
        }

    private:
        sw::redis::Redis m_redis;
};


enum class Method {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
};

struct http_request {
    Method method;
    std::string request;
    std::string resource;
    std::string http_version;
    unsigned int status;
};

class http_request_parser {
    public:
        http_request_parser(std::string&);
        
        std::shared_ptr<http_request> GetHttpRequest();

    private:
        std::string m_http_request;
};

class http_service {
    public:
        http_service(std::shared_ptr<boost::asio::ip::tcp::socket>);

        void Handle_request();
    
    private:
        std::string get_ip();

        void process_get_request();
        //std::string get_cgi_program(std::string);

        void process_post_request();

        // void execute_program(std::string, std::string);

        // void process_head_request();

        void process_delete_request();

        // void process_options_request();

        std::string get_response_status();

        void send_response();

        void finish();

        std::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
        boost::asio::streambuf m_request;
        std::string m_requested_resource;
        std::unique_ptr<char[]> m_resource_buffer;
        unsigned int m_response_status_code;
        std::size_t m_resource_byte_size;
        bool m_is_response_sent;
        std::string m_server_options;
        std::string m_script_data;
        std::string m_content_type;
        RedisClient m_redis;

        const std::string m_default_index_page = 
        "<!DOCTYPE html>"
"<html>"
"<head>"
"<title>"
"</title>"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
"<style>"
"body {background-color:#f9bebe;background-repeat:no-repeat;background-position:top left;background-attachment:fixed;}"
"h1{font-family:Arial, sans-serif;color:#000000;background-color:#f9bebe;}"
"p {font-family:Georgia, serif;font-size:14px;font-style:normal;font-weight:normal;color:#000000;background-color:#f9bebe;}"
"</style>"
"</head>"
"<body>"
"<h1>Http fast web-server</h1>"
"<p>Made by Jobzz09(MKorytko)</p>"
"<a href=\"https://github.com/Jobzz09\">Github profile</a>"
"</body>"
"</html>";


    const std::string default_get_response1 =
    "<!DOCTYPE html>"
"<html>"
"<head>"
"<title>"
"</title>"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
"<style>"
"body {background-color:#f9bebe;background-repeat:no-repeat;background-position:top left;background-attachment:fixed;}"
"h1{font-family:Arial, sans-serif;color:#000000;background-color:#f9bebe;}"
"p {font-family:Georgia, serif;font-size:14px;font-style:normal;font-weight:normal;color:#000000;background-color:#f9bebe;}"
"</style>"
"</head>"
"<body>"
"<h1>Get request result</h1>"
"<p>requested resource: ";

    const std::string default_get_response2 = 
    "</p>"
"</body>"
"</html>";
};

std::string GetCurrentTime();