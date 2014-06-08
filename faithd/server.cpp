#include "server.h"
#include <QTcpSocket>
#include <QDebug>
#include "faithmessage.h"
#include "config.h"
#include "fdstring.h"
#include "fdhostinfo.h"
#include "dhcpconfig.h"

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
        case Faithcore::GET_LAB_LIST_OR_HOST_INFO:
        {
            FdString* value = static_cast<FdString*>(msg.getData());
            if (value)
            {
                ComputerLab* lab = 0;
                QString mac = value->value();
                foreach (QString lab_name, Config::instance().labList()) {
                    ComputerLab* tmp = Config::instance().getLab(lab_name);
                    if (tmp->containHw(mac))
                    {
                        lab = tmp;
                        break;
                    }
                }
                if (lab)
                {
                    DhcpHost* host = lab->hostByHw(mac);
                    FaithMessage resp = FaithMessage::MsgHostInfo(lab->name(), host->hostname(), host->ip());
                    resp.send(socket);
                }
                else
                {
                    FaithMessage resp = FaithMessage::MsgLabList(Config::instance().labList());
                    resp.send(socket);
                }
            }
            else
            {
                FaithMessage::MsgError("can't extract FaithString from data").send(socket);
            }
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
        case Faithcore::ACCEPT_IP:
        {
            FdHostInfo *hinfo = static_cast<FdHostInfo*>(msg.getData());
            if (hinfo)
            {                
                DhcpHost* host_by_hw, *host_by_ip, *host_by_hostname;
                host_by_hw = DhcpConfig::instance().hostByHw(hinfo->mac());
                host_by_ip = DhcpConfig::instance().hostByIp(hinfo->ip_string());
                host_by_hostname = DhcpConfig::instance().hostByIp(hinfo->ip_string());

                ComputerLab *lab = Config::instance().getLab(hinfo->lab());
                if (!lab)
                {
                    FaithMessage::MsgError("specified ComputerLab don't exist").send(socket);
                    break;
                }
                if (hinfo->ip() < lab->ip_start() || hinfo->ip() > lab->ip_end())
                {
                    FaithMessage::MsgError("selected ip address don't belong to specified ComputerLab").send(socket);
                    break;
                }
                if (host_by_hw)
                {
                    if (host_by_hw!=host_by_ip)
                    {
                        FaithMessage::MsgError("you can't change ip from this application").send(socket);
                        break;
                    }
                    if (host_by_hw!=host_by_hostname)
                    {
                        FaithMessage::MsgError("you can't change hostname from this application").send(socket);
                        break;
                    }
                }
                else
                {
                    if (host_by_ip)
                    {
                        FaithMessage::MsgError("ip already taken by diferent host").send(socket);
                        break;
                    }
                    if (host_by_hostname)
                    {
                        FaithMessage::MsgError("hostname already taken by diferent host").send(socket);
                        break;
                    }
                    DhcpHost* host = new DhcpHost(hinfo->hostname());
                    host->setHw(hinfo->mac());
                    host->setIp(hinfo->ip());
                    DhcpConfig::instance().appendHost(host);
                    lab->appendHost(host);
                }
                FaithMessage::MsgOk().send(socket);
            }
            else
            {
                FaithMessage::MsgError("can't extract HostInfo from data").send(socket);
            }
        }
        default:
            qDebug() << "Message " << Faithcore::MessageCodeToString(msg.getMessageCode()) << " not implemented";
            FaithMessage::MsgError("Message " + Faithcore::MessageCodeToString(msg.getMessageCode()) + " not implemented").send(socket);
            break;
        }
        socket->disconnectFromHost();
        if (socket->state()!=QTcpSocket::UnconnectedState) socket->waitForDisconnected();
    }
}
