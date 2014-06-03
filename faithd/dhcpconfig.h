#ifndef DHCPCONFIG_H
#define DHCPCONFIG_H

#include "dhcpobject.h"
#include "dhcphost.h"
#include <QList>
#include "hostscontainer.h"

class DhcpConfig : public HostsContainer
{
    QList<DhcpObject*> config;        
    DhcpConfig();
public:
    static DhcpConfig &instance();
    bool readConfiguration(QString filename);
    bool writeConfiguration(QString filename) const;
};

#endif // DHCPCONFIG_H
