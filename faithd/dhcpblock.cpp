#include "dhcpblock.h"

DhcpBlock::DhcpBlock()
{
}

DhcpBlock::~DhcpBlock()
{
    foreach (DhcpObject* ob, children) {
        delete ob;
    }
    children.clear();
}

QString DhcpBlock::toString(int level)
{
    QString out = spaces(level) + block + "{\n";
    foreach (DhcpObject* ob, children) {
        out += ob->toString(level+1)+"\n";
    }
    out += spaces(level) + "}";
    return out;
}
