// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#include <QVBoxLayout>
#include <QDir>
#include <QString>
#include <QStringBuilder>
#include <QFileInfo>

#include <atomic>
#include <filesystem>
#include <thread>

#include "mainwindow.h"
#include "hashworker.h"
#include "imageviewerdialog.h"
#include "notificationbar.h"
#include "utils/sizeformat.h"
#include "utils/utils.h"

namespace {
    template <typename T>
    concept IsDirIterator = requires(T iter) {
        { *iter } -> std::same_as<const std::filesystem::directory_entry&>;
        { *std::filesystem::begin(iter) } -> std::same_as<const std::filesystem::directory_entry&>;
        { *std::filesystem::end(iter) } -> std::same_as<const std::filesystem::directory_entry&>;
    };

    inline void fillImages(IsDirIterator auto &&dir, std::vector<std::string> &images) noexcept
    {
        auto result = dir | std::views::filter([](const std::filesystem::directory_entry &p) { return p.is_regular_file(); })
                          | std::views::filter([](const std::filesystem::directory_entry &p) {
                                const auto &path = QString::fromStdString(p.path().string());
                                return Utils::isSupportImageFormat(path);
                            })
                          | std::views::transform([](const std::filesystem::directory_entry &p) { return p.path().string(); });
        std::ranges::copy(result, std::back_inserter(images)); // using c++23's ranges::to is the best way
    }

    template <std::ranges::range Container>
    [[nodiscard]] inline size_t getThreadNumber(const Container &contents) noexcept
    {
        size_t nThreads = 1;
        if (int n = QThread::idealThreadCount(); n > 1) {
            nThreads = static_cast<size_t>(n);
        }
        return std::min(std::ranges::size(contents), nThreads);
    }

    [[nodiscard]] constexpr inline size_t getNextLimit(const size_t oldLimit, const size_t threadID, const size_t maxThreads, const size_t totalItems) noexcept
    {
        if (threadID + 1 == maxThreads) {
            return totalItems;
        }
        return oldLimit+totalItems / maxThreads;
    }

    template <std::ranges::range Container>
    inline uint64_t countFilesSize(const Container &files) noexcept
    {
        // std::atomic_ref<uint64_t> must be lock-free
        static_assert(std::atomic_ref<uint64_t>::is_always_lock_free, "std::atomic_ref<uint64_t> is not lock-free");
        alignas(std::atomic_ref<uint64_t>::required_alignment) uint64_t result{};
        std::atomic_ref<uint64_t> ret{result};

        const auto nThreads = getThreadNumber(files);
        std::vector<std::thread> threads;
        threads.reserve(nThreads);

        // Because QThreadPool doesn't support move-only functions until 6.6.0, use STL for back compatibilities
        for (size_t id = 0, start = 0, limit = getNextLimit(0, 0, nThreads, files.size());
            id < nThreads;
            ++id, start = limit, limit = getNextLimit(limit, id, nThreads, files.size())) {
            threads.emplace_back([start, limit, &files, &ret](){
                uint64_t size{};

                for (size_t i{start}; i < limit; ++i) {
                    size += std::filesystem::file_size(files[i]);
                }

                ret.fetch_add(size, std::memory_order_relaxed);
            });
        }
    
        for (size_t i = 0; i < nThreads; ++i) {
            threads[i].join();
        }

        return result;
    }
}

MainWindow::MainWindow(QWidget *parent) noexcept
    : QWidget(parent)
{
    pool.reserve(QThread::idealThreadCount());
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
            disableStartBtn();
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

        const auto nThreads = getThreadNumber(images);
        init_pool(nThreads);
        const auto distance = settings->getSimilarDistance();
        for (size_t id = 0, start = 0, limit = getNextLimit(0, 0, nThreads, images.size());
             id < nThreads;
             ++id, start = limit, limit = getNextLimit(limit, id,nThreads, images.size())) {
            // cannot use a QThreadPool because we need an event-loop in our worker functions
            auto worker = new HashWorker(start, limit, distance, images, hashes, insertHistory, hashesLock);
            worker->moveToThread(pool[id].get());
            connect(pool[id].get(), &QThread::finished, worker, &QObject::deleteLater);
            connect(worker, &HashWorker::doneAllWork, pool[id].get(), &QThread::quit);
            connect(pool[id].get(), &QThread::started, worker, &HashWorker::doWork);
            connect(worker, &HashWorker::doneOneImg, this, &MainWindow::onProgress);
            connect(worker, &HashWorker::sameImg, this, [this](size_t originIndex, size_t sameIndex){
                const auto &origin = images[originIndex];
                const auto &same = images[sameIndex];
                if (!sameImageResults.contains(origin)) {
                    // construct vectors directly with the origin image
                    sameImageResults.emplace(origin, std::vector<std::string>{origin});
                }
                sameImageResults[origin].emplace_back(same);
                qDebug() << QString::fromStdString(origin) % tr(" same with: ") % QString::fromStdString(same);
            });
            pool[id]->start();
        }
    });

    auto showResultDialog = [this](){
        initResultDialog(); // ownerships of sameImageResults' elements will be taken
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
        // wait for all threads to exit before cleaning up, otherwise data races will occur
        insertHistory.clear();
        hashes.clear();
        sameImageResults.clear();
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
        if (dirName.isEmpty() || !QFileInfo::exists(dirName)) {
            return;
        }

        pathEdit->setText(dirName);
        disableStartBtn();
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

void MainWindow::onProgress() noexcept
{
    const auto value = bar->value();
    bar->setValue(value + 1);
    if (bar->value() != bar->maximum()) [[likely]] {
        return;
    }

    freezeMainGUI(false);
    // should click load button first
    disableStartBtn();
    quitPool();
    bar->hide();
    cancelButton->hide();
    dialogBtn->show();
    sort_result();
    Q_EMIT completed();
}

void MainWindow::setImages() noexcept
{
    dialogBtn->hide();
    releaseResultDialog();
    sameImageResults.clear();
    hashes.clear();
    images.clear();
    insertHistory.clear();

    info->hide(); // 重写的hide会设置isClosing
    const auto &path = pathEdit->text();
    if (!QDir{path}.exists()) {
        disableStartBtn();
        info->setText(path % tr(" directory does not exist"));
        info->animatedShow();
        return;
    }

    constexpr auto opts = std::filesystem::directory_options::skip_permission_denied;
    if (settings->isRecursiveSearching()) {
        fillImages(std::filesystem::recursive_directory_iterator{path.toStdString(), opts}, images);
    } else {
        fillImages(std::filesystem::directory_iterator{path.toStdString(), opts}, images);
    }
    QCoreApplication::processEvents();
    if (!images.empty()) {
        bar->setValue(0);
        bar->setMaximum(static_cast<int>(images.size()));
        const auto &totalSize = Utils::sizeFormat(countFilesSize(images));
        startBtn->setEnabled(true);
        startBtn->setToolTip(tr("%1 images, total size: %2").arg(images.size()).arg(totalSize));
    } else {
        disableStartBtn();
        info->setText(tr("no image here"));
        info->animatedShow();
    }
}

void MainWindow::initResultDialog() noexcept
{
    if (imageDialog != nullptr) {
        return;
    }
    dialogBtn->setEnabled(false);
    imageDialog = new ImageViewerDialog{std::move(sameImageResults)};
    sameImageResults = SameImagesContainer{};
    imageDialog->setModal(true);
    dialogBtn->setEnabled(true);
}

void MainWindow::releaseResultDialog() noexcept
{
    if (imageDialog != nullptr) {
        imageDialog->deleteLater();
        imageDialog = nullptr;
    }
}

MainWindow::~MainWindow() noexcept
{
    if (!pool.empty()) {
        quitPool(true);
    }
}
