#include "dialog.h"
#include "ui_dialog.h"
#include "themehelpers.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    applyAppearance("#07111F", "#D8F7FF");
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::applyAppearance(const QString &bgColor, const QString &fontColor)
{
    const QString normalizedBg = ThemeHelpers::normalizeBgColor(bgColor);
    const QString normalizedFont = fontColor.isEmpty() ? ThemeHelpers::defaultFontColorForBg(normalizedBg) : fontColor;
    const bool light = ThemeHelpers::isLightTheme(normalizedBg);

    setStyleSheet(QString(
        "QDialog#Dialog { background: %1; border: 1px solid %2; border-radius: 14px; }"
        "QLabel { color: %3; font: 700 14px \"Microsoft YaHei\"; }")
                      .arg(light
                               ? "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F5FBFF, stop:0.55 #E9F6FF, stop:1 #DCEEFF)"
                               : "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111F, stop:0.55 #071B2F, stop:1 #0B1023)",
                           light ? "rgba(15,39,64,0.16)" : "rgba(0,229,255,0.24)",
                           normalizedFont));
}

void Dialog::reConnectInfo(int count)
{
    ui->label->setText(QStringLiteral("自动重连 %1 次，失败 %2 次").arg(count).arg(count - 1));
    if (count == 10) {
        accept();
    }
}
