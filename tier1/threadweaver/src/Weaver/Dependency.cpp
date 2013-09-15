#include "Dependency.h"
#include "JobInterface.h"
#include "ManagedJobPointer.h"

namespace ThreadWeaver {

Dependency::Dependency(const JobPointer& dependent, const JobPointer& dependee)
    : m_dependent(dependent)
    , m_dependee(dependee)
{
}

Dependency::Dependency(JobInterface *dependent, JobInterface *dependee)
    : m_dependent(ManagedJobPointer(dependent))
    , m_dependee(ManagedJobPointer(dependee))
{
}

Dependency::Dependency(const JobPointer& dependent, JobInterface *dependee)
    : m_dependent(dependent)
    , m_dependee(ManagedJobPointer(dependee))
{
}

Dependency::Dependency(JobInterface *dependent, const JobPointer& dependee)
    : m_dependent(ManagedJobPointer(dependent))
    , m_dependee(dependee)
{
}

JobPointer Dependency::dependent() const
{
    return m_dependent;
}

JobPointer Dependency::dependee() const
{
    return m_dependee;
}

}
