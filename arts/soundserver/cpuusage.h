
#ifndef _arts_cpuusage_h
#define _arts_cpuusage_h

#include "iomanager.h"

namespace Arts {
class CPUUsage
{
private:
        class CPUUsagePrivate *d;

public:
        CPUUsage();
        ~CPUUsage();

        /**
         * get the current CPU usage
         */
        float usage();

        /**
         * check the current CPU usage
         */
        void check();
};
};

#endif
