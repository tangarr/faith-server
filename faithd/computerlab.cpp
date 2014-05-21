#include "computerlab.h"
#include "faithcore.h"

ComputerLab::ComputerLab(QString name, QString ip_start, QString ip_end)
{
    _name = name;
    _ip_start = Faithcore::ipFromString(ip_start);
    _ip_end = Faithcore::ipFromString(ip_end);
}
