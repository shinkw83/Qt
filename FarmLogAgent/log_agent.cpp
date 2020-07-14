#include "log_agent.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "json.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include "logging.h"

log_agent::log_agent(tcp::socket *sock, std::string path) {
	sock_ = sock;
	path_ = path;

    set_column_list();

	run_th_ = std::thread(&log_agent::run, this);
	proc_th_ = std::thread(&log_agent::proc_save, this);
}


log_agent::~log_agent() {
	run_flag_ = false;
	if (run_th_.joinable()) {
		run_th_.join();
	}

	proc_flag_ = false;
	if (proc_th_.joinable()) {
		proc_th_.join();
	}

    sock_->close();
	delete sock_;
}

void log_agent::set_column_list() {
    column_list_.resize(26);
    column_list_[0] = "lsindRegistNo";
    column_list_[1] = "itemCode";
    column_list_[2] = "makrId";
    column_list_[3] = "eqpmnCode";
    column_list_[4] = "eqpmnEsntlSn";
    column_list_[5] = "eqpmnNo";
    column_list_[6] = "stallTyCode";
    column_list_[7] = "stallNo";
    column_list_[8] = "roomNo";
    column_list_[9] = "roomDtlNo";
    column_list_[10] = "mesureDt";
    column_list_[11] = "mesureVal01";
    column_list_[12] = "mesureVal02";
    column_list_[13] = "mesureVal03";
    column_list_[14] = "mesureVal04";
    column_list_[15] = "mesureVal05";
    column_list_[16] = "mesureVal06";
    column_list_[17] = "mesureVal07";
    column_list_[18] = "mesureVal08";
    column_list_[19] = "mesureVal00";
    column_list_[20] = "mesureVal10";
    column_list_[21] = "mesureVal11";
    column_list_[22] = "mesureVal12";
    column_list_[23] = "mesureVal13";
    column_list_[24] = "mesureVal14";
    column_list_[25] = "mesureVal15";
}

void log_agent::run() {
	while (run_flag_) {
		fd_set fd_in;
		struct timeval timeout;

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		FD_ZERO(&fd_in);
		int fd = sock_->native_handle();
		FD_SET(fd, &fd_in);

		select(fd + 1, &fd_in, nullptr, nullptr, &timeout);

		if (!FD_ISSET(fd, &fd_in)) {
			save_flag_ = true;
			continue;
		}

        // read data from tcp socket
        std::vector<char> buf;
        buf.resize(100);
        try {
            boost::asio::read(*sock_, boost::asio::buffer(buf.data(), 99));
        } catch (const std::exception &e) {
            QString errorlog = QString::fromUtf8(e.what());
            errorlog = "Read error. " + errorlog;
            qCritical() << errorlog;
            sock_->close();
            save_flag_ = true;
            return;
        }

        std::string data = buf.data();
        std::vector<std::string> words;
        boost::algorithm::split(words, data, boost::algorithm::is_any_of("|"));

        if (words.size() != 26) {
            // error log
            QString errorlog = QString::fromUtf8(data.c_str());
            qCritical() << "Read data colimn size missmatch.";
            qCritical() << errorlog;
            continue;
        }

        // make key
        std::string key = words[2] + "_" + words[0] + "_" + words[3];
        std::vector<std::string> mesureday;
        boost::algorithm::split(mesureday, words[10], boost::algorithm::is_any_of("-"));
        if (mesureday.size() != 4) {
            QString errorlog = QString::fromUtf8(words[10].c_str());
            qCritical() << "DateTime data is wrong.";
            qCritical() << errorlog;
            continue;
        }
        std::vector<std::string> mesuretime;
        boost::algorithm::split(mesuretime, mesureday[3], boost::algorithm::is_any_of(":"));
        if (mesuretime.size() != 3) {
            QString errorlog = QString::fromUtf8(mesureday[3].c_str());
            qCritical() << "Time data is wrong.";
            qCritical() << errorlog;
            continue;
        }

        struct tm mesuredt;
        mesuredt.tm_year = boost::lexical_cast<int>(mesureday[0]) - 1900;
        mesuredt.tm_mon = boost::lexical_cast<int>(mesureday[1]) - 1;
        mesuredt.tm_mday = boost::lexical_cast<int>(mesureday[2]);
        mesuredt.tm_hour = boost::lexical_cast<int>(mesuretime[0]);
        mesuredt.tm_min = boost::lexical_cast<int>(mesuretime[1]);
        mesuredt.tm_sec = boost::lexical_cast<int>(mesuretime[2]);

        words[10] = mesureday[0] + "-" + mesureday[1] + "-" + mesureday[2] + " " + mesuretime[0] + ":" + mesuretime[1] + ":" + mesuretime[2];

        time_t key_time = mktime(&mesuredt);

        std::string json_string;
        for (int i = 0; i < 26; i++) {
            char key_value[256] = {0, };
            if (i < 12 && words[i] == "NONE") {
                words[i] = "";
            } else if (i > 11 && words[i] == "0") {
                words[i] = "";
            }

            if (i == 0) {
                snprintf(key_value, sizeof(key_value), "{ \"%s\" : \"%s\", ", column_list_[i].c_str(), words[i].c_str());
            } else if (i == 25) {
                snprintf(key_value, sizeof(key_value), "\"%s\" : \"%s\" }", column_list_[i].c_str(), words[i].c_str());
            } else {
                snprintf(key_value, sizeof(key_value), "\"%s\" : \"%s\", ", column_list_[i].c_str(), words[i].c_str());
            }
            json_string += key_value;
        }

        std::lock_guard<std::mutex> guard(log_mutex_);

        auto key_it = log_key_.find(key);
        if (key_it == log_key_.end()) {
            std::set<time_t> temp;
            temp.insert(key_time);
            log_key_.insert(std::pair<std::string, std::set<time_t>>(key, temp));

            std::vector<std::string> logdata;
            logdata.push_back(json_string);
            log_data_.insert(std::pair<time_t, std::vector<std::string>>(key_time, logdata));
        } else {
            auto time_it = key_it->second.find(key_time);
            if (time_it == key_it->second.end()) {
                key_it->second.insert(key_time);

                std::vector<std::string> logdata;
                logdata.push_back(json_string);
                log_data_.insert(std::pair<time_t, std::vector<std::string>>(key_time, logdata));
            } else {
                auto data_it = log_data_.find(key_time);
                if (data_it != log_data_.end()) {
                    data_it->second.push_back(json_string);
                }
            }
        }
	}
}

void log_agent::proc_save() {
	const double time_limit = 1.0;
	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
	while (proc_flag_) {
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		std::chrono::duration<double> el_sec = now - start;
		if (el_sec.count() > time_limit) {
			save_func();
		}
	}
}

void log_agent::save_func() {
	std::lock_guard<std::mutex> guard(log_mutex_);

	if (save_flag_) {
		save_flag_ = false;

		// whole save
		auto key_it = log_key_.begin();
		while (key_it != log_key_.end()) {
			for (auto time_it : key_it->second) {
				auto data = log_data_.find(time_it);
				if (data != log_data_.end()) {
					write_func(key_it->first, time_it, data->second);
					log_data_.erase(data);
				}
			}
			key_it->second.clear();
			key_it++;
		}
	}
	else {
		auto key_it = log_key_.begin();
		while (key_it != log_key_.end()) {
			auto time_it = key_it->second.begin();
			while (time_it != key_it->second.end()) {
				if (key_it->second.size() < 2) {
					time_it++;
					continue;
				}

				auto data = log_data_.find(*time_it);
				if (data != log_data_.end()) {
					write_func(key_it->first, *time_it, data->second);
					log_data_.erase(data);
				}
				time_it = key_it->second.erase(time_it);
			}

			key_it++;
		}
	}
}

void log_agent::write_func(const std::string &key, const time_t &mesure_time, const std::vector<std::string> &data) {
	struct tm *ltime = localtime(&mesure_time);
	ltime->tm_year = ltime->tm_year + 1900;
	ltime->tm_mon = ltime->tm_mon + 1;

	char filename[512] = { 0, };
#if defined (MACOS)
    snprintf(filename, sizeof(filename), "%s/%s_%04d%02d%02d_%02d%02d%02d.log",
		path_.c_str(), key.c_str(), ltime->tm_year, ltime->tm_mon, ltime->tm_mday, ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
#elif defined (WINDOWS)
    snprintf(filename, sizeof(filename), "%s\\%s_%04d%02d%02d_%02d%02d%02d.log",
        path_.c_str(), key.c_str(), ltime->tm_year, ltime->tm_mon, ltime->tm_mday, ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
#endif
	std::string write_data;
	for (auto it : data) {
		write_data = write_data + it + "\r\n";
	}

    QString qpath = QString::fromUtf8(filename);

    QFile qfile(qpath);
    qfile.open(QFile::WriteOnly|QFile::Text);

    QString qwrite_data = QString::fromUtf8(write_data.c_str());
    QTextStream saveStream(&qfile);
    saveStream << qwrite_data;
    qfile.close();

    //FILE *p = fopen(filename, "w");
    //fwrite(write_data.data(), write_data.size(), 1, p);
    //fclose(p);
}
