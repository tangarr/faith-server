#include "computerlab.h"
#include "faithcore.h"

ComputerLab::ComputerLab(QString name, QString ip_start, QString ip_end)
{
    _name = name;
    _ip_start = Faithcore::ipFromString(ip_start);
    _ip_end = Faithcore::ipFromString(ip_end);
}

QString ComputerLab::name() const
{
    return _name;
}

quint32 ComputerLab::reserveIp(QString address)
{
    QHashIterator<quint32, ReservedIpInfo> iter(_reserved);

    quint32 delete_key=0;
    while(iter.hasNext())
    {
        iter.next();
        if (delete_key)
        {
            _reserved.remove(delete_key);
            delete_key = 0;
        }
        if (iter.value().address==address)
        {
            return iter.key();
        }
        else
        {
            quint64 seconds = iter.value().time.secsTo(QDateTime::currentDateTime());
            if (seconds>60) delete_key = iter.key();
        }
    }
    if (delete_key)  _reserved.remove(delete_key);
    for (quint32 ip=_ip_start; ip<_ip_end; ip++)
    {
        if (this->containsIp(ip)) continue;
        if (_reserved.contains(ip)) continue;
        return ip;
    }
    return 0;
}

void ComputerLab::releaseIp(quint32 ip)
{
    _reserved.remove(ip);
}

quint32 ComputerLab::ip_start() const
{
    return _ip_start;
}

quint32 ComputerLab::ip_end() const
{
    return _ip_end;
}


ReservedIpInfo::ReservedIpInfo(QString address)
{
    this->address = address;
    time = QDateTime::currentDateTime();
}
