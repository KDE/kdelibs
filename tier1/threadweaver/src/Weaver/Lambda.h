#ifndef LAMBDA_H
#define LAMBDA_H

#include <Weaver/Job.h>
#include <threadweaver_export.h>

namespace ThreadWeaver {

class THREADWEAVER_EXPORT Lambda : public Job
{
    Q_OBJECT
public:
    explicit Lambda(void (*payload)() = 0, QObject *parent = 0);
    ~Lambda();

protected:
    void run() Q_DECL_FINAL;

private:
    class Private;
    Private* const d;
};

}

#endif // LAMBDA_H
