#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

    // 槽函数：更新重连信息
public slots:
    void reConnectInfo(int count);

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H