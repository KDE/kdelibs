#include "Lambda.h"

namespace ThreadWeaver {

class Lambda::Private {
public:
    void (*payload)();
};

Lambda::Lambda(void (*payload)())
    : d(new Lambda::Private)
{
    d->payload = payload;
}

Lambda::~Lambda()
{
    delete d;
}

void Lambda::run(JobPointer, Thread*)
{
    if (d->payload) d->payload();
}

}
