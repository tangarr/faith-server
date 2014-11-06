#include "dhcpblock.h"
#include "dhcphost.h"

QList<DhcpObject *> DhcpBlock::getChildren() const
{
    return children;
}

QString DhcpBlock::getValue() const
{
    return block;
}

void DhcpBlock::setValue(const QString value)
{
    block = value;
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

void DhcpBlock::appendList(QList<DhcpObject *>list)
{
    foreach (DhcpObject* child, list) {
        append(child);
    }
}

QList<DhcpHost *> DhcpBlock::getHosts()
{
    QList<DhcpHost *> list;
    foreach (DhcpObject* ob, children) {
        DhcpHost* host = dynamic_cast<DhcpHost*>(ob);
        if (host) list.append(host);
        else
        {
            DhcpBlock* block = dynamic_cast<DhcpBlock*>(ob);
            if (block) list.append(block->getHosts());
        }
    }
    return list;
}

void DhcpBlock::append(DhcpObject *child)
{
    children.append(child);
}
