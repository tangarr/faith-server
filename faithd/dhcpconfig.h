#ifndef DHCPCONFIG_H
#define DHCPCONFIG_H

#include "dhcpobject.h"
#include "dhcphost.h"
#include <QList>

class DhcpConfig
{
    QList<DhcpObject*> config;
    QList<DhcpHost*> hosts;
public:
    DhcpConfig();
    bool readConfiguration(QString filename);
    bool writeConfiguration(QString filename) const;
    bool containsHostname(const QString &hostname) const;
    bool containsIp(const QString &ip) const;
    bool containHw(const QString &hw) const;
};

#endif // DHCPCONFIG_H
