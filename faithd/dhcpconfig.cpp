#include "dhcpconfig.h"

#include "dhcpcomment.h"
#include "dhcpparameter.h"
#include "dhcpblock.h"
#include "dhcphost.h"
#include "dhcpemptyline.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>

DhcpConfig::DhcpConfig()
{
}

QList<DhcpObject*> parseDhcp(const QString config, int &position)
{
    QList<DhcpObject*> list;

    bool block = false;
    bool whitespace = true;
    bool comment = false;
    bool newLine = false;

    if (config.at(position)=='{')
    {
        block = true;
        position++;
    }
    QString bufor="";
    QString bufor2="";

    while (position < config.length())
    {
        QChar c = config.at(position);
        position++;
        if (comment)
        {
            if (c == '\n')
            {
                list.append(new DhcpComment(bufor));                
                comment = false;
                if (!bufor2.isEmpty())
                {
                    bufor=bufor2;
                    bufor2.clear();
                    whitespace = true;
                }
                else bufor.clear();
            }
            else bufor.append(c);
        }
        else
        {
            if (c == '#')
            {
                if (!bufor.isEmpty())
                {
                    bufor2+=bufor;
                    bufor.clear();
                }
                comment = true;                
            }
            else if (c == ';')
            {
                whitespace = true;
                if (!bufor.isEmpty())
                {
                    list.append(new DhcpParameter(bufor));
                    bufor.clear();
                }
                newLine = false;
            }
            else if (c == '{')
            {
                whitespace = true;
                position--;
                DhcpBlock* block;
                if (bufor.startsWith("host "))
                {
                    bufor = bufor.remove("host ");
                    block = new DhcpHost(bufor);
                }
                else
                    block = new DhcpBlock(bufor);
                bufor.clear();
                QList<DhcpObject*> tmp=parseDhcp(config, position);
                block->appendList(tmp);
                list.append(block);
                newLine = false;
            }
            else if (c == '}')
            {
                if (block)
                {
                    return list;
                }
                else
                {
                    qDebug() << "Error in parsing DHCP configuration";
                    exit(1);
                }
            }
            else if (c == '\n')
            {
                if (newLine) list.append(new DhcpEmptyLine());
                else newLine = true;
            }
            else if (c == ' ' || c == '\t')
            {
                if (!whitespace)
                {
                    whitespace = true;
                    bufor.append(" ");
                }
            }
            else
            {
                whitespace = false;
                bufor.append(c);
                newLine = false;
            }
        }
    }
    if (block)
    {
        qDebug() << "Error in parsing DHCP configuration";
        exit(1);
    }
    else return list;
}

bool DhcpConfig::readConfiguration(QString filename)
{
    foreach (DhcpObject* ob, config) {
        delete ob;
    }
    config.clear();
    hosts.clear();
    QString dhcpd_conf;
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QFile::Text))
    {
        QTextStream stream(&file);
        dhcpd_conf = stream.readAll();
        file.close();
        int pos=0;
        config.append(parseDhcp(dhcpd_conf, pos));
        foreach (DhcpObject* ob, config) {
            DhcpBlock* block = dynamic_cast<DhcpBlock*>(ob);
            if (block) hosts.append(block->getHosts());
        }
        qDebug() << "DHCP config succesfull parsed";
        return true;
    }
    else
    {
        qDebug() << "Can't open DHCP config file: " << filename;
        return false;
    }

}

bool DhcpConfig::writeConfiguration(QString filename) const
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QFile::Text))
    {
        QTextStream stream(&file);
        foreach (DhcpObject* ob, config) {
            stream << ob->toString(0) << "\n";
        }
        file.close();
        qDebug() << "DHCP config writen to:" << filename;
        return true;
    }
    else
    {
        qDebug() << "Can't write DHCP config file: " << filename;
        return false;
    }
    return false;
}

bool DhcpConfig::containsHostname(const QString &hostname) const
{
    foreach (DhcpHost* host, hosts) {
        if (host->hostname().compare(hostname, Qt::CaseInsensitive)) return true;
    }
    return false;
}

bool DhcpConfig::containsIp(const QString &ip) const
{
    const QRegExp regexp("(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9]\\.){3}(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])");
    if (!regexp.exactMatch(ip)) return false;
    foreach (DhcpHost* host, hosts) {
        QStringList a,b;
        a = ip.split(".");
        b = host->ip().split(".");
        for (int i=0; i<4; i++)
        {
            if (a[i].toInt()!=b[i].toInt()) return false;
        }
        return true;
    }
    return false;
}

bool DhcpConfig::containHw(const QString &hw) const
{
    const QRegExp regexp("([0-9a-f]:){5}([0-9a-f])");
    if (!regexp.exactMatch(hw)) return false;
    foreach (DhcpHost* host, hosts) {
        QStringList a,b;
        a = hw.split(":");
        b = host->hw().split(":");
        for (int i=0; i<6; i++)
        {
            if (a[i].toInt(0,16)!=b[i].toInt(0,16)) return false;
        }
        return true;
    }
    return false;
}

DhcpHost *DhcpConfig::hostByIp(const QString &ip) const
{
    const QRegExp regexp("(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9]\\.){3}(25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])");
    if (!regexp.exactMatch(ip)) return 0;
    foreach (DhcpHost* host, hosts) {
        QStringList a,b;
        a = ip.split(".");
        b = host->ip().split(".");
        for (int i=0; i<4; i++)
        {
            if (a[i].toInt()!=b[i].toInt()) return host;
        }
        return 0;
    }
    return 0;

}

DhcpHost *DhcpConfig::hostByHw(const QString &hw) const
{
    const QRegExp regexp("([0-9a-f]:){5}([0-9a-f])");
    if (!regexp.exactMatch(hw)) return 0;
    foreach (DhcpHost* h, hosts) {
        QStringList a,b;
        a = hw.split(":");
        b = h->hw().split(":");
        for (int i=0; i<6; i++)
        {
            if (a[i].toInt(0,16)!=b[i].toInt(0,16)) return 0;
        }
        return h;
    }
    return 0;
}

DhcpHost *DhcpConfig::hostByName(const QString &hostname) const
{
    foreach (DhcpHost* host, hosts) {
        if (host->hostname().compare(hostname, Qt::CaseInsensitive)) return host;
    }
    return 0;
}
