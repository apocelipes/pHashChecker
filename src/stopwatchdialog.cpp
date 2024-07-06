// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#include <QDialogButtonBox>
#include  <QFormLayout>
#include <QVBoxLayout>
#include <QLocale>
#include <QStringBuilder>

#include "stopwatchdialog.h"

#define TIMERDIALOG_TIME_FORMAT QStringLiteral(u"yyyy-MM-dd HH:mm:ss.zzz ddd")

StopwatchDialog::StopwatchDialog(const QString &title, QWidget *parent) noexcept
    : QDialog(parent)
{
    startLabel = new  QLabel{this};
    endLabel = new  QLabel{this};
    durationLabel = new  QLabel{this};
    auto timerLayout  = new QFormLayout{};
    timerLayout->addRow(tr("Start:"), startLabel);
    timerLayout->addRow(tr("End:"), endLabel);
    timerLayout->addRow(tr("Cost:"), durationLabel);

    auto buttons = new QDialogButtonBox;
    buttons->addButton(QDialogButtonBox::Ok);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto mainLayout = new QVBoxLayout{};
    mainLayout->addLayout(timerLayout);
    mainLayout->addWidget(buttons);
    setLayout(mainLayout);
    setWindowTitle(title);
    setModal(true);
}

void StopwatchDialog::start() noexcept
{
    if (running) {
        qWarning() << tr("StopwatchDialog has already started");
        return;
    }
    endLabel->setText(QString{});
    durationLabel->setText(QString{});
    startTime = QDateTime::currentDateTime();
    startLabel->setText(QLocale().toString(startTime, TIMERDIALOG_TIME_FORMAT));
    running = true;
}

void StopwatchDialog::stop() noexcept
{
    if (!running) {
        qWarning() << tr("stop must be called after start()");
        return;
    }
    endTime = QDateTime::currentDateTime();
    endLabel->setText(QLocale().toString(endTime, TIMERDIALOG_TIME_FORMAT));
    const auto seconds = startTime.secsTo(endTime);
    durationLabel->setText(tr("%1 min %2.%3 seconds").arg(seconds / 60).arg(seconds % 60).arg(startTime.msecsTo(endTime) % 1000));
    running = false;
}
