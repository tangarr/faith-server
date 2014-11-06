#include "server.h"
#include <QTcpSocket>
#include <QDebug>
#include "faithmessage.h"
#include "config.h"
#include "fdstring.h"
#include "fdhostinfo.h"
#include "dhcpconfig.h"
#include "fdfile.h"
#include "fdiplist.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryFile>
#include <QSqlError>
#include <QHash>
#include <QCryptographicHash>
#include <QUdpSocket>
#include <QProcess>

void sendWakeOnLanMagicPacket(QString ip)
{
    DhcpHost* host = DhcpConfig::instance().hostByIp(ip);
    if (host)
    {
        qDebug() << "host:" << host->ip() << host->hw() << host->hostname();
        QStringList tmp = host->hw().split(":");
        QByteArray mac;
        for (int i=0;i<6;i++)
        {
            mac.append((char)tmp.at(i).toUInt(0,16));
        }
        QByteArray magic_packet;
        magic_packet.append("\xFF\xFF\xFF\xFF\xFF\xFF");
        for (int i=0;i<16;i++) magic_packet.append(mac);  
        QUdpSocket socket;             
        foreach (QNetworkInterface iface, QNetworkInterface::allInterfaces()) {
            QList <QNetworkAddressEntry> entries = iface.addressEntries();
            if (entries.count())
            {
                quint32 e_ip = entries[0].ip().toIPv4Address();
                quint32 e_broadcast = entries[0].broadcast().toIPv4Address();
                if (e_ip < host->ip() && e_broadcast > host->ip())
                {
                    socket.writeDatagram(magic_packet, entries[0].broadcast(), 8000);
                }
            }
        }
    }
}


bool restartDhcpServer()
{
   return  QProcess::execute("/etc/init.d/isc-dhcp-server restart");
}

bool generatePxeDefault()
{
    QString port = QString::number(Config::instance().port());
    QString cmd = "/usr/sbin/fai-chboot -iF -k \"FAI_ACTION=faith FAITH_PORT="+port+"\" default ";
    qDebug() << cmd;
    return  QProcess::execute(cmd);
}
bool generatePxeLocalboot(QString ip)
{
    QString cmd = "/usr/sbin/fai-chboot -o "+ip;
    qDebug() << cmd;
    return  QProcess::execute(cmd);
}
bool generatePxeInstall(QString ip)
{
    QString port = QString::number(Config::instance().port());
    QString cmd = "/usr/sbin/fai-chboot -IFv -k \"FAITH_PORT="+port+"\" "+ip;
    qDebug() << cmd;
    return  QProcess::execute(cmd);
}

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
        generatePxeDefault();
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
                        FaithMessage::MsgError("Host ip not in Dhcp configuration").send(socket);
                        return;
                    }
                    else
                    {
                        qDebug() << "SEND_FILE found ip of host:";
                        qDebug() << host->hostname();
                        qDebug() << host->ip();
                        qDebug() << host->hw();
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
                if (success)
                {
                    FaithMessage::MsgOk().send(socket);
                    restartDhcpServer();
                    generatePxeLocalboot(Faithcore::ipFromInt(ip));
                }
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
            if (success)
            {
                FaithMessage::MsgOk().send(socket);
                QString ext = file->filename().split(".").last();
                if (ext=="diskconfig")
                {
                    QString src = Config::instance().configDir()+"/"+file->filename();
                    QString dst = Config::instance().faiConfigDir()+"/disk_config/"+(file->filename().remove(".diskconfig"));
                    QDir dir;
                    if (!dir.rename(src, dst))
                    {
                        system(QString("mv "+src+" "+dst).toStdString().c_str());
                    }
                }
                else if (ext=="var" || ext=="software")
                {
                    QString src = Config::instance().configDir()+"/"+file->filename();
                    QString dst = Config::instance().faiConfigDir()+"/class/"+(file->filename());
                    QDir dir;
                    if (!dir.rename(src, dst))
                    {
                        system(QString("mv "+src+" "+dst).toStdString().c_str());
                    }
                }
                else if (ext=="script")
                {
                    QString labname = file->filename().split(".").first();
                    QDir dir(Config::instance().faiConfigDir()+"/scripts/"+labname);
                    if (!dir.exists()) dir.mkpath(dir.absolutePath());
                    QString src = Config::instance().configDir()+"/"+file->filename();
                    QString dst = Config::instance().faiConfigDir()+"/scripts/"+labname+"/"+(file->filename().remove(labname+".").remove(".script"));
                    if (!dir.rename(src, dst))
                    {
                        system(QString("mv "+src+" "+dst).toStdString().c_str());
                    }
                    QFile f(dst);
                    f.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner | QFile::ReadGroup | QFile::ExeGroup | QFile::ReadOther | QFile::ExeOther);
                }
            }
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

            QString filename = Config::instance().faiConfigDir()+"/class/"+lab->name().toUpper()+".hostlist";
            QFile file(filename);
            if (file.open(QIODevice::Append))
            {
                QTextStream stream(&file);
                stream << Faithcore::ipFromInt(host->ip()) << "\n";
                file.close();
            }

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
        qDebug() << "mac: " << mac;
        foreach (QString lab_name, Config::instance().labList()) {
            ComputerLab* tmp = Config::instance().getLab(lab_name);
            if (tmp->containHw(mac))
            {
                qDebug() << tmp->name() << "TRUE";
                lab = tmp;
                break;
            }
            qDebug() << tmp->name() << "FALSE";
        }
        if (lab)
        {
            qDebug() << lab;
            DhcpHost* host = lab->hostByHw(mac);
            qDebug() << "MsgHostInfo:" << lab->name() << host->hostname() << host->ip();
            FaithMessage resp = FaithMessage::MsgHostInfo(lab->name(), host->hostname(), host->ip());
            resp.send(socket);
        }
        else
        {
            qDebug() << "BLAD";
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
        generatePxeInstall(ip);
        sendWakeOnLanMagicPacket(ip);
    }
    FaithMessage::MsgOk().send(socket);
}

void AcceptMessageInstallCompleted(FaithMessage &msg, QTcpSocket* socket)
{
    Q_UNUSED(msg)
    FaithMessage::MsgOk().send(socket);
    generatePxeLocalboot(socket->peerAddress().toString());
    qDebug() << "installation complete on host "+socket->peerAddress().toString();
}

void AcceptMessageInstallAborted(FaithMessage &msg, QTcpSocket* socket)
{
    FdString* str = static_cast<FdString*>(msg.getData());
    if (str)
    {
        FaithMessage::MsgOk().send(socket);
    }
    else
    {
        FaithMessage::MsgError("Can't extract string from message").send(socket);
    }
    qDebug() << "installation aborted on host "+socket->peerAddress().toString();
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
        case Faithcore::INSTALL_COMPLETE:
        {
            AcceptMessageInstallCompleted(msg, socket);
            break;
        }
        case Faithcore::INSTALL_ABORTED:
        {
            AcceptMessageInstallAborted(msg, socket);
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
