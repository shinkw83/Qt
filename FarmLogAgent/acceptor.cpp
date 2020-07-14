#include "acceptor.h"
#include "logging.h"

acceptor::acceptor(int port, std::string path) : io_ctx_(), acceptor_(io_ctx_, tcp::endpoint(tcp::v4(), port)) {
    path_ = path;
    run_th_ = std::thread(&acceptor::run, this);
}


acceptor::~acceptor() {
    acceptor_.close();
    run_flag_ = false;
    if (run_th_.joinable()) {
        run_th_.join();
    }

    if (agent_ != nullptr) {
        delete agent_;
        agent_ = nullptr;
    }
}

void acceptor::run() {
    while (run_flag_) {
        if (accepted_ == true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        tcp::socket *sock = new tcp::socket(io_ctx_);
        try {
            qInfo() << "Start listen port 6565";
            acceptor_.accept(*sock);
        }
        catch (const std::exception &e) {
            // error log
            QString errorlog = QString::fromUtf8(e.what());
            errorlog = "Accepted error. " + errorlog;
            qCritical() << errorlog;
            delete sock;
            continue;
        }

        // create log agent
        if (agent_ != nullptr) {
            delete agent_;
        }

        qInfo() << "Accepted from client.";
        agent_ = new log_agent(sock, path_);
        accepted_ = true;
    }
}
