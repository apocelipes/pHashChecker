// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#pragma once

#include <QDialog>
#include <QDateTime>
#include <QLabel>

class TimerDialog: public QDialog {
    Q_OBJECT
public:
    TimerDialog(QWidget *parent = nullptr) noexcept;
    void start() noexcept;
    void stop() noexcept;
private:
    QDateTime startTime, endTime;
    QLabel *startLabel = nullptr;
    QLabel *endLabel = nullptr;
    QLabel *durationLabel = nullptr;
    bool running = false;
};
