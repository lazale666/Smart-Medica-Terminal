#ifndef FORMWINDOW_H
#define FORMWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class FormWindow; }
QT_END_NAMESPACE

class MainWindow;
class DialogWindow;

class FormWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FormWindow(QWidget *parent = nullptr);
    ~FormWindow();

signals:
    void messageToMain(const QString &sender, const QString &message);
    void messageToDialog(const QString &sender, const QString &message);

public slots:
    void receiveMessage(const QString &message);

private slots:
    void switchToMain();
    void switchToDialog();
    void sendMessageToMain();
    void sendMessageToDialog();

private:
    Ui::FormWindow *ui;

    MainWindow *mainWindow;
    DialogWindow *dialogWindow;
};

#endif // FORMWINDOW_H
