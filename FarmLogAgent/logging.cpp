#include "logging.h"

QScopedPointer<QFile> logging::log_file_(nullptr);

logging::logging()
{

}

void logging::write() {
    QString path(QCoreApplication::applicationDirPath() + "\\log");
    QDir dir;
    if (!dir.exists(path)) {
        dir.mkpath(path);
    }

    log_file_.reset(new QFile(path + "\\log_" + QDateTime::currentDateTime().toString("yyyy-MM-dd") + ".log"));
    log_file_.data()->open(QFile::Append | QFile::Text);

    qInstallMessageHandler(message_handler);
}

void logging::message_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QTextStream out(log_file_.data());
    out << QDateTime::currentDateTime().toString("hh:mm:ss");

    switch(type) {
    case QtInfoMsg:
        out << "[ Info]";
        break;
    case QtDebugMsg:
        out << "[ Debug]";
        break;
    case QtWarningMsg:
        out << "[ Warn]";
        break;
    case QtCriticalMsg:
        out << "[Error]";
        break;
    case QtFatalMsg:
        out << "[Fatal]";
        break;
    }

    out << " : " << msg << endl;
    out.flush();
}
