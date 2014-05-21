#ifndef FAITHOBJECT_H
#define FAITHOBJECT_H

#include "faithcore.h"
#include <QByteArray>

class FaithData
{
public:
    FaithData() {}
    virtual ~FaithData() {}
    virtual QByteArray& rawDada() const = 0;
    virtual bool readRawData(const QByteArray&) = 0;
};

#endif // FAITHOBJECT_H
