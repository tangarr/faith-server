#ifndef DHCPBLOCK_H
#define DHCPBLOCK_H

#include "dhcpobject.h"
#include <QList>
#include <QString>

class DhcpBlock : public DhcpObject
{
private:
    QString block;
    QList<DhcpObject*> children;
public:
    DhcpBlock();
    virtual ~DhcpBlock();
    virtual QString toString(int level);
};

#endif // DHCPBLOCK_H
