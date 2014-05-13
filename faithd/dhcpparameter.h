#ifndef DHCPPARAMETER_H
#define DHCPPARAMETER_H

#include "dhcpobject.h"

class DhcpParameter : public DhcpObject
{
private:
    QString parameter;
public:
    DhcpParameter(QString parameter);
    QString toString(int level) const;
    QString value() const;
};

#endif // DHCPPARAMETER_H
