#ifndef DEPENDENCY_H
#define DEPENDENCY_H

#include <JobPointer.h>

namespace ThreadWeaver {

class JobInterface;

class THREADWEAVER_EXPORT Dependency {
public:
    explicit Dependency(const JobPointer& dependent, const JobPointer& dependee);
    explicit Dependency(JobInterface* dependent, JobInterface* dependee);
    explicit Dependency(const JobPointer& dependent, JobInterface* dependee);
    explicit Dependency(JobInterface* dependent, const JobPointer& dependee);

    JobPointer dependent() const;
    JobPointer dependee() const;
private:
    const JobPointer m_dependent;
    const JobPointer m_dependee;
};

}

#endif // DEPENDENCY_H
