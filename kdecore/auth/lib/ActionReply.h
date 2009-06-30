#ifndef ACTION_REPLY_H
#define ACTION_REPLY_H

#include <QString>
#include <QVariant>
#include <QHash>

class ActionReply : public QHash<QString, QVariant>
{    
public:
    enum Response
    {
        OK,
        Failed,
        UserCancelled
    };
    
private:
    Response m_response;
    
public:
    Response response() { return m_response; }
    void setResponse(Response response) { m_response = response; }
        
    bool operator==(Response response) { return response == m_response; }
    bool operator!=(Response response) { return response != m_response; }
};

#endif