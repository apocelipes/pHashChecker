// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#include <QComboBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QStackedWidget>
#include <QtGlobal>
#include <QVBoxLayout>
#include <QPushButton>

#include "imageviewerdialog.h"
#include "imageviewer.h"
#include "utils.h"

ImageViewerDialog::ImageViewerDialog(const std::unordered_map<std::string, std::vector<std::string>> &sameImageList)
{
    auto stackView = new QStackedWidget{this};
    auto comboBox = new QComboBox{this};
    viewers.reserve(sameImageList.size());
    unsigned int index = 1u;
    for (const auto &[_, images] : sameImageList) {
        auto imageView = new ImageViewer{images, this};
        viewers.emplace_back(imageView);
        stackView->addWidget(imageView);
        comboBox->addItem(QString{"Group %1"}.arg(index++));
        connect(imageView, &ImageViewer::emptied, this, [this, imageView, stackView, comboBox](){
            stackView->removeWidget(imageView);
            auto targetIndex = Utils::indexOf(viewers.cbegin(), viewers.cend(), imageView);
            if (!targetIndex) {
                qWarning() << tr("target ImageViewer not found");
                return;
            }
            viewers.erase(viewers.begin() + *targetIndex);
            comboBox->removeItem(static_cast<int>(*targetIndex));
            imageView->deleteLater();
        });
        QCoreApplication::processEvents();
    }
    auto emptyWidget = new QLabel{tr("No data here!"), this};
    emptyWidget->setMinimumSize(900, 700);
    emptyWidget->setAlignment(Qt::AlignCenter);
    stackView->addWidget(emptyWidget);

    auto buttons = new QDialogButtonBox{this};
    auto prevBtn = new QPushButton{style()->standardIcon(QStyle::SP_ArrowLeft), tr("prev")};
    connect(prevBtn, &QPushButton::clicked, [comboBox](){
        auto index = comboBox->currentIndex();
        comboBox->setCurrentIndex(index - 1);
    });
    buttons->addButton(prevBtn, QDialogButtonBox::ActionRole);

    auto ignoreBtn = new QPushButton{style()->standardIcon(QStyle::SP_BrowserStop), tr("ignore this")};
    connect(ignoreBtn, &QPushButton::clicked, this, [this, comboBox, stackView](){
        auto widget = stackView->currentWidget();
        auto index = comboBox->currentIndex();
        viewers.erase(viewers.begin() + index);
        comboBox->removeItem(index);
        stackView->removeWidget(widget);
        widget->deleteLater();
    });
    buttons->addButton(ignoreBtn, QDialogButtonBox::ActionRole);

    auto nextBtn = new QPushButton{style()->standardIcon(QStyle::SP_ArrowRight), tr("next")};
    connect(nextBtn, &QPushButton::clicked, comboBox, [comboBox](){
        auto index = comboBox->currentIndex();
        comboBox->setCurrentIndex(index + 1);
    });
    buttons->addButton(nextBtn, QDialogButtonBox::ActionRole);
    buttons->addButton(QDialogButtonBox::Ok);

    auto buttonsSetEnable = [this, stackView, prevBtn, nextBtn, ignoreBtn](){
        auto index = stackView->currentIndex();
        // 因为有空白组件做default，不能靠currentIndex == -1判断
        auto hasViewer = !viewers.empty();
        prevBtn->setEnabled(hasViewer && index != 0);
        nextBtn->setEnabled(hasViewer
                            && static_cast<unsigned int>(index) != (viewers.size() - 1));
        ignoreBtn->setEnabled(hasViewer);
    };
    connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [stackView, buttonsSetEnable](int index){
        stackView->setCurrentIndex(index);
        buttonsSetEnable();
    });
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    buttonsSetEnable();

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(comboBox, 0, Qt::AlignLeft);
    mainLayout->addWidget(stackView);
    mainLayout->addWidget(buttons);
    setLayout(mainLayout);
    setWindowTitle(tr("Check Images"));
}
