#ifndef DHCPCONFIG_H
#define DHCPCONFIG_H

#include "dhcpobject.h"
#include <QList>

class DhcpConfig
{
    QList<DhcpObject*> config;
public:
    DhcpConfig();
    bool readConfiguration(QString filename);
    bool writeConfiguration(QString filename) const;
};

#endif // DHCPCONFIG_H
