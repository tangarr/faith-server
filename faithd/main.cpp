#include "config.h"

#include "dhcpconfig.h"

int main(int argc, char *argv[])
{    
    QCoreApplication a(argc, argv);
    if (!Config::instance().read_config()) exit(1);
    DhcpConfig dhcp_config;
    dhcp_config.readConfiguration("/etc/dhcp/dhcpd.conf");
    dhcp_config.writeConfiguration("/etc/dhcp/dhcpd.conf");

    return a.exec();
}
