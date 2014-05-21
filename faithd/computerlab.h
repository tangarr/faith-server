#ifndef COMPUTERLAB_H
#define COMPUTERLAB_H

#include "hostscontainer.h"

class ComputerLab : public HostsContainer
{
    QString _name;
    quint32 _ip_start, _ip_end;
public:
    ComputerLab(QString name, QString ip_start, QString ip_end);    
};

#endif // COMPUTERLAB_H
