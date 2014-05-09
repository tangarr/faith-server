#ifndef DHCPHOST_H
#define DHCPHOST_H

#include "dhcpblock.h"

class DhcpHost : public DhcpBlock
{
public:
    DhcpHost(QString hostname);
    virtual QString toString(int level) const;
};

#endif // DHCPHOST_H
