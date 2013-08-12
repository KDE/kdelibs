#ifndef LAMBDA_H
#define LAMBDA_H

#include <Job.h>
#include <threadweaver_export.h>

namespace ThreadWeaver {

class THREADWEAVER_EXPORT Lambda : public Job
{
public:
    explicit Lambda(void (*payload)() = 0);
    ~Lambda();

protected:
    void run(JobPointer self, Thread* thread) Q_DECL_FINAL;

private:
    class Private;
    Private* const d;
};

}

#endif // LAMBDA_H
