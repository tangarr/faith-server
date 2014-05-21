#ifndef FDPROPOSEDIP_H
#define FDPROPOSEDIP_H

#include "faithdata.h"

class FdProposedIp : public FaithData
{
    quint32 _ip;
    QString _hostname;
public:
    FdProposedIp(quint32 ip=0, QString hostname=QString());
    quint32 ip() const;
    QString hostname() const;

    // FaithData interface
public:
    QByteArray &rawDada() const;
    bool readRawData(const QByteArray &);
};

#endif // FDPROPOSEDIP_H
