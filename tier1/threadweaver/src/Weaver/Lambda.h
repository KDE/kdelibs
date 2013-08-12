#ifndef LAMBDA_H
#define LAMBDA_H

#include <Job.h>
#include <threadweaver_export.h>

namespace ThreadWeaver {

/** @brief Lambda is a template that takes any type on which operator() is available, and executes it in run(). */
template <typename T>
class Lambda : public Job
{
public:
    explicit Lambda(T t_)
        : t(t_)
    {}

protected:
    void run(JobPointer, Thread*) Q_DECL_FINAL {
        t();
    }

private:
    T t;
};

}

#endif // LAMBDA_H
