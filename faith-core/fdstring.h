#ifndef FAITHSTRING_H
#define FAITHSTRING_H

#include "faithdata.h"
#include <QString>
#include <QByteArray>

class FdString : public FaithData
{
    QString _data;
public:
    FdString(QString value=QString());

    // FaithData interface
public:
    QByteArray &rawDada() const;
    bool readRawData(const QByteArray &);
    QString value() const;
};

#endif // FAITHSTRING_H
