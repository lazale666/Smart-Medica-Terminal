#include "historydialog.h"
#include "ui_historydialog.h"

HistoryDialog::HistoryDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HistoryDialog)
{
    ui->setupUi(this);
}

HistoryDialog::~HistoryDialog()
{
    delete ui;
}

void HistoryDialog::setHistoryData(const QJsonArray &history)
{
    QString htmlContent = "<html><head><style>"
                         "body { font-family: 'Microsoft YaHei', sans-serif; font-size: 14px; line-height: 1.6; color: #37474f; }"
                         ".user-msg { background-color: #f1f8e9; color: #333; padding: 12px 16px; border-radius: 16px; margin: 10px 0; max-width: 80%; border: 1px solid #ddd; }"
                         ".agent-msg { background-color: #e3f2fd; color: #333; padding: 12px 16px; border-radius: 16px; margin: 10px 0; max-width: 80%; margin-left: auto; border: 1px solid #b3d9f2; }"
                         ".sender { font-weight: 600; color: #00838f; margin-bottom: 4px; }"
                         ".timestamp { font-size: 12px; color: #78909c; margin-top: 4px; }"
                         "</style></head><body>";

    for (const QJsonValue &value : history) {
        QJsonObject msgObj = value.toObject();
        QString sender = msgObj.value("sender").toString();
        QString message = msgObj.value("message").toString();
        QString timestamp = msgObj.value("timestamp").toString();

        QString msgClass = (sender == "client") ? "user-msg" : "agent-msg";
        QString senderText = (sender == "client") ? "用户" : "智能体";

        htmlContent += QString("<div class='%1'>"
                              "<div class='sender'>%2</div>"
                              "<div>%3</div>"
                              "<div class='timestamp'>%4</div>"
                              "</div>").arg(msgClass).arg(senderText).arg(message).arg(timestamp);
    }

    htmlContent += "</body></html>";
    ui->textBrowser->setHtml(htmlContent);
}
