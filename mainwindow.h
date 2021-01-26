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

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include <pHash.h>

class ImageViewerDialog;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    unsigned int getThreadNumber()
    {
        return std::min(static_cast<unsigned int>(images.size()), static_cast<unsigned int>(QThread::idealThreadCount()));
    }

    unsigned long getNextLimit(const unsigned long oldLimit, const unsigned long threadID)
    {
        if (threadID + 1 == getThreadNumber()) {
            return images.size();
        }
        return oldLimit+images.size()/getThreadNumber();
    }
signals:
    void completed();
public slots:
    void setImages();
    void onProgress();
private:
    void initResultDialog();
    void releaseResultDialog();

    void quitPool(bool cancelAllThread = false) {
        for (int i = 0; i < QThread::idealThreadCount(); ++i) {
            if (cancelAllThread) {
                pool[i].requestInterruption();
            }
            pool[i].quit();
            pool[i].wait();
        }
    }

    void freezeLineLayout(bool flag = true)
    {
        for (int i = 0; i < lineLayout->count(); ++i) {
            lineLayout->itemAt(i)->widget()->setEnabled(!flag);
        }
    }

    QHBoxLayout *lineLayout = nullptr;
    QLineEdit *pathEdit = nullptr;
    QPushButton *loadImgBtn = nullptr;
    QPushButton *startBtn = nullptr;
    QProgressBar *bar = nullptr;
    QPushButton *fileDialogBtn = nullptr;
    QFileDialog *fileDialog = nullptr;
    QPushButton *dialogBtn = nullptr;
    ImageViewerDialog *imageDialog = nullptr;

    std::vector<std::string> images;
    std::unordered_map<ulong64, std::string> hashes;
    std::unordered_map<std::string, std::size_t> sameImageIndex;
    std::vector<std::vector<std::string>> sameImageLists;
    QReadWriteLock lock;
    QThread *pool = nullptr;

    // supported image formats
    inline static std::vector<std::string> imgExts {
        ".jpg",
        ".jpeg",
        ".png",
        ".webp",
        ".bmp"
    };
};
#endif // MAINWINDOW_H
