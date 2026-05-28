#ifndef DOCTORCHATWIDGET_H
#define DOCTORCHATWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>
#include <QVector>
#include <QHash>
#include "../Client/recorddetailwidget.h"
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
class QListWidget;
class QListWidgetItem;

struct DoctorSideChatMessage
{
    QString sender;
    QString message;
    bool isSelf;
    bool isSystem;
};

struct DoctorConversationState
{
    QString sessionId;
    QString clientName;
    QVector<DoctorSideChatMessage> messages;
    QString historyFile;
    int unreadCount = 0;
    bool historyStarted = false;
    QString lastPreview;
};

class DoctorChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DoctorChatWidget(QWidget *parent = nullptr);
    explicit DoctorChatWidget(QTcpSocket *existingSocket, QWidget *parent = nullptr);
    ~DoctorChatWidget();

    void setUsername(const QString &username);
    void setCredentials(const QString &username, const QString &password);
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void connectToServer();
    void readData();
    void on_sendBtn_clicked();
    void on_historyBtn_clicked();
    void on_settingsBtn_clicked();
    void on_viewRecordBtn_clicked();
    void onSessionItemClicked(QListWidgetItem *item);

private:
    bool sendMessage(const QString &sessionId, const QString &message);
    void sendLoginRequest();
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
    QString startHistorySession(const QString &clientName);
    void appendHistoryMessage(const QString &historyFile, const QString &role, const QString &sender, const QString &message);
    void endHistorySession(const QString &sessionId, bool appendDisconnectedMessage);
    void ensureConversationExists(const QString &sessionId, const QString &clientName);
    void switchToConversation(const QString &sessionId);
    void updateSessionList();
    QString summarizePreview(const QString &message) const;
    QString displayNameForClient(const QString &clientName) const;
    QString sanitizeUserName(const QString &username) const;
    QString recordDirForClient(const QString &clientName) const;
    QStringList availableRecordsForClient(const QString &clientName) const;
    bool loadRecordDataForClient(const QString &clientName, const QString &fileName, QString &diseaseName, QString &diagnosisDate, QString &treatment) const;
    void appendChatMessage(const QString &sender, const QString &message, bool isSelf);
    void appendSystemMessage(const QString &message);

private:
    Ui::DoctorChatWidget *ui;
    QListWidget *m_sessionList;
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
    RecordDetailWidget *m_recordDetailWidget;
    SettingsWidget_Doc *settingsWidget;
    QString m_username;
    QString m_password;
    QString m_currentClientName;
    QString m_activeConversationSessionId;
    bool m_hasActiveClient;
    bool m_isUserNearBottom;
    QHash<QString, DoctorConversationState> m_conversations;
};

#endif // DOCTORCHATWIDGET_H
