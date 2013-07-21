#include "Lambda.h"

namespace ThreadWeaver {

class Lambda::Private {
public:
    void (*payload)();
};

Lambda::Lambda(void (*payload)(), QObject *parent)
    : Job(parent)
    , d(new Lambda::Private)
{
    d->payload = payload;
}

Lambda::~Lambda()
{
    delete d;
}

void Lambda::run()
{
    if (d->payload) d->payload();
}

}
