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
public slots:
    void setImages();
    void onProgress();
private:
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

    QHBoxLayout *lineLayout;
    QLineEdit *pathEdit;
    QPushButton *loadImgBtn;
    QPushButton *startBtn;
    QProgressBar *bar;
    QPushButton *fileDialogBtn;
    QFileDialog *dialog;

    std::vector<std::string> images;
    std::unordered_map<ulong64, std::string> hashes;
    QReadWriteLock lock;
    QThread *pool;
};
#endif // MAINWINDOW_H
