// -*- c++ -*-

#ifndef KJAVAPROCESS_H
#define KJAVAPROCESS_H

#include <kprocess.h>
#include <qmap.h>
#include <qcstring.h>

/**
 * @short A class for invoking a Java VM
 *
 * This class is a general tool for invoking a Java interpreter. It allows you
 * to specifiy some of the standard options that should be understood by all
 * JVMs, and in future will also ensure that QtAWT is used instead of the system
 * default toolkit.
 *
 * @author Richard J. Moore, rich@kde.org
 */

class KJavaProcessPrivate;
class KJavaProcess : public QObject
{
Q_OBJECT

public:
    /**
     * Create a process object, the process is NOT invoked at this point.
     */
    KJavaProcess();
    virtual ~KJavaProcess();

    /**
     * Invoke the JVM.
     */
    bool startJava();

    /**
     * Stop the JVM (if it's running).
     */
    void stopJava();

    bool isOK();
    bool isRunning();

    /**
     * Used to specify the location of the JVM.
     */
    void setJVMPath( const QString& path );

    void setSystemProperty( const QString& name, const QString& value );

    /**
     * The class to be called when startJava() is called.
     */
    void setMainClass( const QString& clazzName );

    /**
     * Extra flags passed to the JVM.
     */
    void setExtraArgs( const QString& args );

    /**
     * Arguments passed to the main class.
     */
    void setClassArgs( const QString& classArgs );

    /**
     * same as above, but will create a proper message for the new KJAS
     * protocol
     */
    void send( char cmd_code, const QStringList& args );
    void send( char cmd_code, const QStringList& args, const QByteArray& data );

protected slots:
    void slotWroteData();
    void slotReceivedData( int, int& );
    void slotJavaDied();

protected:
    virtual bool invokeJVM();
    virtual void killJVM();

    QByteArray* addArgs( char cmd_code, const QStringList& args );
    void        popBuffer();
    void        sendBuffer( QByteArray* buff );
    void        storeSize( QByteArray* buff );

    KProcess* javaProcess;

signals:
    void received( const QByteArray& );

private:
    KJavaProcessPrivate *d;

};

#endif // KJAVAPROCESS_H
