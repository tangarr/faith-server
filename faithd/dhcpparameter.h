#ifndef DHCPPARAMETER_H
#define DHCPPARAMETER_H

#include "dhcpobject.h"

class DhcpParameter : public DhcpObject
{
private:
    QString parameter;
public:
    DhcpParameter();
    QString toString(int level);
};

#endif // DHCPPARAMETER_H
