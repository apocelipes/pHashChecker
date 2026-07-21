// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 apocelipes

#include <QApplication>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStringBuilder>
#include <QStyle>
#include <QVBoxLayout>

#include "aboutdialog.h"
#include "utils/widgethelpers.h"
#include "utils/versions.h"

AboutDialog::AboutDialog(QWidget *parent) noexcept
    : QDialog(parent)
{
    setModal(true);

    auto titleLabel = new QLabel{"<h2><b>pHashChecker</b></h2>", this};
    auto contentLabel = new QLabel{tr("pHashChecker is a tool written in Qt6 and modern C++ for finding similar images."), this};
    auto libLabel = new QLabel{tr("<br>Libraries used by this software:"), this};

    auto libsBegin = new QFrame{this};
    libsBegin->setFrameShape(QFrame::HLine);
    auto libsEnd = new QFrame{this};
    libsEnd->setFrameShape(QFrame::HLine);

    auto libsForm = new QFormLayout;
    libsForm->setRowWrapPolicy(QFormLayout::DontWrapRows);
    libsForm->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    libsForm->setFormAlignment(Qt::AlignCenter | Qt::AlignTop);
    libsForm->setLabelAlignment(Qt::AlignRight);
    libsForm->addRow(
        Utils::createOpenableLabel("<a href='https://github.com/apocelipes/pHashChecker'>pHashChecker</a>:"),
        new QLabel{Utils::getPHashCheckerVersion()}
    );
    libsForm->addRow(
        Utils::createOpenableLabel("<a href='https://www.qt.io/'>Qt</a>:"),
        new QLabel{Utils::getQtVersion()}
    );
    libsForm->addRow(
        Utils::createOpenableLabel("<a href='https://github.com/GreycLab/CImg'>CImg</a>:"),
        new QLabel{Utils::getCImgVersion()}
    );
    libsForm->addRow(
        Utils::createOpenableLabel("<a href='https://github.com/apocelipes/pHash'>pHash</a>:"),
        new QLabel{Utils::getPHashVersion()}
    );
    libsForm->addRow(
        Utils::createOpenableLabel("<a href='https://github.com/Morwenn/cpp-sort'>cpp-sort</a>:"),
        new QLabel{Utils::getCppSortVersion()}
    );
    libsForm->addRow(
        Utils::createOpenableLabel("<a href='https://github.com/martinus/unordered_dense'>unordered_dense</a>:"),
        new QLabel{Utils::getAnkerlUnorderedDenseVersion()}
    );
    libsForm->addRow(
        Utils::createOpenableLabel("<a href='https://github.com/kimwalisch/libpopcnt'>libpopcnt</a>:"),
        new QLabel{Utils::getLibpopcntVersion()}
    );
    libsForm->addRow(
        Utils::createOpenableLabel("<a href='https://github.com/pinam45/dynamic_bitset'>dynamic_bitset</a>:"),
        new QLabel{Utils::getSulDynamicBitsetVersion()}
    );

    auto aboutQtBtn = new QPushButton{tr("About Qt"), this};
    connect(aboutQtBtn, &QPushButton::clicked, this, []() noexcept {
        QApplication::aboutQt();
    });
    auto copyBtn = new QPushButton{tr("Copy"), this};
    copyBtn->setToolTip(tr("Copy version information to your clipboard."));
    connect(copyBtn, &QPushButton::clicked, this, &AboutDialog::copyVersionsToClipboard);
    auto buttons = new QDialogButtonBox;
    const auto buttonsStyle = static_cast<QDialogButtonBox::ButtonLayout>(buttons->style()->
        styleHint(QStyle::SH_DialogButtonLayout, nullptr, buttons));
    if (buttonsStyle == QDialogButtonBox::ButtonLayout::KdeLayout) {
        buttons->addButton(copyBtn, QDialogButtonBox::AcceptRole);
        buttons->addButton(QDialogButtonBox::Ok);
    } else {
        buttons->addButton(QDialogButtonBox::Ok);
        buttons->addButton(copyBtn, QDialogButtonBox::AcceptRole);
    }
    buttons->addButton(aboutQtBtn, QDialogButtonBox::ActionRole);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto copyRightLabel = new QLabel{QStringLiteral(u"© 2021-2026 apocelipes, under GPLv3"), this};

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(contentLabel);
    mainLayout->addWidget(libLabel);
    mainLayout->addWidget(libsBegin);
    mainLayout->addLayout(libsForm);
    mainLayout->addWidget(libsEnd);
    mainLayout->addWidget(copyRightLabel);
    mainLayout->addStretch(1);
    auto btnLineLayout = new QHBoxLayout;
    btnLineLayout->addStretch(1);
    btnLineLayout->addWidget(buttons);
    mainLayout->addLayout(btnLineLayout);
    setLayout(mainLayout);
    setWindowTitle(tr("About pHashChecker"));
}

void AboutDialog::copyVersionsToClipboard() noexcept
{
    QString result = QStringLiteral(u"pHashChecker: ") % Utils::getPHashCheckerVersion() % '\n';
    result += QStringLiteral(u"Qt: ") % Utils::getQtVersion() % '\n';
    result += QStringLiteral(u"CImg: ") % Utils::getCImgVersion() % '\n';
    result += QStringLiteral(u"pHash: ") % Utils::getPHashVersion() % '\n';
    result += QStringLiteral(u"cpp-sort: ") % Utils::getCppSortVersion() % '\n';
    result += QStringLiteral(u"unordered_dense: ") % Utils::getAnkerlUnorderedDenseVersion() % '\n';
    result += QStringLiteral(u"libpopcnt: ") % Utils::getLibpopcntVersion() % '\n';
    result += QStringLiteral(u"dynamic_bitset: ") % Utils::getSulDynamicBitsetVersion();
    QGuiApplication::clipboard()->setText(result);
}
