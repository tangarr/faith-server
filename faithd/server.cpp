#include "server.h"
#include <QTcpSocket>
#include <QDebug>
#include "faithmessage.h"
#include "config.h"
#include "fdstring.h"

QString generate_hostname(ComputerLab *lab, quint32 ip)
{
    int bytes;
    for (bytes=4; bytes>=0; bytes--)
    {
        quint32 test = (0xFF) << ((bytes-1)*8);
        if ((lab->ip_start() & test) != (lab->ip_end() & test)) break;
    }
    QString hostname=lab->name();
    for (int i=bytes; i>1; i--)
    {
        quint32 b = (((0xFF) << ((bytes-1)*8)) & ip) >> ((bytes-1)*8);
        hostname += QString::number(b)+".";
    }
    quint32 last = ip & 0xff;
    hostname += QString::number(last);
    return hostname;
}

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
        socket->waitForReadyRead();
        FaithMessage msg;
        msg.recive(socket);

        qDebug() << "Message: " + Faithcore::MessageCodeToString(msg.getMessageCode());

        switch (msg.getMessageCode()) {
        case Faithcore::GET_LAB_LIST:
        {            
            FaithMessage resp = FaithMessage::MsgLabList(Config::instance().labList());
            resp.send(socket);
            break;
        }
        case Faithcore::RESERVE_IP:
        {
            FdString* value = static_cast<FdString*>(msg.getData());
            if (value)
            {
                ComputerLab *lab=Config::instance().getLab(value->value());
                if (!lab)
                {
                    FaithMessage::MsgError("Laboratory "+value->value()+" not found").send(socket);
                }
                else
                {
                    quint32 ip = lab->reserveIp(socket->peerAddress().toString());
                    if (!ip)
                    {
                        FaithMessage::MsgError("Computer Lab '"+lab->name()+"' has no free ip address").send(socket);
                    }
                    else
                    {
                        QString hostname = generate_hostname(lab, ip);
                        FaithMessage::MsgProposedIp(ip, hostname).send(socket);
                    }
                }
            }
            else
            {
                FaithMessage::MsgError("can't extract FaithString from data").send(socket);
            }
            break;
        }
        default:
            qDebug() << "Message " << Faithcore::MessageCodeToString(msg.getMessageCode()) << " not implemented";
            break;
        }
        socket->disconnectFromHost();
        if (socket->state()!=QTcpSocket::UnconnectedState) socket->waitForDisconnected();
    }
}
