#ifndef PHASHCHECKER_NOTIFICATIONBAR_H
#define PHASHCHECKER_NOTIFICATIONBAR_H

#include <QFrame>
#include <QColor>
#include <QString>

class QLabel;
class QPushButton;
class QGraphicsOpacityEffect;
class QPropertyAnimation;

class NotificationBar: public QFrame {
    Q_OBJECT
public:
    NotificationBar(const QColor &borderColor, const QColor &bgColor, QWidget *parent = nullptr);

public Q_SLOTS:
    void setColor(const QColor &borColor, const QColor &bgColor)
    {
        if (!borColor.isValid() || !bgColor.isValid()) {
            return;
        }

        auto borderColorStyle = QString::asprintf("rgba(%d,%d,%d,%d)",
                                                  borColor.red(),
                                                  borColor.green(),
                                                  borColor.blue(),
                                                  borColor.alpha());
        auto bgColorStyle = QString::asprintf("rgba(%d,%d,%d,%d)",
                                              bgColor.red(),
                                              bgColor.green(),
                                              bgColor.blue(),
                                              bgColor.alpha());
        setStyleSheet(".NotificationBar{border: 1px solid " +
                      borderColorStyle +
                      "; background-color: " +
                      bgColorStyle + ";}");
    }

    void setCloseButtonVisible(bool visible);
    void setIcon(const QIcon &notifyIcon);
    void setText(const QString &text);

    void animatedShow();
    void animatedHide();
    void showAndHide();

    static NotificationBar *createInformationBar(QWidget *parent = nullptr);
    static NotificationBar *createErrorBar(QWidget *parent = nullptr);

private:
    QLabel *iconLabel = nullptr;
    QLabel *textLabel = nullptr;
    QPushButton *closeBtn = nullptr;
    QGraphicsOpacityEffect *effect = nullptr;
};

#endif //PHASHCHECKER_NOTIFICATIONBAR_H
