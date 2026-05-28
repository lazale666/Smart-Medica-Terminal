#ifndef DOCTORCHATWIDGET_H
#define DOCTORCHATWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>
#include <QVector>
#include "historydialog.h"
#include "settingswidget_doc.h"

namespace Ui {
class DoctorChatWidget;
}

class QScrollArea;
class QWidget;
class QVBoxLayout;
class QPushButton;
class QLabel;

struct DoctorSideChatMessage
{
    QString sender;
    QString message;
    bool isSelf;
    bool isSystem;
};

class DoctorChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DoctorChatWidget(QWidget *parent = nullptr);
    explicit DoctorChatWidget(QTcpSocket *existingSocket, QWidget *parent = nullptr);
    ~DoctorChatWidget();

    void setUsername(const QString &username);

private slots:
    void connectToServer();
    void readData();
    void on_sendBtn_clicked();
    void on_historyBtn_clicked();
    void on_settingsBtn_clicked();

private:
    void sendMessage(const QString &message);
    void initConnections();
    void setupMessageArea();
    void rebuildMessages();
    void showWelcomeState();
    void clearWelcomeStateIfNeeded();
    void updateScrollState();
    void updateNewMessageButtonVisibility(bool visible);
    void scrollToBottomAndClearReminder();
    void updateCurrentClientLabel(const QString &clientName);
    void setStatusText(const QString &text, const QString &color);
    QString getHistoryDir() const;
    void startHistorySession(const QString &clientName);
    void appendHistoryMessage(const QString &role, const QString &sender, const QString &message);
    void endHistorySession();
    void appendChatMessage(const QString &sender, const QString &message, bool isSelf);
    void appendSystemMessage(const QString &message);

private:
    Ui::DoctorChatWidget *ui;
    QScrollArea *m_messageScrollArea;
    QWidget *m_messageContent;
    QVBoxLayout *m_messageLayout;
    QPushButton *m_newMessageButton;
    QLabel *m_currentClientLabel;
    QTcpSocket *socket;
    bool m_externalSocket;
    QByteArray m_buffer;
    QVector<DoctorSideChatMessage> m_messages;
    QStringList messageHistory;
    QJsonArray userHistory;
    HistoryDialog *historyDialog;
    SettingsWidget_Doc *settingsWidget;
    QString m_username;
    QString m_currentClientName;
    QString m_currentHistoryFile;
    bool m_isUserNearBottom;
};

#endif // DOCTORCHATWIDGET_H
