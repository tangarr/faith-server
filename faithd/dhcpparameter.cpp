#include "dhcpparameter.h"

DhcpParameter::DhcpParameter()
{
}

QString DhcpParameter::toString(int level)
{
   return spaces(level)+parameter;
}
