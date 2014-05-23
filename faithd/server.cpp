#include "server.h"
#include <QTcpSocket>
#include <QDebug>
#include "faithmessage.h"
#include "config.h"

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

        switch (msg.getMessageCode()) {
        case Faithcore::GET_LAB_LIST:
        {
            qDebug() << "Message: GET_LAB_LIST";
            FaithMessage resp = FaithMessage::MsgLabList(Config::instance().labList());
            resp.send(socket);
            break;
        }
        default:
            qDebug() << "Message " << msg.getMessageCode() << " not implemented";
            break;
        }

        socket->waitForDisconnected();
    }
}
