#ifndef COMPUTERLAB_H
#define COMPUTERLAB_H

#include "hostscontainer.h"
#include <QDateTime>
#include <QHash>

struct ReservedIpInfo
{
    ReservedIpInfo(QString address);
    QDateTime time;
    QString address;
};


class ComputerLab : public HostsContainer
{
    QString _name;
    quint32 _ip_start, _ip_end;
    QHash<quint32, ReservedIpInfo> _reserved;

public:
    ComputerLab(QString name, QString ip_start, QString ip_end);    
    QString name() const;
    quint32 reserveIp(QString address);
    void releaseIp(quint32 ip);
    quint32 ip_start() const;
    quint32 ip_end() const;
};

#endif // COMPUTERLAB_H
