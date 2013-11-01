#include "Exception.h"

namespace ThreadWeaver {

Exception::Exception(const QString& message)
    : std::runtime_error(message.toStdString())
    , m_message(message)
{
}

Exception::~Exception() throw() {}

QString Exception::message() const {
    return m_message;
}

JobAborted::JobAborted(const QString &message)
    : Exception(message)
{
}

JobFailed::JobFailed(const QString &message)
    : Exception(message)
{
}

}
