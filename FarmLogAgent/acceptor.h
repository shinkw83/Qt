#pragma once

#include <boost/asio.hpp>
#include "log_agent.h"
#include <QObject>
#include <QString>

using boost::asio::ip::tcp;

class acceptor : public QObject {
    Q_OBJECT
public:
    acceptor(int port, std::string path, int pkt_size);
	~acceptor();

    void receive_date_time(QString dateTime);
    void open_listen_socket();

private:
	void run();

signals:
    void send_date_time(QString dateTime);

private:
	boost::asio::io_context io_ctx_;
	tcp::acceptor acceptor_;

	std::thread run_th_;
	std::atomic<bool> run_flag_{ true };
	std::atomic<bool> accepted_{ false };

	log_agent *agent_ = nullptr;

	std::string path_;
    int pkt_size_;
};

