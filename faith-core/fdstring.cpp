#include "fdstring.h"

FdString::FdString(QString value)
{
    _data=value;
}

QByteArray &FdString::rawDada() const
{
    QByteArray* array = new QByteArray(_data.toUtf8());
    return *array;
}

bool FdString::readRawData(const QByteArray &array)
{
    _data= QString::fromUtf8(array);
    return true;
}

QString FdString::value() const
{
    return _data;
}
