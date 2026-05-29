#include "formwindow.h"
#include "ui_formwindow.h"
#include "mainwindow.h"

FormWindow::FormWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormWindow)
{
    ui->setupUi(this);

    connect(ui->btnSwitchToMain, SIGNAL(clicked()), this, SLOT(switchToMain()));
    connect(ui->btnSwitchToDialog, SIGNAL(clicked()), this, SLOT(switchToDialog()));
    connect(ui->btnSendToMain, SIGNAL(clicked()), this, SLOT(sendMessageToMain()));
    connect(ui->btnSendToDialog, SIGNAL(clicked()), this, SLOT(sendMessageToDialog()));
}

FormWindow::~FormWindow()
{
    delete ui;
}

void FormWindow::switchToMain()
{
    hide();
    QWidget *parent = parentWidget();
    if (parent) {
        parent->show();
        parent->raise();
        parent->activateWindow();
    }
}

void FormWindow::switchToDialog()
{
    hide();
    MainWindow *mainWin = qobject_cast<MainWindow*>(parentWidget());
    if (mainWin) {
        mainWin->openDialog();
    }
}

void FormWindow::sendMessageToMain()
{
    QString message = ui->messageInput->text();
    if (!message.isEmpty()) {
        emit messageToMain("Form窗口", message);
        ui->messageInput->clear();
    }
}

void FormWindow::sendMessageToDialog()
{
    QString message = ui->messageInput->text();
    if (!message.isEmpty()) {
        emit messageToDialog("Form窗口", message);
        ui->messageInput->clear();
    }
}

void FormWindow::receiveMessage(const QString &message)
{
    ui->messageLabel->setText(QString("收到消息: %1").arg(message));
}
