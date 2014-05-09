#include "dhcphost.h"

DhcpHost::DhcpHost(QString hostname) : DhcpBlock(hostname)
{
}

QString DhcpHost::toString(int level) const
{
    QString out=spaces(level)+"host "+getValue()+" {\n";
    foreach (DhcpObject* ob, getChildren()) {
        out+=ob->toString(level+1)+"\n";
    }
    out += spaces(level)+"}";
    return out;
}
