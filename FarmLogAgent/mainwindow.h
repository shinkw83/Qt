#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "acceptor.h"
#include <iostream>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void init();

private slots:
    void on_btn_info__clicked();

    void on_btn_path__clicked();

    void on_btn_home__clicked();

    void on_btn_on_off__clicked();

    void on_btn_dot__clicked();

    void on_btn_ok__clicked();

    void on_btn_cancle__clicked();

public slots:
    void receive_data(QString dateTime);

private:
    Ui::MainWindow *ui;

    acceptor *listener_ = nullptr;
    QString save_path_;

    int port_ = 6565;
    int pkt_size_ = 99;
};
#endif // MAINWINDOW_H
