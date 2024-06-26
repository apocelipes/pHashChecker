// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#include <QThread>
#include <QDebug>
#include <QReadWriteLock>
#include <QStringBuilder>

#include <algorithm>
#include <ranges>
#include <utility>

#include "hashworker.h"

void HashWorker::doWork()
{
    for (size_t index = _start; index < _limit; ++index) {
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
        _hashesLock.lockForRead();
        // 获得读锁后即为最新的size
        // 在获取读锁之前取得size，size可能会在读锁阻塞期间被更新，导致已经进入hashes的数据被重复比较
        auto lastInsertIndex = _insertHistory.size();
        if (auto iter = std::ranges::find_if(std::as_const(_hashes), [this, hash](const auto &item) {
            return checkSameImage(hash, item.first);
        }); iter != _hashes.end()) {
            foundSame = true;
            Q_EMIT sameImg(iter->second, index);
        }
        _hashesLock.unlock();

        if (foundSame) {
            Q_EMIT doneOneImg();
            continue;
        }

        _hashesLock.lockForWrite();
        if (auto iter = std::ranges::find_if(std::as_const(_insertHistory) | std::views::drop(lastInsertIndex), [this, hash](const ulong64 item) {
            return checkSameImage(hash, item);
        }); iter != _insertHistory.end()) {
            foundSame = true;
            Q_EMIT sameImg(_hashes[*iter], index);
        }
        if (!foundSame) {
            _hashes.emplace(std::make_pair(hash, index));
            _insertHistory.emplace_back(hash);
        }
        _hashesLock.unlock();
        Q_EMIT doneOneImg();
    }

    Q_EMIT doneAllWork();
}
