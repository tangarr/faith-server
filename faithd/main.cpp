#include "config.h"

int main(int argc, char *argv[])
{    
    QCoreApplication a(argc, argv);
    if (!Config::instance().read_config()) exit(1);

    return a.exec();
}
