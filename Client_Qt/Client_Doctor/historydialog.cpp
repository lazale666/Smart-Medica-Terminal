#include "historydialog.h"
#include "ui_historydialog.h"
#include <QPushButton>

HistoryDialog::HistoryDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HistoryDialog)
{
    ui->setupUi(this);
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
        QTextBrowser {
            background: rgba(2, 9, 20, 0.86);
            border: 1px solid rgba(0, 229, 255, 0.35);
            border-radius: 18px;
            color: #EAFBFF;
            padding: 18px;
        }
    )");
    ui->headerWidget->setStyleSheet("background: rgba(4, 15, 31, 0.82); border: 1px solid rgba(0, 229, 255, 0.35); border-radius: 18px;");
    ui->buttonWidget->setStyleSheet("background: rgba(4, 15, 31, 0.82); border: 1px solid rgba(0, 229, 255, 0.35); border-radius: 18px;");
    ui->textBrowser->setStyleSheet("QTextBrowser { background: rgba(2, 9, 20, 0.86); border: 1px solid rgba(0, 229, 255, 0.35); border-radius: 18px; color: #EAFBFF; padding: 18px; }");
    ui->titleLabel->setText("用户历史对话记录");
    ui->closeBtn->setText("关闭");
    connect(ui->closeBtn, &QPushButton::clicked, this, &HistoryDialog::close);
}

HistoryDialog::~HistoryDialog()
{
    delete ui;
}

void HistoryDialog::setHistoryData(const QJsonArray &history)
{
    QString htmlContent = "<html><head><style>"
                         "body { font-family: 'Microsoft YaHei', sans-serif; font-size: 14px; line-height: 1.6; color: #eafbff; background: #020914; }"
                         ".user-msg { background-color: rgba(49,255,183,0.14); color: #eafbff; padding: 12px 16px; border-radius: 16px; margin: 10px 0; max-width: 80%; border: 1px solid rgba(49,255,183,0.35); }"
                         ".agent-msg { background-color: rgba(0,229,255,0.22); color: #eafbff; padding: 12px 16px; border-radius: 16px; margin: 10px 0; max-width: 80%; margin-left: auto; border: 1px solid rgba(0,229,255,0.45); }"
                         ".sender { font-weight: 600; color: #00e5ff; margin-bottom: 4px; }"
                         ".timestamp { font-size: 12px; color: #8bb9c8; margin-top: 4px; }"
                         "</style></head><body>";

    for (const QJsonValue &value : history) {
        if (value.isString()) {
            htmlContent += QString("<div class='user-msg'><div>%1</div></div>").arg(value.toString().toHtmlEscaped());
            continue;
        }

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
                              "</div>").arg(msgClass, senderText.toHtmlEscaped(), message.toHtmlEscaped(), timestamp.toHtmlEscaped());
    }

    htmlContent += "</body></html>";
    ui->textBrowser->setHtml(htmlContent);
}
