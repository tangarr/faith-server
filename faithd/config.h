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
    const QString _settings_file = "/etc/fai/faith.ini";
    quint16 _port;
    QString _config_dir;
    QList<ComputerLab*> _laboratories;
public:
    static Config& instance();
    bool ip_validate(const QString &ip, const QList<QPair<QHostAddress, int> > &interfaces, quint32 *ip_out = 0, quint32 *subnet_out = 0);
    bool read_config();
    quint16 port() const;
    QStringList &labList() const;
    ComputerLab *getLab(const QString& name);
    QString configDir() const;
    bool assignHostToLaboratories();
};

#endif // CONFIG_H
