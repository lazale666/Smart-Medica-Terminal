#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogwindow.h"
#include "formwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
    , dialogWindow(nullptr)
    , formWindow(nullptr)
    , windowOffset(50)
{
    ui->setupUi(this);

    connect(ui->btnOpenDialog, SIGNAL(clicked()), this, SLOT(openDialog()));
    connect(ui->btnOpenForm, SIGNAL(clicked()), this, SLOT(openForm()));
    connect(ui->btnSendToForm, SIGNAL(clicked()), this, SLOT(sendMessageToForm()));
}

MainWindow::~MainWindow()
{
    delete ui;
    if (dialogWindow) delete dialogWindow;
    if (formWindow) delete formWindow;
}

void MainWindow::openDialog()
{
    if (!dialogWindow) {
        dialogWindow = new DialogWindow(this);
        dialogWindow->move(x() + windowOffset, y() + windowOffset);
        connect(dialogWindow, SIGNAL(messageToMain(QString,QString)), this, SLOT(receiveMessage(QString,QString)));
        connect(this, SIGNAL(messageToDialog(QString)), dialogWindow, SLOT(receiveMessage(QString)));
    }
    dialogWindow->show();
    dialogWindow->raise();
    dialogWindow->activateWindow();
}

void MainWindow::openForm()
{
    if (!formWindow) {
        formWindow = new FormWindow(this);
        formWindow->move(x() + windowOffset * 2, y() + windowOffset * 2);
        connect(formWindow, SIGNAL(messageToMain(QString,QString)), this, SLOT(receiveMessage(QString,QString)));
        connect(this, SIGNAL(messageToForm(QString)), formWindow, SLOT(receiveMessage(QString)));
    }
    formWindow->show();
    formWindow->raise();
    formWindow->activateWindow();
}

void MainWindow::sendMessageToForm()
{
    QString message = ui->messageInput->text();
    if (!message.isEmpty()) {
        emit messageToForm(message);
        ui->messageInput->clear();
    }
}

void MainWindow::receiveMessage(const QString &sender, const QString &message)
{
    ui->messageLabel->setText(QString("收到%1的消息: %2").arg(sender, message));
}
