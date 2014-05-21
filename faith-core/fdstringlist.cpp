#include "fdstringlist.h"
#include <QByteArray>
#include <QDataStream>

FdStringList::FdStringList(QStringList list)
{
    _data = list;
}

int FdStringList::count() const
{
    return _data.count();
}

QString FdStringList::at(const int index) const
{
    return _data.at(index);
}

QByteArray &FdStringList::rawDada() const
{
    QByteArray *raw = new QByteArray();
    QDataStream stream(raw, QIODevice::WriteOnly);
    stream << _data;
    return *raw;
}

bool FdStringList::readRawData(const QByteArray &raw)
{
    QDataStream stream(raw);
    stream >> _data;
    return false;
}
