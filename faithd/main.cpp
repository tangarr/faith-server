#include "config.h"
#include "dhcpconfig.h"
#include "server.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void mainFunc()
{
    if (!Config::instance().read_config()) exit(1);
    if (!DhcpConfig::instance().readConfiguration("/etc/dhcp/dhcpd.conf")) exit(1);
    Config::instance().assignHostToLaboratories();

    Server server(Config::instance().port());
    while (true) server.acceptConnection();
}

int main(int argc, char *argv[])
{    
    QCoreApplication a(argc, argv);  
    int opt;

    bool demonize = 0;
    while ((opt = getopt(argc, argv, "D"))!= -1)
    {
        switch (opt) {
        case 'D':
            demonize = true;
            break;
        default:
            qDebug() << "usage " << argv[0] << " [-D]";
            return 1;
        }
    }

    if (demonize)
    {
        pid_t child;
        child = fork();
        if (child>=0)
        {
            if (child==0)
            {
                int fd = open("faith.log", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                dup2(fd, 1);
                dup2(fd, 2);
                close(fd);
                mainFunc();
            }
            else
            {
                qDebug() << child;
            }
        }
        else
        {
            qDebug() << "Fork failed !!!";
            return 1;
        }
    }
    else
    {
        mainFunc();
    }
    return 0;
}
