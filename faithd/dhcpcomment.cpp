#include "dhcpcomment.h"

DhcpComment::DhcpComment()
{
}

QString DhcpComment::toString(int level)
{
    return spaces(level) + "#" + comment;
}
