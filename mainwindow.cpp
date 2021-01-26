#include "mainwindow.h"
#include <QVBoxLayout>
#include <QDir>
#include <QString>
#include <QDebug>

#include <cctype>
#include <iostream>
#include <filesystem>

#include "hashworker.h"
#include "imageviewerdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    pool = new QThread[QThread::idealThreadCount()];
    images.reserve(1000);
    pathEdit = new QLineEdit;
    loadImgBtn = new QPushButton{tr("load")};
    loadImgBtn->setEnabled(false);
    startBtn = new QPushButton{tr("start")};
    startBtn->setEnabled(false);
    dialogBtn = new QPushButton{tr("show result")};
    dialogBtn->hide();
    connect(pathEdit, &QLineEdit::textChanged, [this](){
        if (pathEdit->text() == "") {
            loadImgBtn->setEnabled(false);
            startBtn->setEnabled(false);
        } else {
            loadImgBtn->setEnabled(true);
        }
    });
    connect(loadImgBtn, &QPushButton::clicked, this, &MainWindow::setImages);
    connect(pathEdit, &QLineEdit::returnPressed, this, &MainWindow::setImages);
    connect(startBtn, &QPushButton::clicked, [this]() {
        dialogBtn->hide();
        releaseResultDialog();
        freezeLineLayout();
        sameImageIndex.clear();
        sameImageLists.clear();
        hashes.clear();
        hashes.reserve(images.size());
        bar->show();
        bar->setValue(0);

        for (unsigned long id = 0, start = 0, limit = getNextLimit(0, 0);
             id < getThreadNumber();
             ++id, start = limit, limit = getNextLimit(limit, id)) {
            auto worker = new HashWorker(start, limit, images, hashes, lock);
            worker->moveToThread(pool + id);
            connect(pool + id, &QThread::finished, worker, &QObject::deleteLater);
            connect(worker, &HashWorker::doneAllWork, pool + id, &QThread::quit);
            connect(pool + id, &QThread::started, worker, &HashWorker::doWork);
            connect(worker, &HashWorker::doneOneImg, this, &MainWindow::onProgress);
            connect(worker, &HashWorker::sameImg, [this](const std::string &origin, const std::string &same){
                if (sameImageIndex.find(origin) == sameImageIndex.end()) {
                    sameImageIndex[origin] = sameImageLists.size();
                    // construct vectors directly with a string
                    sameImageLists.emplace_back(std::vector<string>{origin});
                }
                sameImageLists[sameImageIndex[origin]].emplace_back(same);
                auto info = qInfo();
                info.setAutoInsertSpaces(true);
                info << QString::fromStdString(origin) << "same with:" << QString::fromStdString(same);
            });
            (pool + id)->start();
        }
    });
    connect(dialogBtn, &QPushButton::clicked, [this](){
        initResultDialog();
        imageDialog->exec();
    });
    connect(this, &MainWindow::completed, [this](){
        initResultDialog();
        imageDialog->exec();
    });
    bar = new QProgressBar;
    bar->hide();

    fileDialog = new QFileDialog;
    fileDialog->setOption(QFileDialog::ShowDirsOnly, true);
    fileDialog->setOption(QFileDialog::ReadOnly, true);
    fileDialog->setDirectory(QDir::homePath());
    fileDialog->setModal(true);
    fileDialogBtn = new QPushButton(tr("select a directory"));
    connect(fileDialogBtn, &QPushButton::clicked, fileDialog, &QFileDialog::exec);
    connect(fileDialog, &QFileDialog::fileSelected, [this](const QString &dirName) {
        if (dirName == "" || !std::filesystem::exists(dirName.toStdString())) {
            return;
        }

        pathEdit->setText(dirName);
    });

    lineLayout = new QHBoxLayout;
    lineLayout->addWidget(loadImgBtn);
    lineLayout->addWidget(startBtn);
    lineLayout->addWidget(dialogBtn);
    lineLayout->addWidget(pathEdit);
    lineLayout->addWidget(fileDialogBtn);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(lineLayout);
    mainLayout->addWidget(bar);
    setLayout(mainLayout);
}

void MainWindow::onProgress()
{
    const auto value = bar->value();
    bar->setValue(value + 1);
    if (value == bar->maximum() - 1) {
        freezeLineLayout(false);
        // should click load button first
        startBtn->setEnabled(false);
        quitPool();
        bar->hide();
        dialogBtn->show();
        Q_EMIT completed();
    }
}

void MainWindow::setImages()
{
    dialogBtn->hide();
    std::string path = pathEdit->text().toStdString();
    if (!std::filesystem::exists(path)) {
        startBtn->setEnabled(false);
        return;
    }
    std::filesystem::directory_iterator dir{path, std::filesystem::directory_options::skip_permission_denied};
    if (images.size() != 0) {
        images.clear();
    }
    for (const auto &p : dir) {
        if (!p.is_regular_file()) {
            continue;
        }

        auto ext = p.path().extension().generic_u8string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        if (std::find(imgExts.cbegin(), imgExts.cend(), ext) != imgExts.cend()) {
            images.emplace_back(p.path().string());
        }
    }
    startBtn->setEnabled(true);
    bar->setValue(0);
    bar->setMaximum(images.size());
    qInfo() << QString::fromStdString(path) << ": " << images.size();
}

void MainWindow::initResultDialog()
{
    if (imageDialog != nullptr) {
        return;
    }
    imageDialog = new ImageViewerDialog{sameImageLists};
    imageDialog->setModal(true);
}

void MainWindow::releaseResultDialog()
{
    delete imageDialog;
    imageDialog = nullptr;
}

MainWindow::~MainWindow()
{
    if (pool) {
        quitPool(true);
        delete [] pool;
        pool = nullptr;
    }
}
