#ifndef DHCPEMPTYLINE_H
#define DHCPEMPTYLINE_H

#include "dhcpobject.h"

class DhcpEmptyLine : public DhcpObject
{
public:
    DhcpEmptyLine();
    virtual QString toString(int) const;
};

#endif // DHCPEMPTYLINE_H
