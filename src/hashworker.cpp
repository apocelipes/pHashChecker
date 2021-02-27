#include "hashworker.h"
#include <iostream>
#include <QThread>
#include <QDebug>
#include <QReadWriteLock>
#include <QMutex>

constexpr int SMALLER_DISTANCE = 10;

namespace {
    inline bool checkSameImage(ulong64 a, ulong64 b, bool &flag) {
        if (ph_hamming_distance(a, b) <= SMALLER_DISTANCE) {
            flag = true;
            return true;
        }

        return false;
    }
}

void HashWorker::doWork()
{
    for (unsigned long index = _start; index < _limit; ++index) {
        if (this->thread()->isInterruptionRequested()) {
            qInfo() << "thread exit!";
            break;
        }

        _insertHistoryLock.lock();
        auto lastInsertIndex = _insertHistory.size();
        _insertHistoryLock.unlock();
        ulong64 hash = 0;
        bool isSameInHashes = false;
        ph_dct_imagehash(_images[index].c_str(), hash);
        _hashesLock.lockForRead();
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
        _insertHistoryLock.lock();
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
        _insertHistoryLock.unlock();
        Q_EMIT doneOneImg();
    }

    Q_EMIT doneAllWork();
}
