#include "config.h"

Config::Config()// : settings_file("/etc/fai/faith.ini")
{
}

Config &Config::instance()
{
    static Config config;
    return config;
}

bool Config::ip_validate(const QString &ip, const QList<QPair<QHostAddress,int> > &interfaces, quint32* ip_out, quint32* subnet_out)
{
    const QRegExp regexp("(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9]\\.){3}(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])");
    if (!regexp.exactMatch(ip)) return false;
    QHostAddress hostAddress = QHostAddress(ip);
    for (int i=0; i<interfaces.count();i++)
    {
        QPair<QHostAddress,int> p = interfaces.at(i);
        if (hostAddress.isInSubnet(p))
        {
            QHostAddress subnet = QHostAddress::parseSubnet(p.first.toString()+"/"+QString::number(p.second)).first;
            quint32 bits = (0xffffffff << (32-p.second)) ^ 0xffffffff;
            QHostAddress bcast = QHostAddress(subnet.toIPv4Address() | bits);
            if (hostAddress == subnet) return false;
            if (hostAddress == bcast) return false;
            if (ip_out) *ip_out = hostAddress.toIPv4Address();
            if (subnet_out) *subnet_out = subnet.toIPv4Address();
            return true;
        }
    }
    return false;

}

bool Config::read_config()
{
    QList<QPair<QHostAddress,int> > interfaces;

    QList<QNetworkInterface> ifaces_tmp =QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface i, ifaces_tmp) {
        if (i.name() != "lo")
        foreach (QNetworkAddressEntry e, i.addressEntries()) {
            if (e.ip().protocol() == QAbstractSocket::IPv4Protocol)
            {
                interfaces.append(QPair<QHostAddress,int>(e.ip(), e.prefixLength()));
            }
        }
    }
    ifaces_tmp.clear();


    QFile file(settings_file);
    if (!file.exists())
    {
        qDebug() << "Config file not found!";
        return false;
    }
    QSettings settings(settings_file, QSettings::IniFormat);
    if (!settings.contains("port"))
    {
        qDebug() << "Config doesn't contain 'port' value";
        return false;
    }
    else
    {
        bool ok;
        port = settings.value("port").toInt(&ok);
        if (!ok)
        {
            qDebug() << "'port' value is not valid";
            return false;
        }
    }
    if (!settings.contains("config_dir"))
    {
        qDebug() << "Config doesn't contain 'config_dir' value";
        return false;
    }
    else
    {
        config_dir = settings.value("config_dir").toString();
        QDir dir(config_dir);
        if (!dir.exists())
        {
            qDebug() << "Config direcotry"<<config_dir<<"doesn't exist";
            return false;
        }
    }
    if (settings.childGroups().empty())
    {
        qDebug() << "Config doesn't contain any computer lab";
        return false;
    }
    foreach (QString lab, settings.childGroups()) {
        settings.beginGroup(lab);
        QString ip_start, ip_end;
        quint32 addr_start, addr_end, subnet_start, subnet_end;

        if (!settings.contains("ip_start"))
        {
            qDebug() << "lab" << lab << "doesn't contain 'ip_start' value";
            return false;
        }
        else
        {
            ip_start = settings.value("ip_start").toString();
            if(!ip_validate(ip_start, interfaces, &addr_start, &subnet_start))
            {
                qDebug() << lab << " 'ip_start' doesn't contain valid ip address";
                return false;
            }
        }
        if (!settings.contains("ip_end"))
        {
            qDebug() << "lab" << lab << "doesn't contain 'ip_end' value";
            return false;
        }
        else
        {
            ip_end = settings.value("ip_end").toString();
            if(!ip_validate(ip_end, interfaces, &addr_end, &subnet_end))
            {
                qDebug() << lab << " 'ip_end' doesn't contain valid ip address";
                return false;
            }
        }
        settings.endGroup();

        if (subnet_start != subnet_end)
        {
            qDebug() << lab << ": 'ip_start' and 'ip_end' must be in the same subnet";
            return false;
        }
        if (addr_start > addr_end)
        {
            qDebug() << lab << ": 'ip_start' couldn't be less than 'ip_end'";
            return false;
        }
    }

    qDebug() << "Configuration... OK";
    return true;
}
