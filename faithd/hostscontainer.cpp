#include "hostscontainer.h"
#include <QRegExp>
#include <QStringList>

void HostsContainer::clearHostsList()
{
    hosts.clear();
}

void HostsContainer::destroyHosts()
{
    foreach (DhcpHost* h, hosts) {
        delete h;
    }
}

HostsContainer::HostsContainer()
{
}

bool HostsContainer::containsHostname(const QString &hostname) const
{
    foreach (DhcpHost* host, hosts) {
        if (host->hostname().compare(hostname, Qt::CaseInsensitive)) return true;
    }
    return false;
}

bool HostsContainer::containsIp(const QString &ip) const
{
    const QRegExp regexp("(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9]\\.){3}(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])");
    if (!regexp.exactMatch(ip)) return false;
    foreach (DhcpHost* host, hosts) {
        QStringList a,b;
        a = ip.split(".");
        b = host->ip().split(".");
        for (int i=0; i<4; i++)
        {
            if (a[i].toInt()!=b[i].toInt()) return false;
        }
        return true;
    }
    return false;
}

bool HostsContainer::containHw(const QString &hw) const
{
    const QRegExp regexp("([0-9a-f]:){5}([0-9a-f])");
    if (!regexp.exactMatch(hw)) return false;
    foreach (DhcpHost* host, hosts) {
        QStringList a,b;
        a = hw.split(":");
        b = host->hw().split(":");
        for (int i=0; i<6; i++)
        {
            if (a[i].toInt(0,16)!=b[i].toInt(0,16)) return false;
        }
        return true;
    }
    return false;
}

DhcpHost *HostsContainer::hostByIp(const QString &ip) const
{
    const QRegExp regexp("(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9]\\.){3}(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])");
    if (!regexp.exactMatch(ip)) return 0;
    foreach (DhcpHost* host, hosts) {
        QStringList a,b;
        a = ip.split(".");
        b = host->ip().split(".");
        for (int i=0; i<4; i++)
        {
            if (a[i].toInt()!=b[i].toInt()) return host;
        }
        return 0;
    }
    return 0;

}

DhcpHost *HostsContainer::hostByHw(const QString &hw) const
{
    const QRegExp regexp("([0-9a-f]:){5}([0-9a-f])");
    if (!regexp.exactMatch(hw)) return 0;
    foreach (DhcpHost* h, hosts) {
        QStringList a,b;
        a = hw.split(":");
        b = h->hw().split(":");
        for (int i=0; i<6; i++)
        {
            if (a[i].toInt(0,16)!=b[i].toInt(0,16)) return 0;
        }
        return h;
    }
    return 0;
}

DhcpHost *HostsContainer::hostByName(const QString &hostname) const
{
    foreach (DhcpHost* host, hosts) {
        if (host->hostname().compare(hostname, Qt::CaseInsensitive)) return host;
    }
    return 0;
}

void HostsContainer::appendHost(DhcpHost *host)
{
    hosts.append(host);
}

void HostsContainer::appendHost(QList<DhcpHost *> host)
{
    hosts.append(host);
}