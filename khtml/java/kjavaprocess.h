// -*- c++ -*-

#ifndef KJAVAPROCESS_H
#define KJAVAPROCESS_H

#include <kprocess.h>
#include <qmap.h>

class KJavaProcessPrivate;

/**
 * @short A class for invoking a Java VM
 *
 * This class is a general tool for invoking a Java interpreter. It allows you
 * to specifiy some of the standard options that should be understood by all
 * JVMs, and in future will also ensure that QtAWT is used instead of the system
 * default toolkit.
 *
 * <H3>Change Log</H3>
 * <PRE>
 * $Log$
 * Revision 1.4  1999/12/14 19:56:59  rich
 * Many fixes, see changelog
 *
 * Revision 1.3  1999/11/12 01:22:34  rich
 * Now trys adding a / to the code base if the class loader could not find the applet class file. Fixed applet start/stop
 *
 * Revision 1.2  1999/10/09 18:10:53  rich
 * Const QString fixes
 *
 * Revision 1.1.1.1  1999/07/22 17:28:07  rich
 * This is a current snapshot of my work on adding Java support
 * to KDE. Applets now work!
 *
 * </PRE>
 *
 * @version $Id$
 * @author Richard J. Moore, rich@kde.org
 */
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
    void startJava();

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

    /**
     * Used to decide the parameter names for JVM stack size etc.
     */
    void setJVMVersion( int major, int minor = 0, int patch = 0 );

    /**
     * The HTTP proxy.
     */
    void setHTTPProxy( const QString& host, int port );

    /**
     * The FTP proxy.
     */
    void setFTPProxy( const QString& host, int port );

    /**
     * Set system properties by adding -D<I>name</I>=<I>value</I> to
     * the java command line.
     */
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
     * Send a string to the standard input (System.in) of the JVM.
     */
    void send( const QString& command );

protected slots:
    void wroteData();
    void processExited();
    void receivedData( KProcess *, char *, int );
    void javaHasDied();

protected:
    virtual void invokeJVM();
    virtual void killJVM();
    KProcess *javaProcess;
signals:
    void received( const QString & );
private:
    KJavaProcessPrivate *d;
    QStrList inputBuffer;
    QMap <QString, QString> systemProps;
};

#endif // KJAVAPROCESS_H


