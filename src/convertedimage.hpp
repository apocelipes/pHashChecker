// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2023 apocelipes

#pragma once

#include <memory>
#include <optional>

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringBuilder>
#include <QProcess>

#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"

namespace {
    [[nodiscard]] inline bool searchPath(const char *file) noexcept
    {
        if (!file || !*file) {
            return false;
        }
        const char *path = std::getenv("PATH");

        if (!path) {
            path = "/usr/local/bin:/bin:/usr/bin";
        }
        std::size_t file_len = strnlen(file, NAME_MAX + 1);
        if (file_len > NAME_MAX) {
            return false;
        }
        std::size_t path_total_len = strnlen(path, PATH_MAX - 1) + 1;

        auto buf = std::make_unique_for_overwrite<char[]>(path_total_len + file_len + 1);
        const char *p = path, *z = nullptr;
        while (true) {
            z = std::strchr(p, ':');
            if (!z) {
                z = p + std::strlen(p);
            }
            if (static_cast<std::size_t>(z - p) >= path_total_len) {
                if (!*z++) {
                    break;
                }
                continue;
            }
            std::memcpy(buf.get(), p, z - p);
            buf[z - p] = '/';
            std::memcpy(buf.get() + (z - p) + (z > p), file, file_len + 1);
            struct stat st{};
            if (stat(buf.get(), &st) == 0 && S_ISREG(st.st_mode) && faccessat(AT_FDCWD, buf.get(), X_OK, AT_EACCESS) == 0) {
                return true;
            }
            if (!*z++) {
                break;
            }
            p = z;
        }
        return false;
    }

    inline const QString &getImageMagickPath() noexcept
    {
        static const QString &path = searchPath("magick") ? QStringLiteral(u"magick") : QStringLiteral(u"convert");
        return path;
    }
}

class ConvertedImage
{
    std::optional<QString> path;
    bool isKeepAlive;

    [[nodiscard]] static QString getConvertedFullName(const QString &srcPath, const int width, const int height) noexcept
    {
        const auto &tmpPath = Utils::getTempDirPath();
        const auto fileName = QFileInfo{srcPath}.baseName();
        // name_WxH.jpg
        const QString &name = fileName % QChar('_')
                           % QString::number(width) % QChar('x') % QString::number(height)
                           % QStringLiteral(u".jpg");
        return QDir::cleanPath(tmpPath % QDir::separator() % name);
    }

    void clear() noexcept
    {
        if (!isKeepAlive && path && !path->isEmpty()) {
            QFile::remove(*path);
        }
    }
public:
    ConvertedImage(const QString &srcPath, const int width, const int height, bool keepAlive = false) noexcept
    : isKeepAlive{keepAlive}
    {
        if (!QFileInfo::exists(srcPath)) [[unlikely]] {
            return;
        }
        const auto &newPath = getConvertedFullName(srcPath, width, height);
        path.emplace(newPath);
        // maybe some TOCTOU problems, but no impacts on this code
        if (QFileInfo::exists(newPath) && isKeepAlive) {
            return;
        }
        QStringList arguments;
        arguments << srcPath
                  << QStringLiteral(u"-quality") << QStringLiteral(u"75")
                  << QStringLiteral(u"-resize") << QString::number(width) % QChar('x') % QString::number(height)
                  << newPath;
        QProcess cmd;
        cmd.start(getImageMagickPath(), arguments);
        if (!cmd.waitForFinished()) {
            qFatal() << QObject::tr("create converted image failed");
        }
        if (cmd.exitCode() != 0) {
            qFatal() << QObject::tr("call ImageMagick failed:") % QString{cmd.readAllStandardError()};
        }
    }

    ~ConvertedImage() noexcept {
        clear();
    }

    ConvertedImage(const ConvertedImage&) = delete;
    ConvertedImage& operator=(const ConvertedImage&) = delete;

    ConvertedImage(ConvertedImage &&other) noexcept: path{std::move(other.path)}, isKeepAlive{other.isKeepAlive} {}
    ConvertedImage& operator=(ConvertedImage &&other) noexcept
    {
        clear();
        path = std::move(other.path);
        isKeepAlive = other.isKeepAlive;
        return *this;
    }

    [[nodiscard]] QString getImagePath() const noexcept
    {
        return path.value_or(QString{});
    }
};

