#ifndef FAITHDATABUILDER_H
#define FAITHDATABUILDER_H

#include "faithdata.h"
#include "faithcore.h"

class FaithDataBuilder
{
public:
    FaithDataBuilder();
    static FaithData *buildFaithData(Faithcore::MessageCode messageCode);
};

#endif // FAITHDATABUILDER_H
