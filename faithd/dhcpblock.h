#ifndef DHCPBLOCK_H
#define DHCPBLOCK_H

#include "dhcpobject.h"
#include <QList>
#include <QString>

class DhcpHost;

class DhcpBlock : public DhcpObject
{
protected:
    QList<DhcpObject*> getChildren() const;
    QString getValue() const;
    void setValue(const QString value);
private:
    QString block;
    QList<DhcpObject*> children;
public:
    DhcpBlock(QString block);
    virtual ~DhcpBlock();
    virtual QString toString(int level) const;
    virtual void append(DhcpObject*);
    void appendList(QList<DhcpObject *>);
    QList<DhcpHost *> getHosts();

};

#endif // DHCPBLOCK_H
