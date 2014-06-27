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
#include <QCryptographicHash>
#include <QUdpSocket>

void sendWakeOnLanMagicPacket(QString ip)
{
    DhcpHost* host = DhcpConfig::instance().hostByIp(ip);
    if (host)
    {
        QStringList tmp = host->hw().split(":");
        QByteArray mac;
        for (int i=0;i<6;i++)
        {
            mac.append((char)tmp.at(i).toUInt(0,16));
        }
        QByteArray magic_packet;
        magic_packet.append("\xFF\xFF\xFF\xFF\xFF\xFF");
        for (int i=0;i<16;i++) magic_packet.append(mac);
        qDebug() << magic_packet;
        QUdpSocket socket;
        socket.writeDatagram(magic_packet, QHostAddress::Broadcast, 8000);
    }
}
#include "fdiplist.h"

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
    if (_server.listen(QHostAddress::AnyIPv4, port))
    {
        qDebug() << "Starting server at port" << port;
        QSqlDatabase::addDatabase("QSQLITE");
    }
    else
    {
        qDebug() << "Can't open port" << port;
        exit(1);
    }
}

void AcceptMessageGetFile(FaithMessage &msg, QTcpSocket* socket)
{
    QDir dir(Config::instance().configDir());
    FdString* str = static_cast<FdString*>(msg.getData());
    if (!str)
    {
        FaithMessage::MsgError("Can't extract filename from message data").send(socket);
        return;
    }
    QString filename = str->value();
    if (filename.isEmpty())
    {
        FaithMessage::MsgError("Recived empty filename").send(socket);
        return;
    }
    FaithMessage::MsgSendFile(dir.filePath(filename)).send(socket);
}

void AcceptMessageGetFileList(FaithMessage &msg, QTcpSocket* socket)
{
    Q_UNUSED(msg)
    QDir dir(Config::instance().configDir());
    QStringList file_list;
    foreach (QFileInfo finfo, dir.entryInfoList()) {
        QFile f(finfo.absoluteFilePath());
        if (f.open(QIODevice::ReadOnly))
        {
            QByteArray bufor = f.readAll();
            f.close();
            QString filename = finfo.fileName();
            QString md5 = QString(QCryptographicHash::hash(bufor, QCryptographicHash::Md5).toHex());
            file_list.append(filename+"#"+md5);
        }
    }
    FaithMessage::MsgFileList(file_list).send(socket);
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
        if (file->filename()=="host.db")
        {
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
            bool success = file->saveFile(Config::instance().configDir()+"/"+file->filename());
            if (success) FaithMessage::MsgOk().send(socket);
            else FaithMessage::MsgError("SERVER ERROR: Can't save file in config_dir").send(socket);
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

void AcceptMessageRequestInstall(FaithMessage &msg, QTcpSocket* socket)
{
    FdIpList *ipList = static_cast<FdIpList*>(msg.getData());
    if (!ipList)
    {
        FaithMessage::MsgError("Can't extract ip list from message").send(socket);
        return;
    }
    for (int i=0; i<ipList->count(); i++)
    {
        QString ip = ipList->ipString(i);
        qDebug() << ip;

        //TUTAJ TRZEBA WYWOŁAĆ UTWORZENIE KONFIGURACJI PXE
        //TUTAJ TRZEBA WYSLAC PAKIET WOL
    }
    FaithMessage::MsgOk().send(socket);
}

void Server::acceptConnection()
{
    bool newConnection = _server.waitForNewConnection(-1);
    if (newConnection)
    {
        QTcpSocket* socket = _server.nextPendingConnection();
        qDebug() << socket->peerAddress() << " connected";
        //socket->waitForReadyRead();
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
        case Faithcore::GET_FILE_LIST:
        {
            AcceptMessageGetFileList(msg, socket);
            break;
        }
        case Faithcore::GET_FILE:
        {
            AcceptMessageGetFile(msg, socket);
            break;
        }
        case Faithcore::REQUEST_INSTALL:
        {
            AcceptMessageRequestInstall(msg, socket);
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
