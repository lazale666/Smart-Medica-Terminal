#ifndef FACERECOGNIZEWIDGET_H
#define FACERECOGNIZEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>

QT_BEGIN_NAMESPACE
namespace Ui { class FaceRecognizeWidget; }
QT_END_NAMESPACE

class FaceRecognizeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FaceRecognizeWidget(const QString &username, QWidget *parent = nullptr);
    ~FaceRecognizeWidget();
    void applyAppearance(const QString &mode, const QString &bgColor, const QString &fontColor);

signals:
    void recognitionSuccess();
    void backToRecharge();

private slots:
    void onTimerFirstTimeout();
    void onShowImageTimeout();
    void onBackBtnClicked();

private:
    Ui::FaceRecognizeWidget *ui;
    QString m_username;
    QTimer *m_firstTimer;
    QTimer *m_showImageTimer;
    QTimer *m_successTimer;
    QString m_currentMode;
    QString m_bgColor;
    QString m_fontColor;
    
    void setupUI();
    void showFaceImage();
    void showSuccessMessage();
};

#endif // FACERECOGNIZEWIDGET_H
