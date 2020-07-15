#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPixmap>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include "logging.h"
#include <QFontDatabase>
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QFontDatabase::addApplicationFont(":/fonts/font/NanumGothic.ttf");

    logging::write();
    qInfo() << "Start FarmLogAgent";
    ui->setupUi(this);
    QPixmap pix(":/imgs/imgs/MyThings_Info.png");
    int w = ui->lbl_info_image_->width();
    int h = ui->lbl_info_image_->height();
    ui->lbl_info_image_->setPixmap(pix.scaled(w, h, Qt::IgnoreAspectRatio));

    QPixmap pixrun(":/imgs/imgs/Agent_Run.png");
    QPixmap pixstop(":/imgs/imgs/Agent_Stop.png");

    w = ui->lbl_run_->width();
    h = ui->lbl_run_->height();
    ui->lbl_run_->setPixmap(pixrun.scaled(w, h, Qt::IgnoreAspectRatio));
    ui->lbl_stop_->setPixmap(pixstop.scaled(w, h, Qt::IgnoreAspectRatio));

    QFont font(QString("NanumGothic"), 12, 1);
    ui->btn_ok_->setFont(font);
    ui->btn_dot_->setFont(font);
    ui->btn_home_->setFont(font);
    ui->btn_info_->setFont(font);
    ui->btn_path_->setFont(font);
    ui->btn_cancle_->setFont(font);
    ui->lbl_text_->setFont(font);

    QFont lblFont(QString("NanumGothic"), 10, 1);
    ui->lbl_path_->setFont(lblFont);
    ui->ed_path_->setFont(lblFont);
    ui->grp_path_->setFont(lblFont);
    ui->chk_start_->setFont(lblFont);
    ui->chk_auto_start_->setFont(lblFont);
    ui->lbl_current_->setFont(lblFont);

    QFont textFont(QString("NanumGothic"), 12, 1);

    ui->grp_path_->hide();
    ui->btn_ok_->hide();
    ui->btn_cancle_->hide();
    ui->chk_start_->hide();
    ui->chk_auto_start_->hide();
    ui->lbl_info_image_->hide();

    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\MyThings\\FarmLogAgent", QSettings::NativeFormat);
    QString value = settings.value("PATH").toString();
    if (value.isEmpty() || value == "Init") {
        settings.setValue("PATH", "Init");
    } else {
        ui->ed_path_->setText(value);
        save_path_ = value;
    }

    QSettings runsettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    value = runsettings.value("FarmLogAgent").toString();
    if (!value.isEmpty()) {
        ui->chk_start_->setChecked(true);
    }

    value = settings.value("AUTO").toString();
    if (value == "TRUE"){
        ui->chk_auto_start_->setChecked(true);
        ui->btn_on_off_->setChecked(true);
        on_btn_on_off__clicked();
    }

    if (ui->btn_on_off_->isChecked()) {
        ui->lbl_run_->show();
        ui->lbl_stop_->hide();
        ui->lbl_text_->setText("데이터 수집 Agent가 실행 중 입니다.");
    } else {
        ui->lbl_stop_->show();
        ui->lbl_run_->hide();
        ui->lbl_text_->setText("데이터 수집 Agent가 정지 중 입니다.");
    }
    ui->lbl_current_->setText("최근 데이터 수집 : ");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btn_info__clicked()
{
    ui->lbl_info_image_->show();
    ui->grp_path_->hide();
    ui->btn_ok_->hide();
    ui->btn_cancle_->hide();
    ui->btn_on_off_->hide();
    ui->chk_start_->hide();
    ui->chk_auto_start_->hide();
    ui->lbl_run_->hide();
    ui->lbl_stop_->hide();
    ui->lbl_text_->hide();
    ui->lbl_current_->hide();
}

void MainWindow::on_btn_path__clicked()
{
    ui->grp_path_->show();
    ui->btn_ok_->show();
    ui->btn_cancle_->show();
    ui->lbl_info_image_->hide();
    ui->btn_on_off_->hide();
    ui->chk_start_->show();
    ui->chk_auto_start_->show();
    ui->lbl_run_->hide();
    ui->lbl_stop_->hide();
    ui->lbl_text_->hide();
    ui->lbl_current_->hide();
}

void MainWindow::on_btn_home__clicked()
{
    ui->lbl_info_image_->hide();
    ui->grp_path_->hide();
    ui->btn_ok_->hide();
    ui->btn_cancle_->hide();
    ui->btn_on_off_->show();
    ui->chk_start_->hide();
    ui->chk_auto_start_->hide();
    if (ui->btn_on_off_->isChecked()) {
        ui->lbl_run_->show();
        ui->lbl_stop_->hide();
        ui->lbl_text_->setText("데이터 수집 Agent가 실행 중 입니다.");
    } else {
        ui->lbl_stop_->show();
        ui->lbl_run_->hide();
        ui->lbl_text_->setText("데이터 수집 Agent가 정지 중 입니다.");
    }
    ui->lbl_text_->show();
    ui->lbl_current_->show();
}

void MainWindow::on_btn_on_off__clicked()
{
    bool checked = ui->btn_on_off_->isChecked();
    if (!checked) {		// on -> off
        if (listener_ != nullptr) {
            disconnect(listener_, SIGNAL(send_date_time(QString)), this, SLOT(receive_data(QString)));
            delete listener_;
            listener_ = nullptr;
        }
        ui->lbl_run_->hide();
        ui->lbl_stop_->show();
        ui->lbl_text_->setText("데이터 수집 Agent가 정지 중 입니다.");
    } else {
        std::string path = save_path_.toStdString();
        if (path.size() == 0) {
            ui->btn_on_off_->setChecked(false);
            QMessageBox::information(this, "Error", "경로가 지정되지 않았습니다.");
            return;
        }
        ui->lbl_run_->show();
        ui->lbl_stop_->hide();
        ui->lbl_text_->setText("데이터 수집 Agent가 실행 중 입니다.");
        listener_ = new acceptor(6565, path);

        connect(listener_, SIGNAL(send_date_time(QString)), this, SLOT(receive_data(QString)));
    }
}

void MainWindow::on_btn_dot__clicked()
{
    QString qpath = QFileDialog::getExistingDirectory();
    if (qpath.isEmpty()) {
        return;
    }
    qpath.replace('/', '\\');
    ui->ed_path_->setText(qpath);
}

void MainWindow::on_btn_ok__clicked()
{
    save_path_ = ui->ed_path_->text();
    // set regist save path
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\MyThings\\FarmLogAgent", QSettings::NativeFormat);
    settings.setValue("PATH", save_path_);

    if (ui->chk_auto_start_->isChecked()) {
        settings.setValue("AUTO", "TRUE");
    } else {
        settings.setValue("AUTO", "FALSE");
    }

    QSettings runsettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (ui->chk_start_->isChecked()) {
        // add regist start program
        QString runpath = QString("\"%1\"").arg(QCoreApplication::applicationFilePath().replace('/', '\\'));
        runsettings.setValue("FarmLogAgent", runpath);
    } else {
        // delete regist start program
        runsettings.remove("FarmLogAgent");
    }
}

void MainWindow::on_btn_cancle__clicked()
{
    ui->ed_path_->setText(save_path_);
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\MyThings\\FarmLogAgent", QSettings::NativeFormat);
    QString value = settings.value("AUTO").toString();
    if (value != "TRUE"){
        ui->chk_auto_start_->setChecked(false);
    } else {
        ui->chk_auto_start_->setChecked(true);
    }

    QSettings runsettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    value = runsettings.value("FarmLogAgent").toString();
    if (!value.isEmpty()) {
        ui->chk_start_->setChecked(true);
    } else {
        ui->chk_start_->setChecked(false);
    }
}

void MainWindow::receive_data(QString dateTime) {
    QString viewText = "최근 데이터 수집 : " + dateTime;
    ui->lbl_current_->setText(viewText);
}
