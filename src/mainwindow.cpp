// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2022 apocelipes

#include <QApplication>
#include <QVBoxLayout>
#include <QDir>
#include <QString>
#include <QDebug>

#include <cctype>
#include <iostream>
#include <filesystem>

#include "mainwindow.h"
#include "hashworker.h"
#include "imageviewerdialog.h"
#include "notificationbar.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    pool = new QThread[QThread::idealThreadCount()];
    images.reserve(1000);
    pathEdit = new QLineEdit;
    pathEdit->setPlaceholderText(tr("entry a directory"));
    pathEdit->setMinimumWidth(pathEdit->fontMetrics().averageCharWidth() * MIN_EDIT_WIDTH);
    loadImgBtn = new QPushButton{tr("load")};
    loadImgBtn->setEnabled(false);
    startBtn = new QPushButton{tr("start")};
    startBtn->setEnabled(false);
    dialogBtn = new QPushButton{tr("show result")};
    dialogBtn->hide();
    connect(pathEdit, &QLineEdit::textChanged, this, [this](){
        if (pathEdit->text().isEmpty()) {
            loadImgBtn->setEnabled(false);
            startBtn->setEnabled(false);
        } else {
            loadImgBtn->setEnabled(true);
        }
    });
    connect(loadImgBtn, &QPushButton::clicked, this, &MainWindow::setImages);
    connect(pathEdit, &QLineEdit::returnPressed, this, &MainWindow::setImages);
    connect(startBtn, &QPushButton::clicked, this, [this]() {
        freezeMainGUI(true);
        hashes.reserve(images.size());
        insertHistory.reserve(images.size());
        bar->setEnabled(true);
        bar->show();
        bar->setValue(0);
        cancelButton->setEnabled(true);
        cancelButton->show();

        for (unsigned long id = 0, start = 0, limit = getNextLimit(0, 0);
             id < getThreadNumber();
             ++id, start = limit, limit = getNextLimit(limit, id)) {
            auto worker = new HashWorker(start, limit, images, hashes, insertHistory, hashesLock);
            worker->moveToThread(pool + id);
            connect(pool + id, &QThread::finished, worker, &QObject::deleteLater);
            connect(worker, &HashWorker::doneAllWork, pool + id, &QThread::quit);
            connect(pool + id, &QThread::started, worker, &HashWorker::doWork);
            connect(worker, &HashWorker::doneOneImg, this, &MainWindow::onProgress);
            connect(worker, &HashWorker::sameImg, this, [this](const std::string &origin, const std::string &same){
                if (sameImageIndex.count(origin) == 0) {
                    sameImageIndex.emplace(origin, sameImageLists.size());
                    // construct vectors directly with a string
                    sameImageLists.emplace_back(std::vector<std::string>{origin});
                }
                sameImageLists[sameImageIndex[origin]].emplace_back(same);
                auto output = qDebug();
                output.setAutoInsertSpaces(true);
                output << QString::fromStdString(origin) << "same with: " << QString::fromStdString(same);
            });
            (pool + id)->start();
        }
    });

    auto showResultDialog = [this](){
        initResultDialog();
        imageDialog->exec();
    };
    connect(dialogBtn, &QPushButton::clicked, this, showResultDialog);
    connect(this, &MainWindow::completed, this, showResultDialog);

    bar = new QProgressBar;
    bar->hide();
    cancelButton = new QPushButton{tr("cancel")};
    cancelButton->setCursor(Qt::PointingHandCursor);
    cancelButton->hide();
    connect(cancelButton, &QPushButton::clicked, this, [this](){
        cancelButton->setEnabled(false); // quitPool在取消线程时较耗时，防止反复触发
        bar->setEnabled(false);
        quitPool(true);
        freezeMainGUI(false);
        bar->hide();
        cancelButton->hide();
    });

    fileDialog = new QFileDialog{this};
    fileDialog->setOption(QFileDialog::ShowDirsOnly, true);
    fileDialog->setOption(QFileDialog::ReadOnly, true);
    fileDialog->setFileMode(QFileDialog::Directory);
    fileDialog->setDirectory(QDir::homePath());
    fileDialog->setModal(true);
    fileDialogBtn = new QPushButton(tr("select a directory"));
    connect(fileDialogBtn, &QPushButton::clicked, fileDialog, &QFileDialog::exec);
    connect(fileDialog, &QFileDialog::fileSelected, this, [this](const QString &dirName) {
        if (dirName == "" || !std::filesystem::exists(dirName.toStdString())) {
            return;
        }

        pathEdit->setText(dirName);
        startBtn->setEnabled(false);
    });

    info = NotificationBar::createNotificationBar(NotificationBar::NotificationType::ERROR, "", this);
    info->setCloseButtonVisible(true);

    settings = new SettingPanel{this};

    lineLayout = new QHBoxLayout;
    lineLayout->addWidget(loadImgBtn);
    lineLayout->addWidget(startBtn);
    lineLayout->addWidget(dialogBtn);
    lineLayout->addWidget(pathEdit);
    lineLayout->addWidget(fileDialogBtn);
    auto aboutQtBtn = new QPushButton{tr("about"), this};
    connect(aboutQtBtn, &QPushButton::clicked, this, [](){
        QApplication::aboutQt();
    });
    lineLayout->addWidget(aboutQtBtn);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(info);
    mainLayout->addLayout(lineLayout);
    auto settingLayout = new QHBoxLayout;
    settingLayout->addWidget(settings, 2);
    settingLayout->addStretch(1);
    mainLayout->addLayout(settingLayout);
    auto progressLayout = new QHBoxLayout;
    progressLayout->addWidget(bar);
    progressLayout->addWidget(cancelButton);
    mainLayout->addLayout(progressLayout);
    setLayout(mainLayout);
    pathEdit->setFocus();
}

void MainWindow::onProgress()
{
    const auto value = bar->value();
    bar->setValue(value + 1);
    if (value == bar->maximum() - 1) {
        freezeMainGUI(false);
        // should click load button first
        startBtn->setEnabled(false);
        quitPool();
        bar->hide();
        cancelButton->hide();
        dialogBtn->show();
        sort_result();
        Q_EMIT completed();
    }
}

void MainWindow::setImages()
{
    dialogBtn->hide();
    releaseResultDialog();
    sameImageIndex.clear();
    sameImageLists.clear();
    hashes.clear();
    images.clear();
    insertHistory.clear();

    info->hide(); //FIXME: animatedHide的动画是异步执行的，不等待动画结束就进行下面的操作会导致animatedShow提前执行，导致动画被中断，从而无法更新isClosing标志
    std::string path = pathEdit->text().toStdString();
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
        startBtn->setEnabled(false);
        info->setText(QString::fromStdString(path) + tr(" does not exist"));
        info->animatedShow();
        return;
    }

    std::filesystem::directory_iterator dir{path, std::filesystem::directory_options::skip_permission_denied};
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
    if (!images.empty()) {
        startBtn->setEnabled(true);
        bar->setValue(0);
        bar->setMaximum(static_cast<int>(images.size()));
    } else {
        startBtn->setEnabled(false);
        info->setText(tr("no image here"));
        info->animatedShow();
    }
}

void MainWindow::initResultDialog()
{
    if (imageDialog != nullptr) {
        return;
    }
    dialogBtn->setEnabled(false);
    imageDialog = new ImageViewerDialog{sameImageLists};
    imageDialog->setModal(true);
    dialogBtn->setEnabled(true);
}

void MainWindow::releaseResultDialog()
{
    if (imageDialog != nullptr) {
        imageDialog->deleteLater();
        imageDialog = nullptr;
    }
}

MainWindow::~MainWindow()
{
    if (pool) {
        quitPool(true);
        delete [] pool;
        pool = nullptr;
    }
}
