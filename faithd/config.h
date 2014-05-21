#ifndef CONFIG_H
#define CONFIG_H

#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QNetworkInterface>
#include <QList>
#include <QPair>
#include "computerlab.h"
#include "faithcore.h"

class ComputerLab;

class Config
{
private:
    Config();
    const QString settings_file = "/etc/fai/faith.ini";
    int port;
    QString config_dir;
    QList<ComputerLab*> laboratories;
public:
    static Config& instance();
    bool ip_validate(const QString &ip, const QList<QPair<QHostAddress, int> > &interfaces, quint32 *ip_out = 0, quint32 *subnet_out = 0);
    bool read_config();    
};

#endif // CONFIG_H
