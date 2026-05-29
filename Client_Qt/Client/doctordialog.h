#ifndef DOCTORDIALOG_H
#define DOCTORDIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVector>

namespace Ui {
class DoctorDialog;
}

class QScrollArea;
class QWidget;
class QVBoxLayout;
class QPushButton;

struct DoctorChatMessage
{
    QString sender;
    QString message;
    bool isSelf;
    bool isSystem;
};

class DoctorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DoctorDialog(QTcpSocket *socket, const QString &username, const QString &doctorName,
                          const QString &sessionId, QWidget *parent = nullptr);
    ~DoctorDialog();
    void applyAppearance(const QString &mode, const QString &bgColor, const QString &fontColor);
    void applyModeSettings(const QString &mode);

private slots:
    void readData();
    void onSendBtnClicked();
    void onCloseBtnClicked();

private:
    Ui::DoctorDialog *ui;
    QScrollArea *m_messageScrollArea;
    QWidget *m_messageContent;
    QVBoxLayout *m_messageLayout;
    QPushButton *m_newMessageButton;
    QTcpSocket *m_socket;
    QString m_username;
    QString m_doctorName;
    QString m_sessionId;
    QString m_currentMode;
    QString m_bgColor;
    QString m_fontColor;
    QByteArray m_buffer;
    QVector<DoctorChatMessage> m_messages;
    bool m_isUserNearBottom;

    void sendMessage(const QString &message);
    void setupMessageArea();
    void rebuildMessages();
    void updateScrollState();
    void updateNewMessageButtonVisibility(bool visible);
    void scrollToBottomAndClearReminder();
    void appendChatMessage(const QString &sender, const QString &message, bool isSelf);
    void appendSystemMessage(const QString &message);
};

#endif // DOCTORDIALOG_H
