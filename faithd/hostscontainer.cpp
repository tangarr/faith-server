#include "hostscontainer.h"
#include <QRegExp>
#include <QStringList>
#include "dhcphost.h"
#include "faithcore.h"

void HostsContainer::clearHostsList()
{
    _hosts.clear();
}

void HostsContainer::destroyHosts()
{
    foreach (DhcpHost* h, _hosts) {
        delete h;
    }
}

HostsContainer::HostsContainer()
{
}

bool HostsContainer::containsHostname(const QString &hostname) const
{
    foreach (DhcpHost* host, _hosts) {
        if (host->hostname().compare(hostname, Qt::CaseInsensitive)) return true;
    }
    return false;
}

bool HostsContainer::containsIp(const QString &ip) const
{
    const QRegExp regexp("(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9]\\.){3}(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])");
    if (!regexp.exactMatch(ip)) return false;
    quint32 ip_a = Faithcore::ipFromString(ip);
    foreach (DhcpHost* host, _hosts) {
        if (ip_a == host->ip()) return true;
    }
    return false;
}

bool HostsContainer::containsIp(const quint32 &ip) const
{
    foreach (DhcpHost* host, _hosts) {
        if (ip == host->ip()) return true;
    }
    return false;
}

bool HostsContainer::containHw(const QString &hw) const
{
    foreach (DhcpHost* host, _hosts) {
        if (host->hw()==hw) return true;
    }
    return false;
}

DhcpHost *HostsContainer::hostByIp(const QString &ip) const
{
    const QRegExp regexp("(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9]\\.){3}(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])");
    if (!regexp.exactMatch(ip)) return 0;
    quint32 ip_a = Faithcore::ipFromString(ip);
    foreach (DhcpHost* host, _hosts) {
        if (ip_a == host->ip()) return host;
    }
    return 0;
}

DhcpHost *HostsContainer::hostByHw(const QString &hw) const
{
    foreach (DhcpHost* h, _hosts) {
        if (h->hw()==hw) return h;
    }
    return 0;
}

DhcpHost *HostsContainer::hostByName(const QString &hostname) const
{
    foreach (DhcpHost* host, _hosts) {
        if (host->hostname().compare(hostname, Qt::CaseInsensitive)) return host;
    }
    return 0;
}

void HostsContainer::appendHost(DhcpHost *host)
{
    host->addObserver(this);
    _hosts.append(host);
}

void HostsContainer::appendHostList(QList<DhcpHost *> hostsList)
{
    foreach (DhcpHost* host, hostsList) {
        appendHost(host);
    }
}

void HostsContainer::removeHost(DhcpHost *host)
{
    _hosts.removeAll(host);
}

QList<DhcpHost *> HostsContainer::hosts() const
{
    return _hosts;
}
