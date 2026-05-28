#include "widget.h"
#include "loginwidget.h"
#include "menuwidget.h"
#include "medicalrecordwidget.h"
#include "doctorlistwidget.h"
#include "memberrechargewidget.h"

#include <QApplication>
#include <QSettings>
#include <QString>

static QString cyberStyleSheet()
{
    return QStringLiteral(R"(
        QWidget#LoginWidget QLabel, QWidget#MenuWidget QLabel, QWidget#Widget QLabel,
        QWidget#DoctorListWidget QLabel, QDialog#DoctorDialog QLabel,
        QWidget#SettingsWidget QLabel, QWidget#MedicalRecordWidget QLabel,
        QWidget#RecordDetailWidget QLabel, QWidget#MemberRechargeWidget QLabel {
            color: #D8F7FF;
            font-family: "Microsoft YaHei";
        }
        QWidget#LoginWidget QPushButton, QWidget#MenuWidget QPushButton, QWidget#Widget QPushButton,
        QWidget#DoctorListWidget QPushButton, QDialog#DoctorDialog QPushButton,
        QWidget#SettingsWidget QPushButton, QWidget#MedicalRecordWidget QPushButton,
        QWidget#RecordDetailWidget QPushButton, QWidget#MemberRechargeWidget QPushButton {
            background: rgba(6, 24, 45, 0.92);
            color: #D8F7FF;
            border: 1px solid rgba(0, 229, 255, 0.62);
            border-radius: 14px;
            padding: 8px 16px;
            font-family: "Microsoft YaHei";
            font-weight: 700;
            min-height: 30px;
        }
        QWidget#LoginWidget QPushButton:hover, QWidget#MenuWidget QPushButton:hover, QWidget#Widget QPushButton:hover,
        QWidget#DoctorListWidget QPushButton:hover, QDialog#DoctorDialog QPushButton:hover,
        QWidget#SettingsWidget QPushButton:hover, QWidget#MedicalRecordWidget QPushButton:hover,
        QWidget#RecordDetailWidget QPushButton:hover, QWidget#MemberRechargeWidget QPushButton:hover {
            background: rgba(0, 229, 255, 0.18);
            border-color: #00E5FF;
        }
        QWidget#LoginWidget QPushButton:pressed, QWidget#MenuWidget QPushButton:pressed, QWidget#Widget QPushButton:pressed,
        QWidget#DoctorListWidget QPushButton:pressed, QDialog#DoctorDialog QPushButton:pressed,
        QWidget#SettingsWidget QPushButton:pressed, QWidget#MedicalRecordWidget QPushButton:pressed,
        QWidget#RecordDetailWidget QPushButton:pressed, QWidget#MemberRechargeWidget QPushButton:pressed {
            background: rgba(49, 255, 183, 0.24);
            border-color: #31FFB7;
        }
        QWidget#LoginWidget QPushButton:checked, QWidget#MenuWidget QPushButton:checked, QWidget#Widget QPushButton:checked,
        QWidget#DoctorListWidget QPushButton:checked, QDialog#DoctorDialog QPushButton:checked,
        QWidget#SettingsWidget QPushButton:checked, QWidget#MedicalRecordWidget QPushButton:checked,
        QWidget#RecordDetailWidget QPushButton:checked, QWidget#MemberRechargeWidget QPushButton:checked {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00E5FF, stop:1 #31FFB7);
            color: #03111D;
        }
        QWidget#LoginWidget QLineEdit, QWidget#MenuWidget QLineEdit, QWidget#Widget QLineEdit,
        QWidget#DoctorListWidget QLineEdit, QDialog#DoctorDialog QLineEdit,
        QWidget#SettingsWidget QLineEdit, QWidget#MedicalRecordWidget QLineEdit,
        QWidget#RecordDetailWidget QLineEdit, QWidget#MemberRechargeWidget QLineEdit,
        QWidget#Widget QTextBrowser, QWidget#DoctorListWidget QTextBrowser, QDialog#DoctorDialog QTextBrowser,
        QWidget#SettingsWidget QTextBrowser, QWidget#MedicalRecordWidget QTextBrowser, QWidget#RecordDetailWidget QTextBrowser,
        QWidget#SettingsWidget QTextEdit, QWidget#MedicalRecordWidget QTextEdit, QWidget#RecordDetailWidget QTextEdit,
        QWidget#Widget QListWidget, QWidget#DoctorListWidget QListWidget, QWidget#SettingsWidget QListWidget,
        QWidget#MedicalRecordWidget QListWidget, QWidget#SettingsWidget QComboBox, QWidget#MedicalRecordWidget QComboBox,
        QWidget#RecordDetailWidget QComboBox, QWidget#SettingsWidget QSpinBox, QWidget#RecordDetailWidget QDateEdit,
        QWidget#MedicalRecordWidget QDateEdit {
            background: rgba(2, 9, 20, 0.86);
            color: #EAFBFF;
            border: 1px solid rgba(0, 229, 255, 0.38);
            border-radius: 14px;
            padding: 8px 12px;
            selection-background-color: #00E5FF;
            selection-color: #03111D;
            font-family: "Microsoft YaHei";
        }
        QWidget#LoginWidget QLineEdit:focus, QWidget#MenuWidget QLineEdit:focus, QWidget#Widget QLineEdit:focus,
        QWidget#DoctorListWidget QLineEdit:focus, QDialog#DoctorDialog QLineEdit:focus,
        QWidget#SettingsWidget QLineEdit:focus, QWidget#MedicalRecordWidget QLineEdit:focus,
        QWidget#RecordDetailWidget QLineEdit:focus, QWidget#MemberRechargeWidget QLineEdit:focus,
        QWidget#Widget QTextBrowser:focus, QWidget#DoctorListWidget QTextBrowser:focus, QDialog#DoctorDialog QTextBrowser:focus,
        QWidget#SettingsWidget QTextEdit:focus, QWidget#MedicalRecordWidget QTextEdit:focus, QWidget#RecordDetailWidget QTextEdit:focus,
        QWidget#Widget QListWidget:focus, QWidget#DoctorListWidget QListWidget:focus, QWidget#SettingsWidget QListWidget:focus,
        QWidget#MedicalRecordWidget QListWidget:focus, QWidget#SettingsWidget QComboBox:focus, QWidget#MedicalRecordWidget QComboBox:focus,
        QWidget#SettingsWidget QSpinBox:focus, QWidget#RecordDetailWidget QDateEdit:focus, QWidget#MedicalRecordWidget QDateEdit:focus {
            border: 2px solid #00E5FF;
        }
        QWidget#SettingsWidget QGroupBox, QWidget#MedicalRecordWidget QGroupBox {
            color: #D8F7FF;
            border: 1px solid rgba(0, 229, 255, 0.35);
            border-radius: 16px;
            margin-top: 12px;
            padding: 12px;
            font-weight: 700;
            font-family: "Microsoft YaHei";
        }
        QWidget#SettingsWidget QGroupBox::title, QWidget#MedicalRecordWidget QGroupBox::title {
            subcontrol-origin: margin;
            left: 14px;
            padding: 0 8px;
            color: #00E5FF;
        }
        QWidget#SettingsWidget QCheckBox, QWidget#MedicalRecordWidget QCheckBox,
        QWidget#SettingsWidget QRadioButton, QWidget#MedicalRecordWidget QRadioButton {
            color: #D8F7FF;
            spacing: 8px;
            font-family: "Microsoft YaHei";
        }
        QWidget#SettingsWidget QSlider::groove:horizontal {
            height: 8px;
            background: rgba(0, 229, 255, 0.18);
            border-radius: 4px;
        }
        QWidget#SettingsWidget QSlider::handle:horizontal {
            width: 18px;
            margin: -5px 0;
            border-radius: 9px;
            background: #31FFB7;
        }
    )");
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyleSheet(cyberStyleSheet());

    LoginWidget *login = new LoginWidget();
    MenuWidget *menu = new MenuWidget();
    Widget *chat = new Widget();
    MedicalRecordWidget *medicalRecord = new MedicalRecordWidget();
    DoctorListWidget *doctorList = nullptr;
    MemberRechargeWidget *memberRecharge = nullptr;

    QSettings settings("SmartMedica", "Client");
    QString savedMode = settings.value("mode", "普通模式").toString();

    QObject::connect(login, &LoginWidget::loginSuccess, [=](const QString &username) {
        login->hide();
        menu->setUsername(username);
        QSettings settings("SmartMedica", "Client");
        QString serverIP = settings.value("serverIP", "127.0.0.1").toString();
        int serverPort = settings.value("serverPort", 9999).toInt();
        menu->setServerInfo(serverIP, serverPort);
        menu->setWindowTitle("医疗智能体 - " + username);
        menu->show();
    });

    QObject::connect(menu, &MenuWidget::openChat, [=](const QString &serverIP, int serverPort, bool autoConnect) {
        menu->hide();
        chat->setServerInfo(serverIP, serverPort, autoConnect);
        chat->setWindowTitle("医疗智能体 - 聊天问诊");
        chat->show();
    });

    QObject::connect(menu, &MenuWidget::openMedicalRecord, [=](const QString &serverIP, int serverPort, bool autoConnect) {
        menu->hide();
        medicalRecord->setServerInfo(serverIP, serverPort);
        medicalRecord->applyModeSettings(savedMode);
        medicalRecord->setWindowTitle("医疗智能体 - 病例记录");
        medicalRecord->show();
    });

    QObject::connect(menu, &MenuWidget::openDoctorChat, [&](const QString &serverIP, int serverPort) {
        menu->hide();
        if (doctorList) {
            delete doctorList;
        }
        doctorList = new DoctorListWidget(serverIP, serverPort, menu->getUsername());
        doctorList->setWindowTitle("医疗智能体 - 名医对话");
        
        QObject::connect(doctorList, &DoctorListWidget::backToMenu, [&]() {
            doctorList->hide();
            menu->show();
        });
        
        doctorList->show();
    });

    QObject::connect(menu, &MenuWidget::openMemberRecharge, [&]() {
        menu->hide();
        if (memberRecharge) {
            delete memberRecharge;
        }
        memberRecharge = new MemberRechargeWidget();
        memberRecharge->setUsername(menu->getUsername());
        memberRecharge->setWindowTitle("医疗智能体 - 会员充值");
        
        QObject::connect(memberRecharge, &MemberRechargeWidget::backToMenu, [&]() {
            memberRecharge->hide();
            menu->show();
        });
        
        memberRecharge->show();
    });

    QObject::connect(menu, &MenuWidget::logout, [=]() {
        menu->hide();
        login->show();
    });

    QObject::connect(chat, &Widget::backToMenu, [=]() {
        chat->hide();
        menu->show();
    });

    QObject::connect(chat, &Widget::logout, [=]() {
        chat->hide();
        login->show();
    });

    QObject::connect(medicalRecord, &MedicalRecordWidget::backToMenu, [=]() {
        medicalRecord->hide();
        menu->show();
    });

    QObject::connect(medicalRecord, &MedicalRecordWidget::logout, [=]() {
        medicalRecord->hide();
        login->show();
    });

    login->show();

    return QApplication::exec();
}
