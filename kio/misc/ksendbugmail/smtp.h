#ifndef SMTP_H
#define SMTP_H

#include <qobject.h>
#include <qtimer.h>
#include <kbufferedsocket.h>

/*int SMTPServerStatus[] = {
    220,  // greeting from server
    221,  // server acknolages goodbye
    250,  // command successful
    354,  // ready to receive data
    501,  // error
    550,  // user unknown
    0     // null
};

int SMTPClientStatus[] = {
    50,   // not logged in yet.
    100,  // logged in, got 220
    150,  // sent helo, got 250
    200,  // sent mail from, got 250
    250,  // sent rctp to, got 250
    300,  // data sent, got 354
    350,  // sent data/., got 250
    400,  // send quit, got 221
    450,  // finished, logged out
    0     // null
};
*/

#define DEFAULT_SMTP_PORT 25
#define DEFAULT_SMTP_SERVER localhost
#define DEFAULT_SMTP_TIMEOUT 60

#define SMTP_READ_BUFFER_SIZE 256

class SMTP:public QObject
{
    Q_OBJECT
public:
    SMTP(char *serverhost = 0, unsigned short int port = 0, int timeout = DEFAULT_SMTP_TIMEOUT);
    ~SMTP();

    void setServerHost(const QString& serverhost);
    void setPort(unsigned short int port);
    void setTimeOut(int timeout);

    bool isConnected(){return connected;};
    bool isFinished(){return finished;};
    QString getLastLine(){return lastLine;};

    void setSenderAddress(const QString& sender);
    void setRecipientAddress(const QString& recipient);
    void setMessageSubject(const QString& subject);
    void setMessageBody(const QString& message);
    void setMessageHeader(const QString &header);

    typedef enum {
        NONE = 0,             // null
        GREET = 220,          // greeting from server
        GOODBYE = 221,        // server acknolages quit
        SUCCESSFUL = 250,     // command successful
        READYDATA = 354,      // server ready to receive data
        ERROR = 501,          // error
        UNKNOWN = 550        // user unknown
    }SMTPServerStatus;

    typedef enum {
        INIT = 50,            // not logged in yet
        IN = 100,             // logged in, got 220
        READY = 150,          // sent HELO, got 250
        SENTFROM = 200,       // sent MAIL FROM:, got 250
        SENTTO = 250,         // sent RCTP TO:, got 250
        DATA = 300,           // DATA sent, got 354
        FINISHED = 350,       // finished sending data, got 250
        QUIT = 400,           // sent QUIT, got 221
        OUT = 450,            // finished, logged out
        CERROR = 500           // didn't finish, had error or connection drop
    }SMTPClientStatus;

    typedef enum {
        NOERROR = 0,
        CONNECTERROR = 10,
        NOTCONNECTED = 11,
        CONNECTTIMEOUT = 15,
        INTERACTTIMEOUT = 16,
        UNKNOWNRESPONSE = 20,
        UNKNOWNUSER = 30,
        COMMAND = 40
    }SMTPError;

protected:
    void processLine(QString *line);

public Q_SLOTS:
    void openConnection();
    void sendMessage();
    void closeConnection();

    void connectTimerTick();
    void connectTimedOut();
    void interactTimedOut();

    void socketReadyToRead();
    void socketClosed();

Q_SIGNALS:
    void connectionClosed();
    void messageSent();
    void error(int);

private:
    QString serverHost;
    unsigned short int hostPort;
    int timeOut;

    bool connected;
    bool finished;

    QString senderAddress;
    QString recipientAddress;
    QString messageSubject;
    QString messageBody, messageHeader;

    SMTPClientStatus state;
    SMTPClientStatus lastState;
    SMTPServerStatus serverState;

    QString domainName;

    KNetwork::KBufferedSocket *sock;
    QTimer connectTimer;
    QTimer timeOutTimer;
    QTimer interactTimer;

    char readBuffer[SMTP_READ_BUFFER_SIZE];
    QString lineBuffer;
    QString lastLine;
    QString writeString;
};
#endif
