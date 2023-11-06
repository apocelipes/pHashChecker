// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#include <array>

#include <QClipboard>
#include <QCryptographicHash>
#include <QDialogButtonBox>
#include <QFile>
#include <QGuiApplication>
#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>

#include "hashdialog.h"
#include "notificationbar.h"
#include "sizeformat.h"

namespace {
    constexpr int fileSizeIndex = 1;
    constexpr std::array<QCryptographicHash::Algorithm, 4> hashAlgorithms = {
            QCryptographicHash::Md5,
            QCryptographicHash::Sha1,
            QCryptographicHash::Sha256,
            QCryptographicHash::Sha512,
    };

    constexpr std::array<const char *, 4> algorithmNames = {
            "MD5",
            "SHA-1",
            "SHA-256",
            "SHA-512",
    };
}

HashDialog::HashDialog(const QString &path, QWidget *parent) noexcept
    : QDialog(parent)
{
    setModal(true);
    QFile img{path};
    img.open(QIODevice::ReadOnly);
    const auto &data = img.readAll();

    auto table = new QTableWidget;
    table->setColumnCount(2);
    table->setRowCount(2 + std::size(hashAlgorithms));
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setVisible(false);
    table->setShowGrid(false);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->setColumnWidth(1, fontMetrics().averageCharWidth() * 120);
    table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    table->setItem(0, 0, new QTableWidgetItem{tr("File Name:")});
    table->setItem(0, 1, new QTableWidgetItem{path});
    table->setItem(1, 0, new QTableWidgetItem{tr("File Size:")});

    const auto fileSize = img.size();
    table->setItem(1, 1, new QTableWidgetItem{Utils::sizeFormat(fileSize) + QString{" (%1)"}.arg(fileSize)});
    for (std::size_t i = 0; i < std::size(algorithmNames); ++i) {
        table->setItem(i+fileSizeIndex+1, 0, new QTableWidgetItem{algorithmNames[i] + QString{":"}});
        const QString &hashText = QCryptographicHash::hash(data, hashAlgorithms[i]).toHex();
        QCoreApplication::processEvents();
        table->setItem(i+fileSizeIndex+1, 1, new QTableWidgetItem{hashText});
    }

    auto buttons = new QDialogButtonBox;
    buttons->addButton(QDialogButtonBox::Ok);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto infoBar = NotificationBar::createNotificationBar(NotificationBar::NotificationType::INFO, "", this);
    connect(table, &QTableWidget::cellDoubleClicked, [table, infoBar](int row, int column) {
        if (row == fileSizeIndex) [[unlikely]] {
            return;
        }

        QGuiApplication::clipboard()->setText(table->item(row, column)->text());
        const auto &rowName = table->item(row, 0)->text();
        infoBar->setText(tr("%1 has been copied").arg(rowName));
        infoBar->setCloseButtonVisible(true);
        infoBar->showAndHide(3000);
    });

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(infoBar);
    mainLayout->addWidget(table);
    mainLayout->addWidget(buttons);
    setLayout(mainLayout);
    setWindowTitle(tr("Image Hash Dialog"));
}
