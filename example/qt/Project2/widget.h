#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void numClick();
    void opClick();
    void eqClick();
    void clearClick();
    void backClick();
    void signClick();

private:
    // 两个显示框
    QLineEdit *formulaLine;  // 上方：显示算式
    QLineEdit *resultLine;   // 下方：显示结果

    // 计算变量
    QString currentNum;
    QString lastNum;
    QString op;
    bool opClicked;
};

#endif // WIDGET_H