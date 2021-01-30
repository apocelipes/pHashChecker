#ifndef EDITABLEIMAGE_H
#define EDITABLEIMAGE_H

#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QString>

#include <filesystem>

class EditableImage : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QString imagePath READ getImagePath WRITE setImagePath NOTIFY pathChanged);
public:
    explicit EditableImage(const QString &imgPath, QWidget *parent = nullptr);

    QString getImagePath()
    {
        return m_path;
    }

    bool isEmpty()
    {
        return getImagePath() == "";
    }

    void setImagePath(const QString &path)
    {
        if (path == m_path) {
            return;
        }
        if (!std::filesystem::exists(path.toStdString())) {
            clear();
            setToolTip(isEmpty() ? tr("There's no image here") : getImagePath());
            return;
        }
        QPixmap newImg{path};
        setPixmap(newImg);
        m_path = path;
        setToolTip(path);
        Q_EMIT pathChanged(path);
    }

    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        Q_EMIT doubleClicked();
        event->accept();
    }
signals:
    void doubleClicked();
    void deleted();
    void trashMoved();
    void dataCopied(const QPixmap &img);
    void pathCopied(const QString &imgPath);
    void pathChanged(const QString &imgPath);
private slots:
    void showContextMenu(const QPoint &pos);
private:
    QString m_path;
};

#endif // EDITABLEIMAGE_H
