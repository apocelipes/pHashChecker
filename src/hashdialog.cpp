#include "hashdialog.h"

#include <QCryptographicHash>
#include <QDialogButtonBox>
#include <QFile>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

HashDialog::HashDialog(const QString &path, QWidget *parent)
    : QDialog(parent)
{
    setModal(true);
    QFile img{path};
    img.open(QIODevice::ReadOnly);
    auto data = img.readAll();

    auto table = new QTableWidget;
    table->setColumnCount(2);
    table->setRowCount(5);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setVisible(false);
    table->setShowGrid(false);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->setColumnWidth(1, fontMetrics().averageCharWidth() * 80);
    table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    QCryptographicHash::Algorithm hashAlgorithms[] = {
            QCryptographicHash::Md5,
            QCryptographicHash::Sha1,
            QCryptographicHash::Sha256,
            QCryptographicHash::Sha512,
    };

    const QString algorithmNames[] = {
            "MD5",
            "SHA-1",
            "SHA-256",
            "SHA-512",
    };

    table->setItem(0, 0, new QTableWidgetItem{tr("File Name:")});
    table->setItem(0, 1, new QTableWidgetItem{path});
    for (int i = 1; i < table->rowCount(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem{algorithmNames[i-1] + ":"});
        const auto hashText = QCryptographicHash::hash(data, hashAlgorithms[i-1]).toHex();
        table->setItem(i, 1, new QTableWidgetItem{QString{hashText}});
    }

    auto buttons = new QDialogButtonBox;
    buttons->addButton(QDialogButtonBox::Ok);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(table);
    mainLayout->addWidget(buttons);
    setLayout(mainLayout);
}
