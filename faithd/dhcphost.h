#ifndef DHCPHOST_H
#define DHCPHOST_H

#include "dhcpblock.h"

class DhcpHost : public DhcpBlock
{
    QString _hw, _ip;
public:
    DhcpHost(QString hostname);
    virtual QString toString(int level) const;
    QString hostname() const;
    void setHostname(const QString &hostname);
    QString hw() const;
    void setHw(const QString &hw);
    QString ip() const;
    void setIp(const QString &ip);
    void append(DhcpObject *ob);
};

#endif // DHCPHOST_H
