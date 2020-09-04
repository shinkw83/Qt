#pragma once

#include <boost/asio.hpp>
#include <set>
#include <map>
#include <thread>
#include <mutex>

#if defined(__linux) || defined(__linux__) || defined(linux)
# define LINUX

#elif defined(__APPLE__)
# define MACOS

#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(_WIN64)
# define WINDOWS

#endif

using boost::asio::ip::tcp;

class log_agent
{
public:
    log_agent(tcp::socket *sock, std::string path, void *parent, int pkt_size);
	~log_agent();

private:
    void set_column_list();

    void start_read();
    void handle_read(const boost::system::error_code &ec, std::size_t n);
    void save_data(std::string data);

private:
	tcp::socket *sock_;
    std::vector<char> data_;

	std::string path_;
    int pkt_size_;

    std::vector<std::string> column_list_;

    void *parent_ = nullptr;
};

