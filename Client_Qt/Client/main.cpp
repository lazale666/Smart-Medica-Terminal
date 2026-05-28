#include "widget.h"
#include "loginwidget.h"
#include "menuwidget.h"
#include "medicalrecordwidget.h"
#include "doctorlistwidget.h"
#include "doctordialog.h"
#include "memberrechargewidget.h"
#include "facerecognizewidget.h"
#include "dialog.h"
#include "recorddetailwidget.h"
#include "themehelpers.h"

#include <QApplication>
#include <QSettings>
#include <QString>

static QString appThemeStyleSheet(const QString &bgColor, const QString &fontColor)
{
    const bool light = ThemeHelpers::isLightTheme(bgColor);
    const QString buttonBg = light ? "rgba(255, 255, 255, 0.92)" : "rgba(6, 24, 45, 0.92)";
    const QString buttonHover = light ? "rgba(199, 244, 255, 0.96)" : "rgba(0, 229, 255, 0.18)";
    const QString buttonPressed = light ? "rgba(183, 236, 255, 1.0)" : "rgba(49, 255, 183, 0.24)";
    const QString buttonCheckedText = "#03111D";
    const QString borderColor = light ? "rgba(15, 39, 64, 0.18)" : "rgba(0, 229, 255, 0.62)";
    const QString inputBg = light ? "rgba(255, 255, 255, 0.94)" : "rgba(2, 9, 20, 0.86)";
    const QString inputText = light ? "#0F2740" : "#EAFBFF";
    const QString selectionBg = light ? "#7FD9FF" : "#00E5FF";
    const QString selectionText = "#03111D";
    const QString groupTitle = ThemeHelpers::titleColor(bgColor);
    const QString groupBorder = light ? "rgba(15, 39, 64, 0.16)" : "rgba(0, 229, 255, 0.35)";
    const QString sliderGroove = light ? "rgba(15, 39, 64, 0.16)" : "rgba(0, 229, 255, 0.18)";
    const QString sliderHandle = light ? "#0F9FD9" : "#31FFB7";

    return QStringLiteral(R"(
        QWidget#LoginWidget QLabel, QWidget#MenuWidget QLabel, QWidget#Widget QLabel,
        QWidget#DoctorListWidget QLabel, QDialog#DoctorDialog QLabel,
        QWidget#SettingsWidget QLabel, QWidget#MedicalRecordWidget QLabel,
        QWidget#RecordDetailWidget QLabel, QWidget#MemberRechargeWidget QLabel,
        QWidget#FaceRecognizeWidget QLabel, QDialog#Dialog QLabel {
            color: %1;
            font-family: "Microsoft YaHei";
        }
        QWidget#LoginWidget QPushButton, QWidget#MenuWidget QPushButton, QWidget#Widget QPushButton,
        QWidget#DoctorListWidget QPushButton, QDialog#DoctorDialog QPushButton,
        QWidget#SettingsWidget QPushButton, QWidget#MedicalRecordWidget QPushButton,
        QWidget#RecordDetailWidget QPushButton, QWidget#MemberRechargeWidget QPushButton,
        QWidget#FaceRecognizeWidget QPushButton {
            background: %2;
            color: %1;
            border: 1px solid %3;
            border-radius: 14px;
            padding: 8px 16px;
            font-family: "Microsoft YaHei";
            font-weight: 700;
            min-height: 30px;
        }
        QWidget#LoginWidget QPushButton:hover, QWidget#MenuWidget QPushButton:hover, QWidget#Widget QPushButton:hover,
        QWidget#DoctorListWidget QPushButton:hover, QDialog#DoctorDialog QPushButton:hover,
        QWidget#SettingsWidget QPushButton:hover, QWidget#MedicalRecordWidget QPushButton:hover,
        QWidget#RecordDetailWidget QPushButton:hover, QWidget#MemberRechargeWidget QPushButton:hover,
        QWidget#FaceRecognizeWidget QPushButton:hover {
            background: %4;
            border-color: #00E5FF;
        }
        QWidget#LoginWidget QPushButton:pressed, QWidget#MenuWidget QPushButton:pressed, QWidget#Widget QPushButton:pressed,
        QWidget#DoctorListWidget QPushButton:pressed, QDialog#DoctorDialog QPushButton:pressed,
        QWidget#SettingsWidget QPushButton:pressed, QWidget#MedicalRecordWidget QPushButton:pressed,
        QWidget#RecordDetailWidget QPushButton:pressed, QWidget#MemberRechargeWidget QPushButton:pressed,
        QWidget#FaceRecognizeWidget QPushButton:pressed {
            background: %5;
            border-color: #31FFB7;
        }
        QWidget#LoginWidget QPushButton:checked, QWidget#MenuWidget QPushButton:checked, QWidget#Widget QPushButton:checked,
        QWidget#DoctorListWidget QPushButton:checked, QDialog#DoctorDialog QPushButton:checked,
        QWidget#SettingsWidget QPushButton:checked, QWidget#MedicalRecordWidget QPushButton:checked,
        QWidget#RecordDetailWidget QPushButton:checked, QWidget#MemberRechargeWidget QPushButton:checked,
        QWidget#FaceRecognizeWidget QPushButton:checked {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #00E5FF, stop:1 #31FFB7);
            color: %6;
        }
        QWidget#LoginWidget QLineEdit, QWidget#MenuWidget QLineEdit, QWidget#Widget QLineEdit,
        QWidget#DoctorListWidget QLineEdit, QDialog#DoctorDialog QLineEdit,
        QWidget#SettingsWidget QLineEdit, QWidget#MedicalRecordWidget QLineEdit,
        QWidget#RecordDetailWidget QLineEdit, QWidget#MemberRechargeWidget QLineEdit,
        QWidget#FaceRecognizeWidget QLineEdit, QWidget#Widget QTextBrowser,
        QWidget#DoctorListWidget QTextBrowser, QDialog#DoctorDialog QTextBrowser,
        QWidget#SettingsWidget QTextBrowser, QWidget#MedicalRecordWidget QTextBrowser,
        QWidget#RecordDetailWidget QTextBrowser, QWidget#SettingsWidget QTextEdit,
        QWidget#MedicalRecordWidget QTextEdit, QWidget#RecordDetailWidget QTextEdit,
        QWidget#Widget QListWidget, QWidget#DoctorListWidget QListWidget,
        QWidget#SettingsWidget QListWidget, QWidget#MedicalRecordWidget QListWidget,
        QWidget#SettingsWidget QComboBox, QWidget#MedicalRecordWidget QComboBox,
        QWidget#RecordDetailWidget QComboBox, QWidget#SettingsWidget QSpinBox,
        QWidget#RecordDetailWidget QDateEdit, QWidget#MedicalRecordWidget QDateEdit,
        QWidget#MedicalRecordWidget QDateTimeEdit, QWidget#RecordDetailWidget QDateTimeEdit {
            background: %7;
            color: %8;
            border: 1px solid rgba(0, 229, 255, 0.38);
            border-radius: 14px;
            padding: 8px 12px;
            selection-background-color: %9;
            selection-color: %10;
            font-family: "Microsoft YaHei";
        }
        QWidget#LoginWidget QLineEdit:focus, QWidget#MenuWidget QLineEdit:focus, QWidget#Widget QLineEdit:focus,
        QWidget#DoctorListWidget QLineEdit:focus, QDialog#DoctorDialog QLineEdit:focus,
        QWidget#SettingsWidget QLineEdit:focus, QWidget#MedicalRecordWidget QLineEdit:focus,
        QWidget#RecordDetailWidget QLineEdit:focus, QWidget#MemberRechargeWidget QLineEdit:focus,
        QWidget#FaceRecognizeWidget QLineEdit:focus, QWidget#Widget QTextBrowser:focus,
        QWidget#DoctorListWidget QTextBrowser:focus, QDialog#DoctorDialog QTextBrowser:focus,
        QWidget#SettingsWidget QTextEdit:focus, QWidget#MedicalRecordWidget QTextEdit:focus,
        QWidget#RecordDetailWidget QTextEdit:focus, QWidget#Widget QListWidget:focus,
        QWidget#DoctorListWidget QListWidget:focus, QWidget#SettingsWidget QListWidget:focus,
        QWidget#MedicalRecordWidget QListWidget:focus, QWidget#SettingsWidget QComboBox:focus,
        QWidget#MedicalRecordWidget QComboBox:focus, QWidget#SettingsWidget QSpinBox:focus,
        QWidget#RecordDetailWidget QDateEdit:focus, QWidget#MedicalRecordWidget QDateEdit:focus,
        QWidget#MedicalRecordWidget QDateTimeEdit:focus, QWidget#RecordDetailWidget QDateTimeEdit:focus {
            border: 2px solid #00E5FF;
        }
        QWidget#SettingsWidget QGroupBox, QWidget#MedicalRecordWidget QGroupBox {
            color: %1;
            border: 1px solid %12;
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
            color: %11;
        }
        QWidget#SettingsWidget QCheckBox, QWidget#MedicalRecordWidget QCheckBox,
        QWidget#SettingsWidget QRadioButton, QWidget#MedicalRecordWidget QRadioButton {
            color: %1;
            spacing: 8px;
            font-family: "Microsoft YaHei";
        }
        QWidget#SettingsWidget QSlider::groove:horizontal {
            height: 8px;
            background: %13;
            border-radius: 4px;
        }
        QWidget#SettingsWidget QSlider::handle:horizontal {
            width: 18px;
            margin: -5px 0;
            border-radius: 9px;
            background: %14;
        }
    )")
        .arg(fontColor, buttonBg, borderColor, buttonHover, buttonPressed,
             buttonCheckedText, inputBg, inputText, selectionBg, selectionText,
             groupTitle, groupBorder, sliderGroove, sliderHandle);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LoginWidget *login = new LoginWidget();
    MenuWidget *menu = new MenuWidget();
    Widget *chat = new Widget();
    MedicalRecordWidget *medicalRecord = new MedicalRecordWidget();
    DoctorListWidget *doctorList = nullptr;
    MemberRechargeWidget *memberRecharge = nullptr;

    auto applyClientAppearance = [&](const QString &mode, const QString &bgColor, const QString &fontColor) {
        const QString normalizedBg = ThemeHelpers::normalizeBgColor(bgColor);
        const QString normalizedFont = fontColor.isEmpty()
            ? ThemeHelpers::defaultFontColorForBg(normalizedBg)
            : fontColor;

        a.setStyleSheet(appThemeStyleSheet(normalizedBg, normalizedFont));

        login->applyAppearance(normalizedBg, normalizedFont);
        menu->applyAppearance(mode, normalizedBg, normalizedFont);
        chat->applyAppearance(mode, normalizedBg, normalizedFont);
        medicalRecord->applyAppearance(mode, normalizedBg, normalizedFont);

        if (doctorList) {
            doctorList->applyAppearance(mode, normalizedBg, normalizedFont);
        }
        if (memberRecharge) {
            memberRecharge->applyAppearance(mode, normalizedBg, normalizedFont);
        }

        for (QWidget *widget : QApplication::topLevelWidgets()) {
            if (auto detail = qobject_cast<RecordDetailWidget *>(widget)) {
                detail->applyAppearance(mode, normalizedBg, normalizedFont);
            } else if (auto dialog = qobject_cast<DoctorDialog *>(widget)) {
                dialog->applyAppearance(mode, normalizedBg, normalizedFont);
            } else if (auto face = qobject_cast<FaceRecognizeWidget *>(widget)) {
                face->applyAppearance(mode, normalizedBg, normalizedFont);
            } else if (auto reconnectDialog = qobject_cast<Dialog *>(widget)) {
                reconnectDialog->applyAppearance(normalizedBg, normalizedFont);
            }
        }
    };

    auto applyAppearanceFromSettings = [&]() {
        QSettings settings("SmartMedica", "Client");
        const QString mode = settings.value("mode", "普通模式").toString();
        const QString bgColor = ThemeHelpers::normalizeBgColor(settings.value("bgColor", "#07111F").toString());
        const QString fontColor = settings.value("fontColor", ThemeHelpers::defaultFontColorForBg(bgColor)).toString();
        applyClientAppearance(mode, bgColor, fontColor);
    };

    applyAppearanceFromSettings();

    QObject::connect(login, &LoginWidget::loginSuccess, [=](const QString &username) {
        login->hide();
        menu->setUsername(username);
        medicalRecord->setUsername(username);

        QSettings settings("SmartMedica", "Client");
        const QString mode = settings.value("mode", "普通模式").toString();
        const QString bgColor = ThemeHelpers::normalizeBgColor(settings.value("bgColor", "#07111F").toString());
        const QString fontColor = settings.value("fontColor", ThemeHelpers::defaultFontColorForBg(bgColor)).toString();
        const QString serverIP = settings.value("serverIP", "127.0.0.1").toString();
        const int serverPort = settings.value("serverPort", 9999).toInt();

        menu->setServerInfo(serverIP, serverPort);
        menu->applyAppearance(mode, bgColor, fontColor);
        menu->setWindowTitle("医疗智能体 - " + username);
        menu->show();
        menu->ensureServerConnected();
    });

    QObject::connect(menu, &MenuWidget::openChat, [=](const QString &serverIP, int serverPort, bool autoConnect) {
        menu->hide();
        chat->setUsername(menu->getUsername());
        chat->setServerInfo(serverIP, serverPort, autoConnect);

        QSettings settings("SmartMedica", "Client");
        const QString mode = settings.value("mode", "普通模式").toString();
        const QString bgColor = ThemeHelpers::normalizeBgColor(settings.value("bgColor", "#07111F").toString());
        const QString fontColor = settings.value("fontColor", ThemeHelpers::defaultFontColorForBg(bgColor)).toString();
        chat->applyAppearance(mode, bgColor, fontColor);
        chat->setWindowTitle("医疗智能体 - 智能问诊");
        chat->show();
    });

    QObject::connect(menu, &MenuWidget::openMedicalRecord, [=](const QString &serverIP, int serverPort, bool autoConnect) {
        Q_UNUSED(autoConnect);
        menu->hide();
        medicalRecord->setUsername(menu->getUsername());
        medicalRecord->setServerInfo(serverIP, serverPort);

        QSettings settings("SmartMedica", "Client");
        const QString mode = settings.value("mode", "普通模式").toString();
        const QString bgColor = ThemeHelpers::normalizeBgColor(settings.value("bgColor", "#07111F").toString());
        const QString fontColor = settings.value("fontColor", ThemeHelpers::defaultFontColorForBg(bgColor)).toString();
        medicalRecord->applyAppearance(mode, bgColor, fontColor);
        medicalRecord->setWindowTitle("医疗智能体 - 病例记录");
        medicalRecord->show();
    });

    QObject::connect(menu, &MenuWidget::openDoctorChat, [&](const QString &serverIP, int serverPort) {
        menu->hide();
        if (doctorList) {
            delete doctorList;
        }
        doctorList = new DoctorListWidget(serverIP, serverPort, menu->getUsername());

        QSettings settings("SmartMedica", "Client");
        const QString mode = settings.value("mode", "普通模式").toString();
        const QString bgColor = ThemeHelpers::normalizeBgColor(settings.value("bgColor", "#07111F").toString());
        const QString fontColor = settings.value("fontColor", ThemeHelpers::defaultFontColorForBg(bgColor)).toString();
        doctorList->applyAppearance(mode, bgColor, fontColor);
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

        QSettings settings("SmartMedica", "Client");
        const QString mode = settings.value("mode", "普通模式").toString();
        const QString bgColor = ThemeHelpers::normalizeBgColor(settings.value("bgColor", "#07111F").toString());
        const QString fontColor = settings.value("fontColor", ThemeHelpers::defaultFontColorForBg(bgColor)).toString();
        memberRecharge->applyAppearance(mode, bgColor, fontColor);
        memberRecharge->setWindowTitle("医疗智能体 - 会员充值");

        QObject::connect(memberRecharge, &MemberRechargeWidget::backToMenu, [&]() {
            memberRecharge->hide();
            menu->show();
        });

        memberRecharge->show();
    });

    QObject::connect(menu, &MenuWidget::logout, [=]() {
        menu->hide();
        applyAppearanceFromSettings();
        login->show();
    });

    QObject::connect(menu, &MenuWidget::appearanceChanged, [&](const QString &mode, const QString &bgColor, const QString &fontColor) {
        applyClientAppearance(mode, bgColor, fontColor);
    });

    QObject::connect(chat, &Widget::backToMenu, [=]() {
        chat->hide();
        menu->show();
    });

    QObject::connect(chat, &Widget::logout, [=]() {
        chat->hide();
        applyAppearanceFromSettings();
        login->show();
    });

    QObject::connect(chat, &Widget::appearanceChanged, [&](const QString &mode, const QString &bgColor, const QString &fontColor) {
        applyClientAppearance(mode, bgColor, fontColor);
    });

    QObject::connect(medicalRecord, &MedicalRecordWidget::backToMenu, [=]() {
        medicalRecord->hide();
        menu->show();
    });

    QObject::connect(medicalRecord, &MedicalRecordWidget::logout, [=]() {
        medicalRecord->hide();
        applyAppearanceFromSettings();
        login->show();
    });

    QObject::connect(medicalRecord, &MedicalRecordWidget::appearanceChanged, [&](const QString &mode, const QString &bgColor, const QString &fontColor) {
        applyClientAppearance(mode, bgColor, fontColor);
    });

    login->show();

    return QApplication::exec();
}
