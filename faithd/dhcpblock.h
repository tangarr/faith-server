#ifndef DHCPBLOCK_H
#define DHCPBLOCK_H

#include "dhcpobject.h"
#include <QList>
#include <QString>

class DhcpBlock : public DhcpObject
{
protected:
    QList<DhcpObject*> getChildren() const;
    QString getValue() const;
private:
    QString block;
    QList<DhcpObject*> children;
public:
    DhcpBlock(QString block);
    virtual ~DhcpBlock();
    virtual QString toString(int level) const;
    virtual void append(DhcpObject*);
    virtual void append(QList<DhcpObject *>);
};

#endif // DHCPBLOCK_H
