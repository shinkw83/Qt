#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
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

private:
    void start_accept();
    void handle_accept(tcp::socket *sock, const boost::system::error_code &ec);

    void run();
    void check_agent_run_time();

signals:
    void send_date_time(QString dateTime);

private:
	boost::asio::io_context io_ctx_;
	tcp::acceptor acceptor_;

	std::thread run_th_;
    std::thread check_th_;

    std::atomic<bool> check_run_flag_;
    std::mutex check_mutex_;
    std::map<log_agent *, std::chrono::time_point<std::chrono::system_clock>> time_set_;

	std::string path_;
    int pkt_size_;
};

