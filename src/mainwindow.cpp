// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#include <QVBoxLayout>
#include <QCompleter>
#include <QFileDialog>
#include <QProgressBar>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QRegularExpression>
#include <QKeySequence>

#include <atomic>
#include <filesystem>
#include <iterator>
#include <ranges>
#include <thread>

#include "aboutdialog.h"
#include "mainwindow.h"
#include "imageviewerdialog.h"
#include "notificationbar.h"
#include "stopwatchdialog.h"
#include "utils/path.h"
#include "utils/sizeformat.h"

#include <string.h> // for POSIX strcasecmp

namespace {
    [[nodiscard]] inline bool isSupportedImageFormat(const std::string &img) noexcept
    {
        static constexpr std::array imageExtensions{
            ".jpg",
            ".png",
            ".jpeg",
            ".avif",
            ".webp",
            ".bmp",
            ".jxl",
        };

        const auto ext = std::strrchr(img.c_str(), '.');
        if (ext == nullptr) [[unlikely]] {
            return false;
        }

        return imageExtensions.cend() != std::ranges::find_if(imageExtensions, [ext](const char *s) {
            return strcasecmp(s, ext) == 0;
        });
    }

    template <typename T>
    concept IsDirIterator = requires(T iter) {
        { *iter } -> std::same_as<const std::filesystem::directory_entry&>;
        { *std::filesystem::begin(iter) } -> std::same_as<const std::filesystem::directory_entry&>;
        { *std::filesystem::end(iter) } -> std::same_as<const std::filesystem::directory_entry&>;
    };

    inline void fillImages(IsDirIterator auto &&dir, std::output_iterator<std::string> auto &&output) noexcept
    {
        auto result = dir | std::views::filter([](const std::filesystem::directory_entry &p) { return p.is_regular_file() && isSupportedImageFormat(p.path().native()); })
                          | std::views::transform([](const std::filesystem::directory_entry &p) { return p.path().string(); });
        std::ranges::copy(result, output); // using c++23's ranges::to is the best way
    }

    template <std::ranges::range Container>
    [[nodiscard]] inline std::size_t getThreadNumber(const Container &contents) noexcept
    {
        std::size_t nThreads = 1;
        if (int n = QThread::idealThreadCount(); n > 1) {
            nThreads = static_cast<std::size_t>(n);
        }
        return std::min(std::ranges::size(contents), nThreads);
    }

    [[nodiscard]] constexpr inline std::size_t getNextLimit(const std::size_t oldLimit, const std::size_t threadID, const std::size_t maxThreads, const std::size_t totalItems) noexcept
    {
        if (threadID + 1 == maxThreads) {
            return totalItems;
        }
        return oldLimit+totalItems / maxThreads;
    }

    template <std::ranges::range Container>
    [[nodiscard]] inline uint64_t countFilesSize(const Container &files) noexcept
    {
        // std::atomic_ref<uint64_t> must be lock-free
        static_assert(std::atomic_ref<uint64_t>::is_always_lock_free, "std::atomic_ref<uint64_t> is not lock-free");
        alignas(std::atomic_ref<uint64_t>::required_alignment) uint64_t result{};
        std::atomic_ref<uint64_t> ret{result};

        const auto nThreads = getThreadNumber(files);
        std::vector<std::thread> threads;
        threads.reserve(nThreads);

        // Because QThreadPool doesn't support move-only functions until 6.6.0, use STL for back compatibilities
        for (std::size_t id = 0, start = 0, limit = getNextLimit(0, 0, nThreads, files.size());
            id < nThreads;
            ++id, start = limit, limit = getNextLimit(limit, id, nThreads, files.size())) {
            threads.emplace_back([start, limit, &files, &ret](){
                uint64_t size{};

                for (std::size_t i{start}; i < limit; ++i) {
                    size += std::filesystem::file_size(files[i]);
                }

                ret.fetch_add(size, std::memory_order_relaxed);
            });
        }
    
        for (std::size_t i = 0; i < nThreads; ++i) {
            threads[i].join();
        }

        return result;
    }

    [[nodiscard]] inline QString replaceWithHomeDir(const QString &text) noexcept
    {
        static const auto &shellHomeDirPattern = QRegularExpression{"^~/"};
        return text.trimmed().replace(shellHomeDirPattern, QDir::homePath()+'/');
    }
}

MainWindow::MainWindow(QWidget *parent) noexcept
    : QWidget(parent)
{
    pool.reserve(QThread::idealThreadCount());

    pathEdit = new QLineEdit;
    pathEdit->setPlaceholderText(tr("entry a directory"));
    pathEdit->setMinimumWidth(pathEdit->fontMetrics().averageCharWidth() * MIN_EDIT_WIDTH);
    pathCompleter = new QCompleter{this};
    pathCompleter->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    auto fsModel = new QFileSystemModel{pathCompleter};
    fsModel->setOptions(QFileSystemModel::DontWatchForChanges|QFileSystemModel::DontUseCustomDirectoryIcons);
    fsModel->setFilter(QDir::AllDirs|QDir::NoDotAndDotDot);
    fsModel->setReadOnly(true);
    fsModel->setRootPath("/");
    pathCompleter->setModel(fsModel);
    pathCompleter->setMaxVisibleItems(5);
    pathEdit->setCompleter(pathCompleter);

    loadImgBtn = new QPushButton{tr("load")};
    loadImgBtn->setEnabled(false);
    loadImgBtn->setShortcut(QKeySequence{Qt::CTRL | Qt::Key_Return});
    loadImgBtn->setToolTip(tr("Ctrl + Enter"));
    startBtn = new QPushButton{tr("start")};
    startBtn->setEnabled(false);
    startBtn->setShortcut(QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_R});
    startBtn->setToolTip(tr("Ctrl + Shift + R"));
    dialogBtn = new QPushButton{tr("show result")};
    dialogBtn->hide();
    timerDialog = new StopwatchDialog{tr("Stopwatch Dialog"), this};
    connect(pathEdit, &QLineEdit::textChanged, this, [this](const QString &text) noexcept {
        const auto &path = text.trimmed();
        if (path.isEmpty()) {
            loadImgBtn->setEnabled(false);
            pathEdit->setCompleter(nullptr);
            disableStartBtn();
            return;
        }

        loadImgBtn->setEnabled(true);
        if (pathEdit->completer() == nullptr) {
            pathEdit->setCompleter(pathCompleter);
        }
    });
    connect(pathEdit, &QLineEdit::textEdited, this, [this](const QString &text) noexcept {
        if (text.trimmed().startsWith("~/")) {
            const auto &path = replaceWithHomeDir(text);
            pathEdit->setText(path);
            pathCompleter->setCompletionPrefix(path);
            pathCompleter->complete();
        }
    });
    connect(loadImgBtn, &QPushButton::clicked, this, &MainWindow::setImages);
    connect(startBtn, &QPushButton::clicked, this, [this]() noexcept {
        freezeMainGUI(true);
        matchHistory.reserve(images.size());
        bar->setEnabled(true);
        bar->show();
        bar->setValue(0);
        cancelButton->setEnabled(true);
        cancelButton->show();

        const auto nThreads = getThreadNumber(images);
        init_pool(nThreads);
        const auto distance = settings->getSimilarDistance();
        if (settings->isUseStopwatchDialog()) {
            timerDialog->start();
        }
        for (std::size_t id = 0, start = 0, limit = getNextLimit(0, 0, nThreads, images.size());
             id < nThreads;
             ++id, start = limit, limit = getNextLimit(limit, id, nThreads, images.size())) {
            // cannot use a QThreadPool because we need an event-loop in our worker functions
            auto worker = new HashWorker(distance, std::span{images.begin()+start, limit-start}, matchHistory);
            worker->moveToThread(pool[id].get());
            connect(pool[id].get(), &QThread::finished, worker, &QObject::deleteLater);
            connect(worker, &HashWorker::doneAllWork, pool[id].get(), &QThread::quit);
            connect(pool[id].get(), &QThread::started, worker, &HashWorker::doWork);
            connect(worker, &HashWorker::doneOneImg, this, &MainWindow::onProgress);
            connect(worker, &HashWorker::sameImg, this, [this](const std::string_view origin, const std::string_view same) noexcept {
                std::string originImg{origin};
                std::string sameImg{same};
                qDebug() << Utils::getAbsPath(QString::fromStdString(originImg)) % tr(" same with: ") % Utils::getAbsPath(QString::fromStdString(sameImg));
                if (!sameImageResults.contains(origin)) {
                    // construct vectors directly with the origin image
                    sameImageResults.emplace(origin, std::vector<std::string>{std::move(originImg), std::move(sameImg)});
                } else {
                    sameImageResults[origin].emplace_back(std::move(sameImg));
                }
            });
            pool[id]->start();
        }
    });

    connect(dialogBtn, &QPushButton::clicked, this, [this]() noexcept {
        initResultDialog();
        imageDialog->exec();
    });
    connect(this, &MainWindow::completed, this, [this]() noexcept {
        initResultDialog();
        if (settings->isUseStopwatchDialog()) {
            timerDialog->exec();
        }
        imageDialog->exec();
    });

    bar = new QProgressBar;
    bar->hide();
    cancelButton = new QPushButton{tr("cancel")};
    cancelButton->setCursor(Qt::PointingHandCursor);
    cancelButton->hide();
    connect(cancelButton, &QPushButton::clicked, this, [this]() noexcept {
        cancelButton->setEnabled(false); // quitPool在取消线程时较耗时，防止反复触发
        bar->setEnabled(false);
        quitPool(true);
        // wait for all threads to exit before cleaning up, otherwise data races will occur
        matchHistory.clear();
        sameImageResults.clear();
        freezeMainGUI(false);
        bar->hide();
        cancelButton->hide();
        if (settings->isUseStopwatchDialog()) {
            timerDialog->stop();
        }
    });

    fileDialog = new QFileDialog{this};
    fileDialog->setOption(QFileDialog::ShowDirsOnly, true);
    fileDialog->setOption(QFileDialog::ReadOnly, true);
    fileDialog->setFileMode(QFileDialog::Directory);
    fileDialog->setDirectory(QDir::homePath());
    fileDialog->setModal(true);
    fileDialogBtn = new QPushButton(tr("select a directory"));
    connect(fileDialogBtn, &QPushButton::clicked, fileDialog, &QFileDialog::exec);
    connect(fileDialog, &QFileDialog::fileSelected, this, [this](const QString &dirName) noexcept {
        if (dirName.isEmpty() || !QFileInfo::exists(dirName)) [[unlikely]] {
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
    auto aboutBtn = new QPushButton{tr("about"), this};
    connect(aboutBtn, &QPushButton::clicked, this, []() noexcept {
        AboutDialog aboutDialog;
        aboutDialog.exec();
    });
    lineLayout->addWidget(aboutBtn);

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
    startBtn->hide();
    sort_result();
    if (settings->isUseStopwatchDialog()) {
        timerDialog->stop();
    }
    Q_EMIT completed();
}

void MainWindow::setImages() noexcept
{
    const auto path = replaceWithHomeDir(pathEdit->text()); // maybe unnecessary
    if (path.isEmpty()) {
        return;
    }

    dialogBtn->hide();
    dialogBtn->setEnabled(false);
    releaseResultDialog();
    startBtn->setEnabled(false);
    startBtn->show();
    sameImageResults.clear();
    images.clear();
    matchHistory.clear();

    info->hide(); // 重写的hide会设置isClosing
    if (!QFileInfo::exists(path)) {
        disableStartBtn();
        info->setText(path % tr(" directory does not exist"));
        info->animatedShow();
        return;
    }

    constexpr auto opts = std::filesystem::directory_options::skip_permission_denied;
    std::filesystem::current_path(path.toStdString());
    if (settings->isRecursiveSearching()) {
        fillImages(std::filesystem::recursive_directory_iterator{".", opts}, std::back_inserter(images));
    } else {
        fillImages(std::filesystem::directory_iterator{".", opts}, std::back_inserter(images));
    }
    QCoreApplication::processEvents();
    if (!images.empty()) {
        bar->setValue(0);
        bar->setMaximum(static_cast<int>(images.size()));
        const auto &totalSize = Utils::sizeFormat(countFilesSize(images));
        startBtn->show();
        startBtn->setEnabled(true);
        startBtn->setToolTip(tr("%1 images, total size: %2<br/>Ctrl + Shift + R").arg(images.size()).arg(totalSize));
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
    matchHistory = MatchHistoryContainer{};
    images = std::vector<std::string>{};
    imageDialog = new ImageViewerDialog{std::move(sameImageResults)};
    sameImageResults = SameImagesContainer{};
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
