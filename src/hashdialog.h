// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2021 apocelipes

#pragma once

#include <QDialog>
#include <QString>

class HashDialog: public QDialog {
    Q_OBJECT
public:
    explicit HashDialog(const QString &path, QWidget *parent = nullptr) noexcept;
};
