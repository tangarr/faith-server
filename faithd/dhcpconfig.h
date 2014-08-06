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
    QString _current_dhcp_file;
public:
    static DhcpConfig &instance();
    bool readConfiguration(QString filename);
    bool writeConfiguration(QString filename=QString()) const;
    QString current_dhcp_file() const;
    //void appendHost(DhcpHost *host);
};

#endif // DHCPCONFIG_H
