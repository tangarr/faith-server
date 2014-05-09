#ifndef DHCPOBJECT_H
#define DHCPOBJECT_H

#include <QString>

class DhcpObject
{
protected:
    QString spaces(int level) const;
public:
    DhcpObject();
    virtual ~DhcpObject();
    virtual QString toString(int level) const=0;
};

#endif // DHCPOBJECT_H
