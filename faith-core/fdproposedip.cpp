#include "fdproposedip.h"
#include <QByteArray>
#include <QDataStream>

FdProposedIp::FdProposedIp(quint32 ip, QString hostname)
{
    _ip = ip;
    _hostname = hostname;
}

quint32 FdProposedIp::ip() const
{
    return _ip;
}

QString FdProposedIp::hostname() const
{
    return _hostname;
}

QByteArray &FdProposedIp::rawDada() const
{
    QByteArray *raw=new QByteArray();
    QDataStream stream(raw, QIODevice::WriteOnly);
    stream << _ip << _hostname;
    return *raw;
}

bool FdProposedIp::readRawData(const QByteArray &raw)
{
    QDataStream stream(raw);
    stream >> _ip >> _hostname;
    return true;
}
