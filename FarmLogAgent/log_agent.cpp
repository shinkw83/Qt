#include "log_agent.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "json.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include "logging.h"
#include "acceptor.h"

log_agent::log_agent(tcp::socket *sock, std::string path, void *parent, int pkt_size) {
	sock_ = sock;
	path_ = path;
    parent_ = parent;
    pkt_size_ = pkt_size;
    data_.resize(pkt_size_ + 1);

    set_column_list();

    start_read();
}


log_agent::~log_agent() {
	delete sock_;

    qInfo() << "Destroy log agent";
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
    column_list_[19] = "mesureVal09";
    column_list_[20] = "mesureVal10";
    column_list_[21] = "mesureVal11";
    column_list_[22] = "mesureVal12";
    column_list_[23] = "mesureVal13";
    column_list_[24] = "mesureVal14";
    column_list_[25] = "mesureVal15";
}

void log_agent::start_read() {
    boost::asio::async_read(*sock_, boost::asio::buffer(data_, pkt_size_),
        boost::bind(&log_agent::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void log_agent::handle_read(const boost::system::error_code &ec, std::size_t n) {
    if (!ec) {
        data_[pkt_size_] = 0x00;
        qInfo() << "Read data size[" << n << "]";
        qInfo() << "Read data[" << QString::fromUtf8(data_.data()) << "]";
        save_data(data_.data());
        start_read();
    } else {
        qCritical() << "Read Error[" << QString::fromUtf8(ec.message().c_str()) << "]";
    }
}

void log_agent::save_data(std::string data) {
    std::vector<std::string> words;
    boost::algorithm::split(words, data, boost::algorithm::is_any_of("|"));

    // column size check
    if (words.size() != 26) {
        qCritical() << "Read data column count missmatch.";
        return;
    }

    // date time size check
    if (words[10].size() != 19) {
        qCritical() << "DateTime column size missmatch.";
        return;
    }

    // parse date time
    std::string datetime = words[10];
    std::string tyear = datetime.substr(0, 4);
    std::string tmon = datetime.substr(5, 2);
    std::string tday = datetime.substr(8, 2);
    std::string thour = datetime.substr(11, 2);
    std::string tmin = datetime.substr(14, 2);
    std::string tsec = datetime.substr(17, 2);

    // make key
    std::string key = words[2] + "_" + words[0] + "_" + words[3] + "_" + tyear + tmon + tday + "_" + thour + tmin + tsec;

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

    std::string file_path = path_ + "\\" + key + ".log";
    QString qpath = QString::fromUtf8(file_path.c_str());

    QFile qfile(qpath);
    if (!qfile.open(QFile::WriteOnly|QFile::Text)) {
        qCritical() << "File open fail.[" << qpath << "][" << file_path.c_str() << "]";
        return;
    }

    qInfo() << "Save file path : [" << qpath << "][" << file_path.c_str() << "]";

    QString qwrite_data = QString::fromUtf8(json_string.c_str());
    QTextStream saveStream(&qfile);
    saveStream << qwrite_data;
    qfile.close();

    QString qdatetime = QString::fromUtf8(datetime.c_str());
    acceptor* acptor = static_cast<acceptor *>(parent_);
    acptor->receive_date_time(qdatetime);
}
