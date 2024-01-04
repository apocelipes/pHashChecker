// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2024 apocelipes

#include <QThread>
#include <QDebug>
#include <QReadWriteLock>

#include <algorithm>
#include <ranges>
#include <utility>

#include "hashworker.h"

Utils::PHashDistance HashWorker::similar_distance = Utils::PHashDistance::FUZZY;

void HashWorker::doWork()
{
    for (size_t index = _start; index < _limit; ++index) {
        if (this->thread()->isInterruptionRequested()) {
            qInfo() << "thread exit!";
            break;
        }

        ulong64 hash = 0;
        bool isSameInHashes = false;
        ph_dct_imagehash(_images[index].c_str(), hash); //TODO error check
        _hashesLock.lockForRead();
        // 获得读锁后即为最新的size
        // 在获取读锁之前取得size，size可能会在读锁阻塞期间被更新，导致已经进入hashes的数据被重复比较
        auto lastInsertIndex = _insertHistory.size();
        if (auto iter = std::ranges::find_if(std::as_const(_hashes), [hash](const auto &item) {
            return HashWorker::checkSameImage(hash, item.first);
        }); iter != _hashes.end()) {
            isSameInHashes = true;
            Q_EMIT sameImg(iter->second, index);
        }
        _hashesLock.unlock();

        if (isSameInHashes) {
            Q_EMIT doneOneImg();
            continue;
        }

        auto isSameInNewInsert = false;
        _hashesLock.lockForWrite();
        if (auto iter = std::ranges::find_if(std::as_const(_insertHistory) | std::views::drop(lastInsertIndex), [hash](const ulong64 item) {
            return HashWorker::checkSameImage(hash, item);
        }); iter != _insertHistory.end()) {
            isSameInHashes = true;
            Q_EMIT sameImg(_hashes[*iter], index);
        }
        if (!isSameInNewInsert) {
            _hashes.emplace(std::make_pair(hash, index));
            _insertHistory.emplace_back(hash);
        }
        _hashesLock.unlock();
        Q_EMIT doneOneImg();
    }

    Q_EMIT doneAllWork();
}
