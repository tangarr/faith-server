#include "dhcpblock.h"

QList<DhcpObject *> DhcpBlock::getChildren() const
{
    return children;
}

QString DhcpBlock::getValue() const
{
    return block;
}

DhcpBlock::DhcpBlock(QString block)
{
    this->block=block;
}

DhcpBlock::~DhcpBlock()
{
    foreach (DhcpObject* ob, children) {
        delete ob;
    }
    children.clear();
}

QString DhcpBlock::toString(int level) const
{
    QString out = spaces(level) + block + "{\n";
    foreach (DhcpObject* ob, children) {
        out += ob->toString(level+1)+"\n";
    }
    out += spaces(level) + "}";
    return out;
}

void DhcpBlock::append(QList<DhcpObject *>list)
{
    children.append(list);
}

void DhcpBlock::append(DhcpObject *child)
{
    children.append(child);
}
