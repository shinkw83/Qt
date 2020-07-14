#pragma once

#include <boost/asio.hpp>
#include "log_agent.h"

using boost::asio::ip::tcp;

class acceptor {
public:
	acceptor(int port, std::string path);
	~acceptor();

private:
	void run();

private:
	boost::asio::io_context io_ctx_;
	tcp::acceptor acceptor_;

	std::thread run_th_;
	std::atomic<bool> run_flag_{ true };
	std::atomic<bool> accepted_{ false };

	log_agent *agent_ = nullptr;

	std::string path_;
};

