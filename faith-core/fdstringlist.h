#ifndef FDSTRINGLIST_H
#define FDSTRINGLIST_H

#include "QStringList"
#include "faithdata.h"

class FdStringList : public FaithData
{
    QStringList _data;

public:
    FdStringList(QStringList list=QStringList());
    int count() const;
    QString at(const int index) const;

    // FaithData interface
public:
    QByteArray &rawDada() const;
    bool readRawData(const QByteArray &);
};

#endif // FDSTRINGLIST_H
