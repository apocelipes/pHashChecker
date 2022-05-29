// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2022 apocelipes

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
    constexpr int hashStartIndex = 2;
    QCryptographicHash::Algorithm hashAlgorithms[] = {
            QCryptographicHash::Md5,
            QCryptographicHash::Sha1,
            QCryptographicHash::Sha256,
            QCryptographicHash::Sha512,
    };

    const char *algorithmNames[] = {
            "MD5",
            "SHA-1",
            "SHA-256",
            "SHA-512",
    };
}

HashDialog::HashDialog(const QString &path, QWidget *parent)
    : QDialog(parent)
{
    setModal(true);
    QFile img{path};
    img.open(QIODevice::ReadOnly);
    auto data = img.readAll();

    auto table = new QTableWidget;
    table->setColumnCount(2);
    table->setRowCount(hashStartIndex + std::size(hashAlgorithms));
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setVisible(false);
    table->setShowGrid(false);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->setColumnWidth(1, fontMetrics().averageCharWidth() * 80);
    table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    table->setItem(0, 0, new QTableWidgetItem{tr("File Name:")});
    table->setItem(0, 1, new QTableWidgetItem{path});
    table->setItem(1, 0, new QTableWidgetItem{tr("File Size:")});

    const auto fileSize = img.size();
    table->setItem(1, 1, new QTableWidgetItem{Utils::sizeFormat(fileSize) + QString::asprintf(" (%lld)", fileSize)});
    for (int i = hashStartIndex; i < table->rowCount(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem{algorithmNames[i-hashStartIndex] + QString{":"}});
        const auto hashText = QCryptographicHash::hash(data, hashAlgorithms[i-hashStartIndex]).toHex();
        table->setItem(i, 1, new QTableWidgetItem{QString{hashText}});
    }

    auto buttons = new QDialogButtonBox;
    buttons->addButton(QDialogButtonBox::Ok);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto infoBar = NotificationBar::createNotificationBar(NotificationBar::NotificationType::INFO, "", this);
    connect(table, &QTableWidget::cellDoubleClicked, [table, infoBar](int row, int column) {
        if (row < hashStartIndex || column != 1) {
            return;
        }

        QGuiApplication::clipboard()->setText(table->item(row, column)->text());
        auto hashName = table->item(row, 0)->text();
        infoBar->setText(tr("%1 has been copied").arg(hashName));
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
