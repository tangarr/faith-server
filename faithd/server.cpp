#include "server.h"
#include <QTcpSocket>
#include <QDebug>
#include "faithmessage.h"
#include "config.h"
#include "fdstring.h"
#include "fdhostinfo.h"
#include "dhcpconfig.h"
#include "fdfile.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryFile>
#include <QSqlError>
#include <QHash>

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
    QSqlDatabase::addDatabase("QSQLITE");
}

void AcceptMessageSendFile(FaithMessage &msg, QTcpSocket* socket)
{
    FdFile* file = static_cast<FdFile*>(msg.getData());
    if (file)
    {
        if (file->data().isEmpty())
        {
            FaithMessage::MsgError("Recived FdFile have no data").send(socket);
            return;
        }
        else if (file->filename().isEmpty())
        {
            FaithMessage::MsgError("Recived FdFile have no filename").send(socket);
            return;
        }
        if (file->filename()!="host.db")
        {
            FaithMessage::MsgError("Server was expecting to recive file host.db").send(socket);
            return;
        }
        QTemporaryFile tmp_file;
        tmp_file.open();
        tmp_file.close();
        file->saveFile(tmp_file.fileName());
        QSqlDatabase db = QSqlDatabase::database();
        db.setDatabaseName(tmp_file.fileName());
        if (db.open())
        {
            QHash<QString, QString> general;
            QSqlQuery query("select * from general");
            while (query.next())
            {
                general.insert(query.value(0).toString(), query.value(1).toString());
            }
            db.close();

            DhcpHost *host = 0;
            if (general.contains("ip"))
            {
                host = DhcpConfig::instance().hostByIp(general["ip"]);
                if (!host)
                {
                    FaithMessage::MsgError("Host configuration doesn't contain valid ip value").send(socket);
                    return;
                }
            }
            else
            {
                FaithMessage::MsgError("Host configuration doesn't contain ip value").send(socket);
                return;
            }
            if (general.contains("mac"))
            {
                if (host->hw()!=general["mac"])
                {
                    FaithMessage::MsgError("Host configuration mac address don't match to host with selected ip address").send(socket);
                    return;
                }
            }
            else
            {
                FaithMessage::MsgError("Host configuration doesn't contain mac value").send(socket);
                return;
            }
            if (general.contains("hostname"))
            {
                if (host->hostname()!=general["hostname"])
                {
                    FaithMessage::MsgError("Host configuration hostname don't match to host with selected ip address").send(socket);
                    return;
                }
            }
            else
            {
                FaithMessage::MsgError("Host configuration doesn't contain hostname value").send(socket);
                return;
            }
            quint32 ip = host->ip();
            QString new_filename = QString::number(ip, 16).rightJustified(8, '0')+".db";
            bool success = file->saveFile(Config::instance().configDir()+"/"+new_filename);
            if (success) FaithMessage::MsgOk().send(socket);
            else FaithMessage::MsgError("SERVER ERROR: Can't save file in config_dir").send(socket);
        }
        else
        {
            FaithMessage::MsgError("Can't open host configuration database\n"+db.lastError().text()).send(socket);
        }
    }
    else
    {
        FaithMessage::MsgError("can't extract FdFile from data").send(socket);
    }
}

void AcceptMessageAcceptIP(FaithMessage &msg, QTcpSocket* socket)
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
            return;
        }
        if (hinfo->ip() < lab->ip_start() || hinfo->ip() > lab->ip_end())
        {
            FaithMessage::MsgError("selected ip address don't belong to specified ComputerLab").send(socket);
            return;
        }
        if (host_by_hw)
        {
            if (host_by_hw!=host_by_ip)
            {
                FaithMessage::MsgError("you can't change ip from this application").send(socket);
                return;
            }
            if (host_by_hw!=host_by_hostname)
            {
                FaithMessage::MsgError("you can't change hostname from this application").send(socket);
                return;
            }
        }
        else
        {
            if (host_by_ip)
            {
                FaithMessage::MsgError("ip already taken by diferent host").send(socket);
                return;
            }
            if (host_by_hostname)
            {
                FaithMessage::MsgError("hostname already taken by diferent host").send(socket);
                return;
            }
            DhcpHost* host = new DhcpHost(hinfo->hostname());
            host->setHw(hinfo->mac());
            host->setIp(hinfo->ip());
            DhcpConfig::instance().appendHost(host);
            lab->appendHost(host);
            bool succes = DhcpConfig::instance().writeConfiguration();
            if (succes) qDebug() << DhcpConfig::instance().current_dhcp_file() << " written";
            else qDebug() << "can't write " << DhcpConfig::instance().current_dhcp_file();
        }
        FaithMessage::MsgOk().send(socket);
    }
    else
    {
        FaithMessage::MsgError("can't extract HostInfo from data").send(socket);
    }
}

void AcceptMessageReserveIp(FaithMessage &msg, QTcpSocket* socket)
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
}

void AcceptMessageGetLabListOrHostInfo(FaithMessage &msg, QTcpSocket* socket)
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
            AcceptMessageGetLabListOrHostInfo(msg, socket);
            break;
        }
        case Faithcore::RESERVE_IP:
        {
            AcceptMessageReserveIp(msg, socket);
            break;
        }
        case Faithcore::ACCEPT_IP:
        {
            AcceptMessageAcceptIP(msg, socket);
            break;
        }
        case Faithcore::SEND_FILE:
        {
            AcceptMessageSendFile(msg, socket);
            break;
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
