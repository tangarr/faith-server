#ifndef DHCPHOST_H
#define DHCPHOST_H

#include "dhcpblock.h"
#include "hostscontainer.h"

class HostsContainer;

class DhcpHost : public DhcpBlock
{
private:
    QString _hw;
    quint32 _ip;
    QList<HostsContainer*> observers;
    void addObserver(HostsContainer* observer);
public:
    DhcpHost(QString hostname);
    ~DhcpHost();
    virtual QString toString(int level) const;
    QString hostname() const;
    void setHostname(const QString &hostname);
    QString hw() const;
    void setHw(const QString &hw);
    quint32 ip() const;
    void setIp(const quint32 &ip);
    void append(DhcpObject *ob);

    friend class HostsContainer;

    QList<DhcpHost *> getHosts();
};

#endif // DHCPHOST_H
