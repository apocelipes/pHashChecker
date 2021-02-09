#ifndef PHASHCHECKER_HASHDIALOG_H
#define PHASHCHECKER_HASHDIALOG_H

#include <QDialog>
#include <QString>

class HashDialog: public QDialog {
    Q_OBJECT
public:
    explicit HashDialog(const QString &path, QWidget *parent = nullptr);
};


#endif //PHASHCHECKER_HASHDIALOG_H
