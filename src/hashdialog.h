// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2021 apocelipes

#ifndef PHASHCHECKER_HASHDIALOG_H
#define PHASHCHECKER_HASHDIALOG_H

#include <QDialog>
#include <QString>

class HashDialog: public QDialog {
    Q_OBJECT
public:
    explicit HashDialog(const QString &path, QWidget *parent = nullptr) noexcept;
};


#endif //PHASHCHECKER_HASHDIALOG_H
