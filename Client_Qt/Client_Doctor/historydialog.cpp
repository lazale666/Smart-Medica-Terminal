#include "historydialog.h"
#include "ui_historydialog.h"
#include "../Client/chatmessagewidgets.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QScrollArea>
#include <QStringConverter>
#include <QTextStream>
#include <QVBoxLayout>

HistoryDialog::HistoryDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HistoryDialog)
    , m_sessionList(nullptr)
    , m_messageScrollArea(nullptr)
    , m_messageContent(nullptr)
    , m_messageLayout(nullptr)
{
    ui->setupUi(this);
    setMinimumSize(800, 600);
    setStyleSheet(QString());
    ui->headerWidget->setStyleSheet(QString());
    ui->buttonWidget->setStyleSheet(QString());
    if (ui->textBrowser) {
        ui->textBrowser->setStyleSheet(QString());
    }
    setStyleSheet(R"(
        QDialog#HistoryDialog {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111F, stop:0.55 #071B2F, stop:1 #0B1023);
        }
        QWidget#headerWidget, QWidget#buttonWidget {
            background: rgba(4, 15, 31, 0.82);
            border: 1px solid rgba(0, 229, 255, 0.35);
            border-radius: 18px;
        }
        QLabel#titleLabel {
            color: #00E5FF;
            font: 700 22px "Microsoft YaHei";
        }
        QScrollArea#messageScrollArea {
            background: rgba(2, 9, 20, 0.86);
            border: 1px solid rgba(0, 229, 255, 0.35);
            border-radius: 18px;
        }
        QListWidget#sessionList {
            background: rgba(2, 9, 20, 0.86);
            border: 1px solid rgba(0, 229, 255, 0.35);
            border-radius: 18px;
            color: #EAFBFF;
            padding: 8px;
            font: 13px "Microsoft YaHei";
        }
        QListWidget#sessionList::item {
            padding: 10px 8px;
            border-radius: 10px;
        }
        QListWidget#sessionList::item:selected {
            background: rgba(0, 229, 255, 0.22);
            color: #FFFFFF;
        }
        QWidget#messageContent {
            background: transparent;
        }
    )");

    ui->titleLabel->setText(QStringLiteral("用户历史对话记录"));
    ui->closeBtn->setText(QStringLiteral("关闭"));
    setupMessageArea();
    connect(ui->closeBtn, &QPushButton::clicked, this, &HistoryDialog::close);
}

HistoryDialog::~HistoryDialog()
{
    delete ui;
}

void HistoryDialog::setupMessageArea()
{
    QWidget *historyPanel = new QWidget(this);
    historyPanel->setObjectName("historyPanel");
    QHBoxLayout *historyPanelLayout = new QHBoxLayout(historyPanel);
    historyPanelLayout->setContentsMargins(0, 0, 0, 0);
    historyPanelLayout->setSpacing(14);

    m_sessionList = new QListWidget(historyPanel);
    m_sessionList->setObjectName("sessionList");
    m_sessionList->setMinimumWidth(220);
    m_sessionList->setMaximumWidth(280);

    m_messageScrollArea = new QScrollArea(this);
    m_messageScrollArea->setObjectName("messageScrollArea");
    m_messageScrollArea->setWidgetResizable(true);
    m_messageScrollArea->setFrameShape(QFrame::NoFrame);
    m_messageScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_messageContent = new QWidget(m_messageScrollArea);
    m_messageContent->setObjectName("messageContent");
    m_messageLayout = new QVBoxLayout(m_messageContent);
    m_messageLayout->setContentsMargins(18, 18, 18, 18);
    m_messageLayout->setSpacing(2);
    m_messageLayout->addStretch();
    m_messageScrollArea->setWidget(m_messageContent);

    historyPanelLayout->addWidget(m_sessionList);
    historyPanelLayout->addWidget(m_messageScrollArea, 1);

    QVBoxLayout *rootLayout = qobject_cast<QVBoxLayout *>(layout());
    if (rootLayout) {
        rootLayout->replaceWidget(ui->textBrowser, historyPanel);
        ui->textBrowser->hide();
        ui->textBrowser->deleteLater();
        ui->textBrowser = nullptr;
    }

    connect(m_sessionList, &QListWidget::itemClicked, this, &HistoryDialog::onSessionItemClicked);
}

void HistoryDialog::rebuildMessages()
{
    if (!m_messageLayout || !m_messageContent || !m_messageScrollArea) {
        return;
    }

    clearLayoutWidgets(m_messageLayout);
    const ChatThemePalette palette = buildChatThemePalette(false);
    const int maxBubbleWidth = qMax(320, qRound(width() * 0.68));

    if (m_messages.isEmpty()) {
        m_messageLayout->addWidget(createSystemMessageWidget(QStringLiteral("暂无历史对话记录"), palette, m_messageContent));
    } else {
        for (const DoctorHistoryMessage &msg : std::as_const(m_messages)) {
            QWidget *item = msg.isSystem
                ? createSystemMessageWidget(msg.message, palette, m_messageContent)
                : createChatMessageWidget(msg.sender, msg.message, msg.isSelf, palette, maxBubbleWidth, m_messageContent);
            m_messageLayout->addWidget(item);
        }
    }

    m_messageLayout->addStretch();
    scrollAreaToBottom(m_messageScrollArea);
}

void HistoryDialog::setHistoryData(const QJsonArray &history)
{
    m_messages.clear();

    for (const QJsonValue &value : history) {
        if (value.isString()) {
            const QString text = value.toString();
            const bool isSelf = text.startsWith(QStringLiteral("我："));
            const QString sender = isSelf ? QStringLiteral("我") : QStringLiteral("用户");
            QString message = text;
            const int pos = text.indexOf(QStringLiteral("："));
            if (pos >= 0) {
                message = text.mid(pos + 1);
            }
            m_messages.append({sender, message, isSelf, false});
            continue;
        }

        const QJsonObject msgObj = value.toObject();
        if (msgObj.value("type").toString() == "system") {
            m_messages.append({QString(), msgObj.value("message").toString(), false, true});
            continue;
        }

        const QString senderKey = msgObj.value("sender").toString();
        const bool isSelf = (senderKey == "doctor");
        const QString sender = isSelf ? QStringLiteral("我") : QStringLiteral("用户");
        m_messages.append({sender, msgObj.value("message").toString(), isSelf, false});
    }

    rebuildMessages();
}

void HistoryDialog::loadHistoryFromDir(const QString &historyDir)
{
    m_historyDir = historyDir;
    m_messages.clear();
    if (m_sessionList) {
        m_sessionList->clear();
    }

    QDir dir(historyDir);
    if (!dir.exists()) {
        rebuildMessages();
        return;
    }

    dir.setNameFilters(QStringList() << QStringLiteral("doctor_chat_*.txt"));
    const QFileInfoList files = dir.entryInfoList(QDir::Files, QDir::Time);
    for (const QFileInfo &fileInfo : files) {
        QListWidgetItem *item = new QListWidgetItem(sessionTitleFromFile(fileInfo.absoluteFilePath()), m_sessionList);
        item->setData(Qt::UserRole, fileInfo.absoluteFilePath());
    }

    if (m_sessionList && m_sessionList->count() > 0) {
        m_sessionList->setCurrentRow(0);
        loadSessionFile(m_sessionList->item(0)->data(Qt::UserRole).toString());
    } else {
        rebuildMessages();
    }
}

void HistoryDialog::onSessionItemClicked(QListWidgetItem *item)
{
    if (!item) {
        return;
    }
    loadSessionFile(item->data(Qt::UserRole).toString());
}

QString HistoryDialog::sessionTitleFromFile(const QString &filePath) const
{
    const QFileInfo info(filePath);
    QString base = info.completeBaseName();
    base.remove(QStringLiteral("doctor_chat_"));

    QStringList parts = base.split('_');
    if (parts.size() >= 3) {
        const QString date = parts.takeFirst();
        const QString time = parts.takeFirst();
        const QString user = parts.join('_');
        return QStringLiteral("%1\n%2 %3").arg(user, date, time);
    }

    return info.fileName();
}

void HistoryDialog::loadSessionFile(const QString &filePath)
{
    m_messages.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        rebuildMessages();
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    while (!in.atEnd()) {
        const QString line = in.readLine();
        const QStringList parts = line.split('|');
        if (parts.size() < 4) {
            continue;
        }

        const QString role = parts[1];
        QString sender = parts[2];
        QString message = parts.mid(3).join("|");
        sender.replace("\\n", " ");
        sender.replace("\\\\", "\\");
        message.replace("\\n", "\n");
        message.replace("\\\\", "\\");

        if (role == QStringLiteral("system")) {
            m_messages.append({QString(), message, false, true});
        } else if (role == QStringLiteral("doctor")) {
            m_messages.append({sender.isEmpty() ? QStringLiteral("我") : sender, message, true, false});
        } else {
            m_messages.append({sender.isEmpty() ? QStringLiteral("用户") : sender, message, false, false});
        }
    }

    rebuildMessages();
}
