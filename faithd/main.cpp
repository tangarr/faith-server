#include "config.h"
#include "dhcpconfig.h"
#include "server.h"

int main(int argc, char *argv[])
{    
    QCoreApplication a(argc, argv);
    DhcpConfig dhcp_config;

    if (!Config::instance().read_config()) exit(1);
    if (!dhcp_config.readConfiguration("/etc/dhcp/dhcpd.conf")) exit(1);

    dhcp_config.writeConfiguration("/tmp/dhcpd.conf");

    Server server(Config::instance().port());
    server.acceptConnection();

    return 0;
}
