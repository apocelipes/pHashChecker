// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2021 apocelipes

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
#include <unordered_map>

#include <pHash.h>

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

    [[nodiscard]] unsigned int getThreadNumber() noexcept
    {
        return std::min(static_cast<unsigned int>(images.size()), static_cast<unsigned int>(QThread::idealThreadCount()));
    }

    [[nodiscard]] unsigned long getNextLimit(const unsigned long oldLimit, const unsigned long threadID) noexcept
    {
        if (threadID + 1 == getThreadNumber()) {
            return images.size();
        }
        return oldLimit+images.size()/getThreadNumber();
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
        for (auto &[_, v] : sameImageResults) {
            std::sort(v.begin(), v.end());
        }
    }

    void init_pool(unsigned long nThreads) noexcept {
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
    std::unordered_map<ulong64, std::string> hashes;
    std::unordered_map<std::string, std::vector<std::string>> sameImageResults;
    std::vector<ulong64> insertHistory;
    QReadWriteLock hashesLock;
    std::vector<std::unique_ptr<QThread>> pool;
};

#endif // MAINWINDOW_H
