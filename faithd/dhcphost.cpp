#include "dhcphost.h"
#include "dhcpparameter.h"
#include "hostscontainer.h"
#include "faithcore.h"

QString DhcpHost::hw() const
{
    return _hw;
}

void DhcpHost::setHw(const QString &hw)
{
    _hw = hw;
}

quint32 DhcpHost::ip() const
{
    return _ip;
}

void DhcpHost::setIp(const quint32 &ip)
{
    _ip = ip;
}

void DhcpHost::append(DhcpObject *ob)
{
    DhcpParameter* par = dynamic_cast<DhcpParameter*>(ob);
    if (par)
    {
        if (par->value().startsWith("hardware ethernet "))
        {
            setHw(par->value().remove(0, 18).trimmed());
        }
        else if (par->value().startsWith("fixed-address "))
        {
            QString ip = par->value().remove(0, 13).trimmed();
            setIp(Faithcore::ipFromString(ip));
        }
        else if (par->value().startsWith("option host-name "))
        {
            QString tmp = par->value().remove(0, 17).trimmed();
            if (tmp.at(0)=='"' && tmp.at(tmp.length()-1)=='"')
            {
                tmp = tmp.remove(0,1);
                tmp = tmp.remove(tmp.length()-1,1);
                tmp = tmp.trimmed();
            }
            setHw(tmp);
        }
        else DhcpBlock::append(ob);
    }
    else DhcpBlock::append(ob);
}
void DhcpHost::addObserver(HostsContainer *observer)
{
    if (!observers.contains(observer))
        observers.append(observer);
}

DhcpHost::DhcpHost(QString hostname) : DhcpBlock(hostname.trimmed())
{
    _ip = 0;
}

DhcpHost::~DhcpHost()
{
    foreach (HostsContainer* obs, observers) {
        obs->removeHost(this);
    }
}

QString DhcpHost::toString(int level) const
{
    QString out=spaces(level)+"host "+getValue()+" {\n";
    out+=spaces((level+1))+"option host-name \""+hostname()+"\";\n";
    if (!hw().isEmpty()) out+=spaces((level+1))+"hardware ethernet "+hw()+";\n";
    if (ip()) out+=spaces((level+1))+"fixed-address "+Faithcore::ipFromInt(ip())+";\n";
    foreach (DhcpObject* ob, getChildren()) {
        out+=ob->toString(level+1)+"\n";
    }
    out += spaces(level)+"}";
    return out;
}

QString DhcpHost::hostname() const
{
    return getValue();
}

void DhcpHost::setHostname(const QString &hostname)
{
    setValue(hostname);
}
