#include "doctorchatwidget.h"
#include "loginwidget.h"

#include <QApplication>
#include <QString>

static QString cyberStyleSheet()
{
    return QStringLiteral(R"(
        QWidget#LoginWidget QLabel, QWidget#DoctorChatWidget QLabel,
        QWidget#SettingsWidget_Doc QLabel, QDialog#HistoryDialog QLabel {
            color: #D8F7FF;
            font-family: "Microsoft YaHei";
        }
        QWidget#LoginWidget QPushButton, QWidget#DoctorChatWidget QPushButton,
        QWidget#SettingsWidget_Doc QPushButton, QDialog#HistoryDialog QPushButton {
            background: rgba(6, 24, 45, 0.92);
            color: #D8F7FF;
            border: 1px solid rgba(0, 229, 255, 0.62);
            border-radius: 14px;
            padding: 8px 16px;
            font-family: "Microsoft YaHei";
            font-weight: 700;
            min-height: 30px;
        }
        QWidget#LoginWidget QPushButton:hover, QWidget#DoctorChatWidget QPushButton:hover,
        QWidget#SettingsWidget_Doc QPushButton:hover, QDialog#HistoryDialog QPushButton:hover {
            background: rgba(0, 229, 255, 0.18);
            border-color: #00E5FF;
        }
        QWidget#LoginWidget QPushButton:pressed, QWidget#DoctorChatWidget QPushButton:pressed,
        QWidget#SettingsWidget_Doc QPushButton:pressed, QDialog#HistoryDialog QPushButton:pressed {
            background: rgba(49, 255, 183, 0.24);
            border-color: #31FFB7;
        }
        QWidget#LoginWidget QPushButton:checked, QWidget#DoctorChatWidget QPushButton:checked,
        QWidget#SettingsWidget_Doc QPushButton:checked, QDialog#HistoryDialog QPushButton:checked {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00E5FF, stop:1 #31FFB7);
            color: #03111D;
        }
        QWidget#LoginWidget QLineEdit, QWidget#DoctorChatWidget QLineEdit, QWidget#SettingsWidget_Doc QLineEdit,
        QWidget#DoctorChatWidget QTextBrowser, QDialog#HistoryDialog QTextBrowser,
        QWidget#SettingsWidget_Doc QTextEdit, QWidget#SettingsWidget_Doc QListWidget,
        QWidget#SettingsWidget_Doc QComboBox, QWidget#SettingsWidget_Doc QSpinBox, QWidget#SettingsWidget_Doc QDateEdit {
            background: rgba(2, 9, 20, 0.86);
            color: #EAFBFF;
            border: 1px solid rgba(0, 229, 255, 0.38);
            border-radius: 14px;
            padding: 8px 12px;
            selection-background-color: #00E5FF;
            selection-color: #03111D;
            font-family: "Microsoft YaHei";
        }
        QWidget#LoginWidget QLineEdit:focus, QWidget#DoctorChatWidget QLineEdit:focus, QWidget#SettingsWidget_Doc QLineEdit:focus,
        QWidget#DoctorChatWidget QTextBrowser:focus, QDialog#HistoryDialog QTextBrowser:focus,
        QWidget#SettingsWidget_Doc QTextEdit:focus, QWidget#SettingsWidget_Doc QListWidget:focus,
        QWidget#SettingsWidget_Doc QComboBox:focus, QWidget#SettingsWidget_Doc QSpinBox:focus, QWidget#SettingsWidget_Doc QDateEdit:focus {
            border: 2px solid #00E5FF;
        }
        QWidget#SettingsWidget_Doc QGroupBox {
            color: #D8F7FF;
            border: 1px solid rgba(0, 229, 255, 0.35);
            border-radius: 16px;
            margin-top: 12px;
            padding: 12px;
            font-weight: 700;
            font-family: "Microsoft YaHei";
        }
        QWidget#SettingsWidget_Doc QGroupBox::title {
            subcontrol-origin: margin;
            left: 14px;
            padding: 0 8px;
            color: #00E5FF;
        }
        QWidget#SettingsWidget_Doc QCheckBox, QWidget#SettingsWidget_Doc QRadioButton {
            color: #D8F7FF;
            spacing: 8px;
            font-family: "Microsoft YaHei";
        }
    )");
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyleSheet(cyberStyleSheet());

    LoginWidget *login = new LoginWidget();
    DoctorChatWidget *doctorChat = nullptr;

    QObject::connect(login, &LoginWidget::localLoginSuccess, [&](const QString &username, const QString &password) {
        login->hide();
        if (doctorChat) {
            delete doctorChat;
        }

        doctorChat = new DoctorChatWidget();
        doctorChat->setCredentials(username, password);
        doctorChat->setWindowTitle(QStringLiteral("医生咨询中心 - %1").arg(username));
        QObject::connect(doctorChat, &QWidget::destroyed, [&]() {
            doctorChat = nullptr;
            login->show();
        });
        doctorChat->show();
    });

    login->show();
    return QApplication::exec();
}
