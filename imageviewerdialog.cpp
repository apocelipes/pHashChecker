#include "imageviewerdialog.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QStackedWidget>
#include <QtGlobal>
#include <QVBoxLayout>

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
    connect(combox, qOverload<int>(&QComboBox::currentIndexChanged), stackview, &QStackedWidget::setCurrentIndex);
    auto buttons = new QDialogButtonBox{QDialogButtonBox::Ok, this};
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(combox, 0, Qt::AlignLeft);
    mainLayout->addWidget(stackview);
    mainLayout->addWidget(buttons);
    setLayout(mainLayout);
}
