#include "dhcpparameter.h"

DhcpParameter::DhcpParameter(QString parameter)
{
    this->parameter = parameter;
}

QString DhcpParameter::toString(int level) const
{
   return spaces(level)+parameter+";";
}
