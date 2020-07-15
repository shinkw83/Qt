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
    log_agent(tcp::socket *sock, std::string path, void *parent);
	~log_agent();

private:
    void set_column_list();
	void run();
	void proc_save();
	void save_func();
	void write_func(const std::string &key, const time_t &mesure_time, const std::vector<std::string> &data);

private:
	tcp::socket *sock_;

	std::thread run_th_;
	std::atomic<bool> run_flag_{ true };

	std::thread proc_th_;
	std::atomic<bool> proc_flag_{ true };

	std::map<time_t, std::vector<std::string>> log_data_;
	std::map<std::string, std::set<time_t>> log_key_;

	std::mutex log_mutex_;
	std::atomic<bool> save_flag_{ false };

	std::string path_;

    std::vector<std::string> column_list_;

    void *parent_ = nullptr;
};

