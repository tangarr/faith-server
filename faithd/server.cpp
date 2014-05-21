#include "server.h"
#include <QTcpSocket>
#include <QDebug>
#include "faithmessage.h"

Server::Server(quint16 port)
{
    _server.listen(QHostAddress::AnyIPv4, port);
}

void Server::acceptConnection()
{
    bool newConnection = _server.waitForNewConnection(-1);
    if (newConnection)
    {
        QTcpSocket* socket = _server.nextPendingConnection();
        qDebug() << socket->peerAddress() << " connected";
        FaithMessage msg;
        msg.recive(socket);
        qDebug() << msg.getMessageCode();
        socket->close();
    }
}
