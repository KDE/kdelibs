#include "DependencyPolicy.h"

using namespace ThreadWeaver;

void DependencyPolicy::addDependency( Job* dependency, Job* dependant )
{
}

void DependencyPolicy::resolveDependencies( Job* )
{
}

DependencyPolicy& DependencyPolicy::instance ()
{
    static DependencyPolicy policy;
    return policy;
}

bool DependencyPolicy::canRun( Job* job )
{
    return ! hasUnresolvedDependencies( job );
}

void DependencyPolicy::free( Job* )
{
}

void DependencyPolicy::release( Job* )
{
}

bool DependencyPolicy::hasUnresolvedDependencies( Job* )
{
    return true;
}
