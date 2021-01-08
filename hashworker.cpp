#include "hashworker.h"
#include <iostream>
#include <QThread>
#include <QDebug>

void HashWorker::doWork()
{
    for (unsigned long index = _start; index < _limit; ++index) {
        if (this->thread()->isInterruptionRequested()) {
            qInfo() << "thread exit!";
            break;
        }
        ulong64 hash = 0;
        bool flag = false;
        ph_dct_imagehash(_images[index].c_str(), hash);
        _lock.lockForRead();
        for (const auto &[key, val]: _hashes) {
            if (ph_hamming_distance(key, hash) <= SMALLER_DISTANCE) {
                flag = true;
                Q_EMIT sameImg(_images[index], val);
                break;
            }
        }
        _lock.unlock();
        if (!flag) {
            _lock.lockForWrite();
            _hashes.emplace(std::make_pair(hash, _images[index]));
            _lock.unlock();
        }
        Q_EMIT doneOneImg();
    }
    Q_EMIT doneAllWork();
}
