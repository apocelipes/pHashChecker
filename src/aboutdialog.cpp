// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2025 apocelipes

#include <QApplication>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "aboutdialog.h"
#include "utils/versions.h"

namespace {
    inline QLabel *createOpenableLabel(const QString &text, QWidget *parent = nullptr) noexcept
    {
        auto label = new QLabel{text, parent};
        label->setOpenExternalLinks(true);
        return label;
    }
}

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
        createOpenableLabel("<a href='https://github.com/apocelipes/pHashChecker'>pHashChecker</a>:"),
        new QLabel{Utils::getPHashCheckerVersion()}
    );
    libsForm->addRow(
        createOpenableLabel("<a href='https://www.qt.io/'>Qt</a>:"),
        new QLabel{Utils::getQtVersion()}
    );
    libsForm->addRow(
        createOpenableLabel("<a href='https://github.com/GreycLab/CImg'>CImg</a>:"),
        new QLabel{Utils::getCImgVersion()}
    );
    libsForm->addRow(
        createOpenableLabel("<a href='https://github.com/apocelipes/pHash'>pHash</a>:"),
        new QLabel{Utils::getPHashVersion()}
    );
    libsForm->addRow(
        createOpenableLabel("<a href='https://github.com/Morwenn/cpp-sort'>cpp-sort</a>:"),
        new QLabel{Utils::getCppSortVersion()}
    );
    libsForm->addRow(
        createOpenableLabel("<a href='https://github.com/martinus/unordered_dense'>unordered_dense</a>:"),
        new QLabel{Utils::getAnkerlUnorderedDenseVersion()}
    );

    auto aboutQtBtn = new QPushButton{tr("About Qt"), this};
    connect(aboutQtBtn, &QPushButton::clicked, this, [](){
        QApplication::aboutQt();
    });
    auto buttons = new QDialogButtonBox;
    buttons->addButton(QDialogButtonBox::Ok);
    buttons->addButton(aboutQtBtn, QDialogButtonBox::ActionRole);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto copyRightLabel = new QLabel{QStringLiteral(u"© 2021-2025 apocelipes, under GPLv3"), this};

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
