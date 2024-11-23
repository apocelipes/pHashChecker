// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#include <QThread>
#include <QReadWriteLock>
#include <QStringBuilder>

#include <algorithm>
#include <ranges>
#include <utility>

#include "hashworker.h"

namespace {
    inline QReadWriteLock matchHistoryLock;
}

void HashWorker::doWork() noexcept
{
    for (std::size_t index = _start; index < _limit; ++index) {
        if (this->thread()->isInterruptionRequested()) {
            qInfo() << tr("thread exit");
            break;
        }

        ulong64 hash = 0;
        bool foundSame = false;
        if (ph_dct_imagehash(_images[index].c_str(), hash) < 0) [[unlikely]] {
            qWarning() << tr("calculating pHash failed, skip: ") % QString::fromStdString(_images[index]);
            Q_EMIT doneOneImg();
            continue;
        }

        const auto pred = [this, hash](const auto &item) {
            return checkSameImage(hash, item.first);
        };
        matchHistoryLock.lockForRead();
        // 获得读锁后即为最新的size
        // 在获取读锁之前取得size，size可能会在读锁阻塞期间被更新，导致已经进入hashes的数据被重复比较
        auto lastInsertIndex = _matchHistory.size();
        if (auto iter = std::ranges::find_if(std::as_const(_matchHistory), pred); iter != _matchHistory.end()) {
            foundSame = true;
            Q_EMIT sameImg(iter->second, index);
        }
        matchHistoryLock.unlock();

        if (foundSame) {
            Q_EMIT doneOneImg();
            continue;
        }

        matchHistoryLock.lockForWrite();
        if (auto iter = std::ranges::find_if(std::as_const(_matchHistory) | std::views::drop(lastInsertIndex), pred); iter != _matchHistory.end()) {
            foundSame = true;
            Q_EMIT sameImg(iter->second, index);
        }
        if (!foundSame) {
            _matchHistory.emplace_back(hash, index);
        }
        matchHistoryLock.unlock();
        Q_EMIT doneOneImg();
    }

    Q_EMIT doneAllWork();
}
