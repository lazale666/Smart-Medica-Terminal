#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

QImage Widget::imgToCenter(QImage img,QLabel*lab)
{
    QSize imgsize = img.size();
    QSize labsize = lab->size();

    double wigthRatet = (1.0*imgsize.width())/labsize.width();
    double heightRate = (1.0*imgsize.height())/labsize.height();

    if(wigthRatet>heightRate)
    {
    return img.scaledToWidth(labsize.width());
    }else{
    return img.scaledToHeight(labsize.height());
    }
}

void Widget::on_pushButton_clicked()
{
    QImage img("D:/All Program/QT_DATA/Project1/pic/photo.png");
    img = imgToCenter(img,ui->label);
    ui->label->setPixmap(QPixmap::fromImage(img));
}