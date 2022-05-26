// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2022 apocelipes

#include <QThread>
#include <QDebug>
#include <QReadWriteLock>

#include "hashworker.h"

Utils::PHashDistance HashWorker::similar_distance = Utils::PHashDistance::FUZZY;

void HashWorker::doWork()
{
    for (unsigned long index = _start; index < _limit; ++index) {
        if (this->thread()->isInterruptionRequested()) {
            qInfo() << "thread exit!";
            break;
        }

        ulong64 hash = 0;
        bool isSameInHashes = false;
        ph_dct_imagehash(_images[index].c_str(), hash);
        _hashesLock.lockForRead();
        // 获得读锁后即为最新的size
        // 在获取读锁之前取得size，size可能会在读锁阻塞期间被更新，导致已经进入hashes的数据被重复比较
        auto lastInsertIndex = _insertHistory.size();
        for (const auto &[key, val]: _hashes) {
            if (checkSameImage(hash, key, isSameInHashes)) {
                isSameInHashes = true;
                // origin必须为已经存在于hashes里的图片
                Q_EMIT sameImg(val, _images[index]);
                break;
            }
        }
        _hashesLock.unlock();

        if (isSameInHashes) {
            Q_EMIT doneOneImg();
            continue;
        }

        auto isSameInNewInsert = false;
        _hashesLock.lockForWrite();
        for (auto i = lastInsertIndex; i < _insertHistory.size(); ++i) {
            if (checkSameImage(hash, _insertHistory[i], isSameInNewInsert)) {
                isSameInNewInsert = true;
                Q_EMIT sameImg(_hashes[_insertHistory[i]], _images[index]);
                break;
            }
        }
        if (!isSameInNewInsert) {
            _hashes.emplace(std::make_pair(hash, _images[index]));
            _insertHistory.emplace_back(hash);
        }
        _hashesLock.unlock();
        Q_EMIT doneOneImg();
    }

    Q_EMIT doneAllWork();
}
