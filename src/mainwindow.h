// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2021 apocelipes

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QReadWriteLock>
#include <QThread>

#include <array>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include <pHash.h>

#include "settingpanel.h"

class ImageViewerDialog;
class NotificationBar;

constexpr int MIN_EDIT_WIDTH = 30;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    unsigned int getThreadNumber() noexcept
    {
        return std::min(static_cast<unsigned int>(images.size()), static_cast<unsigned int>(QThread::idealThreadCount()));
    }

    unsigned long getNextLimit(const unsigned long oldLimit, const unsigned long threadID) noexcept
    {
        if (threadID + 1 == getThreadNumber()) {
            return images.size();
        }
        return oldLimit+images.size()/getThreadNumber();
    }

Q_SIGNALS:
    void completed();

public Q_SLOTS:
    void setImages();
    void onProgress();

private:
    void initResultDialog();
    void releaseResultDialog();

    void quitPool(bool cancelAllThread = false)
    {
        for (int i = 0; i < QThread::idealThreadCount(); ++i) {
            if (cancelAllThread) {
                pool[i].requestInterruption();
            }
            pool[i].quit();
            pool[i].wait();
        }
    }

    void freezeMainGUI(bool flag)
    {
        freezeLayout(lineLayout, flag);
        freezeLayout(settings->layout(), flag);
        if (flag) {
            setCursor(Qt::WaitCursor);
        } else {
            unsetCursor();
        }
    }

    void freezeLayout(QLayout *layout, bool flag)
    {
        for (int i = 0; i < layout->count(); ++i) {
            auto widget = layout->itemAt(i)->widget();
            if (widget) { // check for not widget layoutitems
                widget->setEnabled(!flag);
            }
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
    std::unordered_map<std::string, std::size_t> sameImageIndex;
    std::vector<std::vector<std::string>> sameImageLists;
    std::vector<ulong64> insertHistory;
    QReadWriteLock hashesLock;
    QThread *pool = nullptr;

    // supported image formats
    inline static std::array<std::string, 5> imgExts {
        ".jpg",
        ".jpeg",
        ".png",
        ".webp",
        ".bmp"
    };
};

#endif // MAINWINDOW_H
