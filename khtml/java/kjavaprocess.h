// -*- c++ -*-

#ifndef KJAVAPROCESS_H
#define KJAVAPROCESS_H

#include <kprocess.h>
#include <qdict.h>

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
 * </PRE>
 *
 * @version $Id$
 * @author Richard J. Moore, rich@kde.org
 */
class KJavaProcess : public QObject
{
Q_OBJECT

public:
   KJavaProcess();
   virtual ~KJavaProcess();

   void startJava();
   void stopJava();

   bool isOK();
   bool isRunning();

   /**
    * Used to specify the location of the JVM.
    */
   void setJVMPath( QString path );

   /**
    * Used to decide the parameter names for JVM stack size etc.
    */
   void setJVMVersion( int major, int minor = 0, int patch = 0 );

   void setHTTPProxy( QString host, int port );
   void setFTPProxy( QString host, int port );
   void setSystemProperty( QString name, QString value );
   void setMainClass( QString clazzName );
   void setExtraArgs( QString args );
   void setClassArgs( QString classArgs );

   void send( QString command );

protected slots:
    void wroteData();

protected:
   virtual void invokeJVM();
   virtual void killJVM();

   KProcess *javaProcess;
   int versionMajor;
   int versionMinor;
   int versionPatch;
   QString httpProxyHost;
   int httpProxyPort;
   QString ftpProxyHost;
   int ftpProxyPort;
   bool ok;
   QString jvmPath;
   QString mainClass;
   QString extraArgs;
  QString classArgs;

    QStrList inputBuffer;
private:
   QDict<char> systemProps;
};

#endif // KJAVAPROCESS_H


