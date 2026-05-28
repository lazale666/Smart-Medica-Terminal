#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::reConnectInfo(int count)
{
    QString data("自动重连%1次，失败%2次");
    data = QString(data).arg(count).arg(count-1);
    ui->label->setText(data);
    if(count==10)
    {
        this->accept();
    }
}