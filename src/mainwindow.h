// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCoreApplication>
#include <QWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QReadWriteLock>
#include <QThread>

#include <algorithm>
#include <chrono>
#include <string>
#include <vector>

#include <pHash.h>
#include <ankerl/unordered_dense.h>

#include "settingpanel.h"

class ImageViewerDialog;
class NotificationBar;

constexpr int MIN_EDIT_WIDTH = 30;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr) noexcept;
    ~MainWindow() noexcept override;

    [[nodiscard]] size_t getThreadNumber() const noexcept
    {
        size_t nThreads = 1;
        if (int n = QThread::idealThreadCount(); n > 1) {
            nThreads = static_cast<size_t>(n);
        }
        return std::min(images.size(), nThreads);
    }

    [[nodiscard]] size_t getNextLimit(const size_t oldLimit, const size_t threadID) const noexcept
    {
        if (threadID + 1 == getThreadNumber()) {
            return images.size();
        }
        return oldLimit+images.size() / getThreadNumber();
    }

Q_SIGNALS:
    void completed();

public Q_SLOTS:
    void setImages() noexcept;
    void onProgress() noexcept;

private:
    void initResultDialog() noexcept;
    void releaseResultDialog() noexcept;

    void quitPool(bool cancelAllThread = false) noexcept
    {
        using namespace std::chrono_literals;
        for (const auto &thread : pool) {
            if (cancelAllThread) {
                thread->requestInterruption();
            }
            thread->quit();
            thread->wait(3s);
            QCoreApplication::processEvents();
        }
    }

    void freezeMainGUI(const bool flag) noexcept
    {
        freezeLayout(lineLayout, flag);
        freezeLayout(settings->layout(), flag);
        if (flag) {
            setCursor(Qt::WaitCursor);
        } else {
            unsetCursor();
        }
    }

    static void freezeLayout(QLayout *layout, bool flag) noexcept
    {
        for (int i = 0; i < layout->count(); ++i) {
            auto it = layout->itemAt(i);
            if (it->layout()) {
                freezeLayout(it->layout(), flag);
            } else if (auto widget = it->widget()) { // check for not widget layoutItems
                widget->setEnabled(!flag);
            }
        }
    }

    void sort_result() noexcept
    {
        std::ranges::for_each(sameImageResults, [](auto &result){
            std::ranges::sort(result.second);
        });
    }

    void init_pool(const size_t nThreads) noexcept {
        const auto oldSize = pool.size();
        quitPool(true);
        if (oldSize > nThreads) {
            pool.resize(nThreads);
            return;
        }
        for (std::size_t i = nThreads; i > oldSize; --i) {
            pool.emplace_back(std::make_unique<QThread>(this));
        }
    }

    void disableStartBtn() noexcept
    {
        startBtn->setEnabled(false);
        startBtn->setToolTip(QString{});
    }

    uint64_t countFilesSize2() const noexcept;

    QHBoxLayout *lineLayout = nullptr;
    QLineEdit *pathEdit = nullptr;
    QPushButton *loadImgBtn = nullptr;
    QPushButton *startBtn = nullptr;
    QProgressBar *bar = nullptr;
    QPushButton *cancelButton = nullptr;
    QPushButton *fileDialogBtn = nullptr;
    QFileDialog *fileDialog = nullptr;
    QPushButton *dialogBtn = nullptr;
    ImageViewerDialog *imageDialog = nullptr;
    NotificationBar *info = nullptr;
    SettingPanel *settings = nullptr;

    std::vector<std::string> images;
    ankerl::unordered_dense::map<ulong64, size_t> hashes;
    ankerl::unordered_dense::map<std::string, std::vector<std::string>> sameImageResults;
    std::vector<ulong64> insertHistory;
    QReadWriteLock hashesLock;
    std::vector<std::unique_ptr<QThread>> pool;
};

#endif // MAINWINDOW_H
