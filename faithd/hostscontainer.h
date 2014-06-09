#ifndef HOSTSCONTAINER_H
#define HOSTSCONTAINER_H

#include <QList>
#include "dhcphost.h"

class HostsContainer
{
private:
    QList<DhcpHost*> _hosts;
protected:
    void clearHostsList();
    void destroyHosts();
public:
    HostsContainer();
    bool containsHostname(const QString &hostname) const;
    bool containsIp(const QString &ip) const;
    bool containsIp(const quint32 &ip) const;
    bool containHw(const QString &hw) const;
    DhcpHost *hostByIp(const QString &ip) const;
    DhcpHost *hostByHw(const QString &hw) const;
    DhcpHost *hostByName(const QString &hostname) const;
    void appendHost(DhcpHost* host);
    void appendHost(QList<DhcpHost*> host);
    void removeHost(DhcpHost* host);
    QList<DhcpHost *> hosts() const;
};

#endif // HOSTSCONTAINER_H
