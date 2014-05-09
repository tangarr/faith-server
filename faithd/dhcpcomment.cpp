#include "dhcpcomment.h"

DhcpComment::DhcpComment(QString comment)
{
    this->comment=comment;
}

QString DhcpComment::toString(int level) const
{
    return spaces(level) + "#" + comment;
}
