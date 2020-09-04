#include "acceptor.h"
#include "logging.h"

acceptor::acceptor(int port, std::string path, int pkt_size) : io_ctx_(), acceptor_(io_ctx_, tcp::endpoint(tcp::v4(), port)) {
    path_ = path;
    pkt_size_ = pkt_size;
    check_run_flag_ = true;

    start_accept();

    run_th_ = std::thread(&acceptor::run, this);
    check_th_ = std::thread(&acceptor::check_agent_run_time, this);
}


acceptor::~acceptor() {
    qInfo() << "Stop io_context";
    io_ctx_.stop();
    if (run_th_.joinable()) {
        run_th_.join();
    }

    check_run_flag_ = false;
    if (check_th_.joinable()) {
        check_th_.join();
    }

    qInfo() << "Delete client";
    for (auto it : time_set_) {
        delete it.first;
    }
    time_set_.clear();

    qInfo() << "Destroy acceptor";
}

void acceptor::start_accept() {
    qInfo() << "Start listen";
    tcp::socket *sock = new tcp::socket(io_ctx_);
    acceptor_.async_accept(*sock, boost::bind(&acceptor::handle_accept, this, sock, boost::asio::placeholders::error));
}

void acceptor::handle_accept(tcp::socket *sock, const boost::system::error_code &ec) {
    if (!ec) {
        qInfo() << "Accepted from client.";
        log_agent *agent = new log_agent(sock, path_, this, pkt_size_);

        std::lock_guard<std::mutex> guard(check_mutex_);
        time_set_[agent] = std::chrono::system_clock::now();
    } else {
        delete sock;
        qCritical() << "Accept fail.[" + QString::fromUtf8(ec.message().c_str()) + "]";
    }

    start_accept();
}

void acceptor::run() {
    io_ctx_.run();
}

void acceptor::receive_date_time(QString dateTime) {
    emit send_date_time(dateTime);
}

void acceptor::check_agent_run_time() {
    while (check_run_flag_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::lock_guard<std::mutex> guard(check_mutex_);
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        auto it = time_set_.begin();
        while (it != time_set_.end()) {
            std::chrono::duration<double> el_sec = now - it->second;
            if (el_sec.count() > 60.0f) {
                delete it->first;
                it = time_set_.erase(it);
            } else {
                it++;
            }
        }
    }
}
