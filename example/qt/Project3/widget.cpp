#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    // 修正：兼容Qt6的中文语音设置，同时兼容Qt5
    tts.setLocale(QLocale::Chinese);
    tts.setPitch(1);
    tts.setRate(0.2);
    tts.setVolume(0.5);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    if(ui->pushButton->text() == "开始"){
        tts.say(ui->textEdit->toPlainText());
        ui->pushButton->setText("结束");
    }else{
        tts.stop();
        ui->pushButton->setText("开始");
        ui->pushButton_2->setText("暂停");
    }
}

void Widget::on_pushButton_2_clicked()
{
    if(ui->pushButton_2->text() == "暂停"){
        tts.pause();
        ui->pushButton_2->setText("恢复");
    }else{
        tts.resume();
        ui->pushButton_2->setText("暂停");
    }
}

void Widget::on_doubleSpinBox_valueChanged(double arg1)
{
    tts.setPitch(arg1);
}

void Widget::on_doubleSpinBox_2_valueChanged(double arg1)
{
    tts.setRate(arg1);
}

void Widget::on_doubleSpinBox_3_valueChanged(double arg1)
{
    tts.setVolume(arg1);
}