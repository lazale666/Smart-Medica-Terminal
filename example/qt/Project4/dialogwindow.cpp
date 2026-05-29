#include "dialogwindow.h"
#include "ui_dialogwindow.h"
#include "mainwindow.h"

DialogWindow::DialogWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DialogWindow)
{
    ui->setupUi(this);

    connect(ui->btnSwitchToMain, SIGNAL(clicked()), this, SLOT(switchToMain()));
    connect(ui->btnSwitchToForm, SIGNAL(clicked()), this, SLOT(switchToForm()));
    connect(ui->btnSendToMain, SIGNAL(clicked()), this, SLOT(sendMessageToMain()));
    connect(ui->btnSendToForm, SIGNAL(clicked()), this, SLOT(sendMessageToForm()));
}

DialogWindow::~DialogWindow()
{
    delete ui;
}

void DialogWindow::switchToMain()
{
    hide();
    QWidget *parent = parentWidget();
    if (parent) {
        parent->show();
        parent->raise();
        parent->activateWindow();
    }
}

void DialogWindow::switchToForm()
{
    hide();
    MainWindow *mainWin = qobject_cast<MainWindow*>(parentWidget());
    if (mainWin) {
        mainWin->openForm();
    }
}

void DialogWindow::sendMessageToMain()
{
    QString message = ui->messageInput->text();
    if (!message.isEmpty()) {
        emit messageToMain("Dialog窗口", message);
        ui->messageInput->clear();
    }
}

void DialogWindow::sendMessageToForm()
{
    QString message = ui->messageInput->text();
    if (!message.isEmpty()) {
        emit messageToForm("Dialog窗口", message);
        ui->messageInput->clear();
    }
}

void DialogWindow::receiveMessage(const QString &message)
{
    ui->messageLabel->setText(QString("收到消息: %1").arg(message));
}
