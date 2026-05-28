#ifndef CHATMESSAGEWIDGETS_H
#define CHATMESSAGEWIDGETS_H

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QSpacerItem>
#include <QTimer>

struct ChatThemePalette
{
    QString pageBackground;
    QString panelBackground;
    QString panelBorder;
    QString mainText;
    QString mutedText;
    QString titleText;
    QString selfNameText;
    QString peerNameText;
    QString selfBubbleBackground;
    QString selfBubbleBorder;
    QString peerBubbleBackground;
    QString peerBubbleBorder;
    QString bubbleText;
    QString systemBackground;
    QString systemBorder;
    QString systemText;
    QString buttonBackground;
    QString buttonHoverBackground;
    QString buttonText;
    QString inputBackground;
    QString inputBorder;
};

inline ChatThemePalette buildChatThemePalette(bool light)
{
    if (light) {
        return {
            "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F5FBFF, stop:0.55 #E9F6FF, stop:1 #DCEEFF)",
            "rgba(255, 255, 255, 0.90)",
            "rgba(15, 39, 64, 0.14)",
            "#0F2740",
            "#4C647A",
            "#0F2740",
            "#0F78B7",
            "#157A52",
            "rgba(127, 217, 255, 0.38)",
            "rgba(15, 120, 183, 0.30)",
            "rgba(196, 240, 214, 0.70)",
            "rgba(21, 122, 82, 0.24)",
            "#0F2740",
            "rgba(15, 39, 64, 0.08)",
            "rgba(15, 39, 64, 0.16)",
            "#4C647A",
            "rgba(255, 255, 255, 0.96)",
            "rgba(199, 244, 255, 0.96)",
            "#0F2740",
            "rgba(255, 255, 255, 0.96)",
            "rgba(15, 39, 64, 0.18)"
        };
    }

    return {
        "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #04111F, stop:0.55 #071B2F, stop:1 #0B1023)",
        "rgba(4, 15, 31, 0.82)",
        "rgba(0, 229, 255, 0.35)",
        "#D8F7FF",
        "#8BB9C8",
        "#00E5FF",
        "#8BD9FF",
        "#31FFB7",
        "rgba(0, 229, 255, 0.18)",
        "rgba(0, 229, 255, 0.45)",
        "rgba(49, 255, 183, 0.14)",
        "rgba(49, 255, 183, 0.35)",
        "#EAFBFF",
        "rgba(139, 185, 200, 0.14)",
        "rgba(139, 185, 200, 0.28)",
        "#8BB9C8",
        "rgba(6, 24, 45, 0.92)",
        "rgba(0, 229, 255, 0.18)",
        "#D8F7FF",
        "rgba(2, 9, 20, 0.88)",
        "rgba(0, 229, 255, 0.55)"
    };
}

inline QWidget *createSystemMessageWidget(const QString &message, const ChatThemePalette &palette, QWidget *parent = nullptr)
{
    QWidget *wrapper = new QWidget(parent);
    QHBoxLayout *row = new QHBoxLayout(wrapper);
    row->setContentsMargins(0, 8, 0, 8);
    row->addStretch();

    QLabel *label = new QLabel(message, wrapper);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignCenter);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setStyleSheet(QString(
        "QLabel { background: %1; border: 1px solid %2; border-radius: 14px; color: %3; padding: 6px 14px; font-size: 12px; }")
                             .arg(palette.systemBackground, palette.systemBorder, palette.systemText));
    row->addWidget(label, 0, Qt::AlignCenter);
    row->addStretch();
    return wrapper;
}

inline QWidget *createChatMessageWidget(const QString &sender,
                                        const QString &message,
                                        bool isSelf,
                                        const ChatThemePalette &palette,
                                        int maxBubbleWidth,
                                        QWidget *parent = nullptr)
{
    QWidget *wrapper = new QWidget(parent);
    QHBoxLayout *row = new QHBoxLayout(wrapper);
    row->setContentsMargins(0, 10, 0, 10);
    row->setSpacing(12);

    QWidget *bubbleContainer = new QWidget(wrapper);
    QVBoxLayout *bubbleLayout = new QVBoxLayout(bubbleContainer);
    bubbleLayout->setContentsMargins(0, 0, 0, 0);
    bubbleLayout->setSpacing(6);

    QLabel *nameLabel = new QLabel(sender, bubbleContainer);
    nameLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 12px; font-weight: 700; }")
                                 .arg(isSelf ? palette.selfNameText : palette.peerNameText));

    QFrame *bubble = new QFrame(bubbleContainer);
    bubble->setObjectName("messageBubble");
    bubble->setMaximumWidth(maxBubbleWidth);
    bubble->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    bubble->setStyleSheet(QString(
        "QFrame#messageBubble { background: %1; border: 1px solid %2; border-radius: 18px; }")
                              .arg(isSelf ? palette.selfBubbleBackground : palette.peerBubbleBackground,
                                   isSelf ? palette.selfBubbleBorder : palette.peerBubbleBorder));

    QVBoxLayout *messageLayout = new QVBoxLayout(bubble);
    messageLayout->setContentsMargins(14, 12, 14, 12);
    messageLayout->setSpacing(0);

    QLabel *messageLabel = new QLabel(message, bubble);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    messageLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 14px; line-height: 1.5; }").arg(palette.bubbleText));

    messageLayout->addWidget(messageLabel);
    bubbleLayout->addWidget(nameLabel, 0, isSelf ? Qt::AlignRight : Qt::AlignLeft);
    bubbleLayout->addWidget(bubble, 0, isSelf ? Qt::AlignRight : Qt::AlignLeft);

    if (isSelf) {
        row->addStretch();
        row->addWidget(bubbleContainer, 0, Qt::AlignRight | Qt::AlignTop);
    } else {
        row->addWidget(bubbleContainer, 0, Qt::AlignLeft | Qt::AlignTop);
        row->addStretch();
    }

    return wrapper;
}

inline void clearLayoutWidgets(QLayout *layout)
{
    if (!layout) {
        return;
    }

    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        if (QLayout *childLayout = item->layout()) {
            clearLayoutWidgets(childLayout);
            delete childLayout;
        }
        delete item;
    }
}

inline void scrollAreaToBottom(QScrollArea *scrollArea)
{
    if (!scrollArea) {
        return;
    }
    const auto scrollNow = [scrollArea]() {
        if (QScrollBar *bar = scrollArea->verticalScrollBar()) {
            bar->setValue(bar->maximum());
        }
    };

    scrollNow();
    QTimer::singleShot(0, scrollArea, scrollNow);
}

inline bool isScrollAreaNearBottom(const QScrollArea *scrollArea, int tolerance = 12)
{
    if (!scrollArea) {
        return true;
    }
    const QScrollBar *bar = scrollArea->verticalScrollBar();
    if (!bar) {
        return true;
    }
    return (bar->maximum() - bar->value()) <= tolerance;
}

#endif // CHATMESSAGEWIDGETS_H
