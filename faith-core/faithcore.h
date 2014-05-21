#ifndef FAITHCORE_H
#define FAITHCORE_H

#include <QtCore>
#include <QString>

class Faithcore
{

public:
    Faithcore();
    enum MessageCode
    {        
        GET_LAB_LIST,
        LAB_LIST,
        RESERVE_IP,
        PROPOSED_IP,
        OK,
        ERROR,
    };
    static quint32 ipFromString(const QString &ip);
    static QString ipFromInt(const quint32 &ip);

};

#endif // FAITHCORE_H
