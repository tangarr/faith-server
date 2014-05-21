#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>

class Server
{
    QTcpServer _server;
public:
    Server(quint16 port);
    void acceptConnection();

};

#endif // SERVER_H
