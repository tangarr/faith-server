#include "dhcpobject.h"

DhcpObject::DhcpObject()
{
}

DhcpObject::~DhcpObject()
{
}

QString DhcpObject::spaces(int level) const
{
    QString out="";
    for (int i=0; i<level; i++) out+="\t";
    return out;
}
