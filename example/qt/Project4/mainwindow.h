#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class DialogWindow;
class FormWindow;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void messageToForm(const QString &message);
    void messageToDialog(const QString &message);

public slots:
    void receiveMessage(const QString &sender, const QString &message);
    void openDialog();
    void openForm();

private slots:
    void sendMessageToForm();

private:
    Ui::MainWindow *ui;

    DialogWindow *dialogWindow;
    FormWindow *formWindow;
    int windowOffset;
};

#endif // MAINWINDOW_H
