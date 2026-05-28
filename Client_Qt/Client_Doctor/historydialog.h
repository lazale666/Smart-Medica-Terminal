#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QList>

namespace Ui {
class HistoryDialog;
}

class QScrollArea;
class QWidget;
class QVBoxLayout;
class QListWidget;
class QListWidgetItem;

struct DoctorHistoryMessage
{
    QString sender;
    QString message;
    bool isSelf;
    bool isSystem;
};

class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryDialog(QWidget *parent = nullptr);
    ~HistoryDialog();

    void setHistoryData(const QJsonArray &history);
    void loadHistoryFromDir(const QString &historyDir);

private slots:
    void onSessionItemClicked(QListWidgetItem *item);

private:
    Ui::HistoryDialog *ui;
    QListWidget *m_sessionList;
    QScrollArea *m_messageScrollArea;
    QWidget *m_messageContent;
    QVBoxLayout *m_messageLayout;
    QVector<DoctorHistoryMessage> m_messages;
    QString m_historyDir;

    void setupMessageArea();
    void rebuildMessages();
    void loadSessionFile(const QString &filePath);
    QString sessionTitleFromFile(const QString &filePath) const;
};

#endif // HISTORYDIALOG_H
