#ifndef DHCPCOMMENT_H
#define DHCPCOMMENT_H

#include "dhcpobject.h"

class DhcpComment : public DhcpObject
{
    QString comment;
public:
    DhcpComment(QString comment);
    QString toString(int level) const;
};

#endif // DHCPCOMMENT_H
