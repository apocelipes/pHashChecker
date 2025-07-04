// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 apocelipes

#pragma once

#include <QDialog>
#include <QString>

class AboutDialog: public QDialog {
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = nullptr) noexcept;
private Q_SLOTS:
    void copyVersionsToClipboard() noexcept;
};
