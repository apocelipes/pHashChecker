#ifndef HASHWORKER_H
#define HASHWORKER_H

#include <QObject>

#include <string>
#include <vector>
#include <unordered_map>

#include <pHash.h>

class QReadWriteLock;


enum class PHashDistance: int
{
    STRICT  = 1,
    PRECISE = 5,
    DEFAULT = 8,
    FUZZY   = 10,
};

class HashWorker : public QObject
{
    Q_OBJECT
public:
    using ContainerType = const std::vector<std::string>;
    using HashContainerType = std::unordered_map<ulong64, std::string>;

    HashWorker(unsigned long start,
               unsigned long limit,
               ContainerType &c,
               HashContainerType &hashes,
               std::vector<ulong64> &insertHistory,
               QReadWriteLock &lock,
               QObject *parent = nullptr)
        : QObject(parent),
          _start{start},
          _limit{limit},
          _images{c},
          _hashes{hashes},
          _insertHistory{insertHistory},
          _hashesLock{lock}
    {}

    void doWork();

    static PHashDistance similar_distance;

Q_SIGNALS:
    void sameImg(const std::string&, const std::string&);
    void doneOneImg();
    void doneAllWork();

private:
    unsigned long _start{};
    unsigned long _limit{};
    ContainerType &_images;
    HashContainerType &_hashes;
    std::vector<ulong64> &_insertHistory;
    QReadWriteLock &_hashesLock;

    bool checkSameImage(ulong64 a, ulong64 b, bool &flag) noexcept
    {
        if (ph_hamming_distance(a, b) <= static_cast<int>(similar_distance)) {
            flag = true;
            return true;
        }

        return false;
    }
};

#endif // HASHWORKER_H
