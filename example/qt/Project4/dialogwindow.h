#ifndef DIALOGWINDOW_H
#define DIALOGWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class DialogWindow; }
QT_END_NAMESPACE

class MainWindow;
class FormWindow;

class DialogWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DialogWindow(QWidget *parent = nullptr);
    ~DialogWindow();

signals:
    void messageToMain(const QString &sender, const QString &message);
    void messageToForm(const QString &sender, const QString &message);

public slots:
    void receiveMessage(const QString &message);

private slots:
    void switchToMain();
    void switchToForm();
    void sendMessageToMain();
    void sendMessageToForm();

private:
    Ui::DialogWindow *ui;

    MainWindow *mainWindow;
    FormWindow *formWindow;
};

#endif // DIALOGWINDOW_H
