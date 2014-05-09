#ifndef DHCPCOMMENT_H
#define DHCPCOMMENT_H

#include "dhcpobject.h"

class DhcpComment : public DhcpObject
{
    QString comment;
public:
    DhcpComment();
    QString toString(int level);
};

#endif // DHCPCOMMENT_H
