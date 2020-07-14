#ifndef LOGGING_H
#define LOGGING_H

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QScopedPointer>
#include <QTextStream>

class logging
{
public:
    logging();

    static QScopedPointer<QFile> log_file_;
    static void write();
    static void message_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

#endif // LOGGING_H
