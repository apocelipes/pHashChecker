#include "hashworker.h"
#include <iostream>
#include <QThread>
#include <QDebug>
#include <QReadWriteLock>
#include <QMutex>

constexpr int SMALLER_DISTANCE = 10;

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
        auto checkSameImage = [] (ulong64 a, ulong64 b) {
            return ph_hamming_distance(a, b) <= SMALLER_DISTANCE;
        };
        ph_dct_imagehash(_images[index].c_str(), hash);
        _hashesLock.lockForRead();
        for (const auto &[key, val]: _hashes) {
            if (checkSameImage(hash, key)) {
                isSameInHashes = true;
                Q_EMIT sameImg(_images[index], val);
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
            if (checkSameImage(hash, _insertHistory[i])) {
                isSameInNewInsert = true;
                Q_EMIT sameImg(_images[index], _hashes[_insertHistory[i]]);
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
