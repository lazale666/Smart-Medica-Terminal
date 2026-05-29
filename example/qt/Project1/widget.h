#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
// 新增：必须包含QImage和QLabel的头文件
#include <QImage>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    // Qt自动关联的槽函数，必须放在slots区域
private slots:
    void on_pushButton_clicked();

    // 自定义工具函数声明
private:
    QImage imgToCenter(QImage img, QLabel* lab);

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H