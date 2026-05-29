#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    // 窗口关闭时自动销毁，避免内存泄漏
    setAttribute(Qt::WA_DeleteOnClose, false);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::reConnectInfo(int count)
{
    // 修复arg顺序错误
    QString data = QString("自动重连%1次，失败%2次")
                       .arg(count)
                       .arg(count);
    ui->label->setText(data);

    // 重连10次后自动关闭弹窗
    if(count >= 10)
    {
        accept();
    }
}