#ifndef EDITABLEIMAGE_H
#define EDITABLEIMAGE_H

#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QString>

class EditableImage : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QString imagePath READ getImagePath WRITE setImagePath NOTIFY pathChanged);
public:
    EditableImage(const QString &imgPath, QWidget *parent = nullptr);

    QString getImagePath()
    {
        return m_path;
    }

    void setImagePath(const QString &path)
    {
        QPixmap newImg{path};
        setPixmap(newImg);
        m_path = path;
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
    void TrashMoved();
    void dataCopied(const QPixmap &img);
    void pathCopied(const QString &imgPath);
    void pathChanged(const QString &imgPath);
private slots:
    void showContextMenu(const QPoint &pos);
private:
    QString m_path;
};

#endif // EDITABLEIMAGE_H
