#include "imageviewerdialog.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QStackedWidget>
#include <QtGlobal>
#include <QVBoxLayout>
#include <QPushButton>

#include "imageviewer.h"
#include "utils.h"

ImageViewerDialog::ImageViewerDialog(const std::vector<std::vector<std::string>> &sameImageList)
{
    auto stackview = new QStackedWidget{this};
    auto combox = new QComboBox{this};
    viewers.reserve(sameImageList.size());
    unsigned int index = 1u;
    for (const auto &images : sameImageList) {
        auto imageView = new ImageViewer{images, this};
        viewers.emplace_back(imageView);
        stackview->addWidget(imageView);
        combox->addItem(QString::asprintf("Group %u", index++));
        connect(imageView, &ImageViewer::emptied, [this, imageView, stackview, combox](){
            stackview->removeWidget(imageView);
            auto targetIndex = indexOf(viewers.cbegin(), viewers.cend(), imageView);
            viewers.erase(viewers.begin() + targetIndex);
            combox->removeItem(targetIndex);
            imageView->deleteLater();
        });
        QCoreApplication::processEvents();
    }

    auto buttons = new QDialogButtonBox{this};
    auto prevBtn = new QPushButton{style()->standardIcon(QStyle::SP_ArrowLeft), tr("prev")};
    connect(prevBtn, &QPushButton::clicked, [combox](){
        auto index = combox->currentIndex();
        combox->setCurrentIndex(index - 1);
    });
    buttons->addButton(prevBtn, QDialogButtonBox::ActionRole);

    auto ignoreBtn = new QPushButton{style()->standardIcon(QStyle::SP_BrowserStop), tr("ignore this")};
    connect(ignoreBtn, &QPushButton::clicked, [this, combox, stackview](){
        auto widget = stackview->currentWidget();
        auto index = combox->currentIndex();
        viewers.erase(viewers.begin() + index);
        combox->removeItem(index);
        stackview->removeWidget(widget);
        widget->deleteLater();
    });
    buttons->addButton(ignoreBtn, QDialogButtonBox::ActionRole);

    auto nextBtn = new QPushButton{style()->standardIcon(QStyle::SP_ArrowRight), tr("next")};
    connect(nextBtn, &QPushButton::clicked, [combox](){
        auto index = combox->currentIndex();
        combox->setCurrentIndex(index + 1);
    });
    buttons->addButton(nextBtn, QDialogButtonBox::ActionRole);
    buttons->addButton(QDialogButtonBox::Ok);

    connect(combox, qOverload<int>(&QComboBox::currentIndexChanged), [this, stackview, prevBtn, nextBtn](int index){
        prevBtn->setEnabled(index != 0);
        nextBtn->setEnabled(static_cast<unsigned int>(index) != (viewers.size() - 1));
        stackview->setCurrentIndex(index);
    });
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(combox, 0, Qt::AlignLeft);
    mainLayout->addWidget(stackview);
    mainLayout->addWidget(buttons);
    setLayout(mainLayout);
}
