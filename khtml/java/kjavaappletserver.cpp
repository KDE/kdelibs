#include <stdlib.h>  

#include <kjavaappletserver.moc>
#include <kjavaprocess.h>
#include <kconfig.h>
#include <kstddirs.h>

// For future expansion
struct KJavaAppletServerPrivate
{
   int counter;
};

static KJavaAppletServer *self = 0;

KJavaAppletServer::KJavaAppletServer()
{
   d = new KJavaAppletServerPrivate;
   CHECK_PTR( d );

   process = new KJavaProcess();
   CHECK_PTR( process );
   
   setupJava( process );
   
   process->startJava();
}

KJavaAppletServer::~KJavaAppletServer()
{
   delete process;
}

KJavaAppletServer* KJavaAppletServer::allocateJavaServer() 
{
   if( self == 0 ) {
      self = new KJavaAppletServer();
      self->d->counter = 0;
   }
   
   self->d->counter++;
   return self;
}

void KJavaAppletServer::freeJavaServer() 
{
   self->d->counter--;
   
   if( self->d->counter == 0 ) {   
      self->quit();
      delete self;
      self = 0;
   }
}

void KJavaAppletServer::setupJava( KJavaProcess *p ) 
{
    KConfig config ( "konquerorrc", true );
    config.setGroup( "Java/JavaScript Settings" );
    
    if( config.readBoolEntry( "JavaAutoDetect", true) ) 
        p->setJVMPath( "java" );
    else
        p->setJVMPath( config.readEntry( "JavaPath", "/usr/lib/jdk" ) + "/java" );
    
    QString extraArgs = config.readEntry( "JavaArgs", "" );
    
    if( config.readBoolEntry( "ShowJavaConsole", false) )
        extraArgs = "-Dkjas.showConsole " + extraArgs;
      
    p->setExtraArgs( extraArgs );
    
    p->setMainClass( "org.kde.kjas.server.Main" );
    
    // Prepare classpath
    QString kjava_classes = locate("data", "kjava/kjava-classes.zip");
    if( kjava_classes.isNull() ) // Should not happen
        return;
    
    QString new_classpath = "CLASSPATH=" +  kjava_classes;
    
    char *classpath = getenv("CLASSPATH");
    if(classpath) {
        new_classpath += ":";
        new_classpath += classpath;
    }
    
    // Need strdup() to prevent freeing the memory we provide to putenv
    putenv(strdup(new_classpath.latin1()));
}

void KJavaAppletServer::createContext( int contextId )
{
    QString s;
    s.sprintf( "createContext!%d\n", contextId );
    process->send( s );
}

void KJavaAppletServer::destroyContext( int contextId )
{
    QString s;
    s.sprintf( "destroyContext!%d\n", contextId );
    process->send( s );
}

void KJavaAppletServer::createApplet( int contextId, int appletId,
				      const QString name, 
                                      const QString clazzName,
				      const QString baseURL,
                                      const QString codeBase,
                                      const QString jarFile,
                                      QSize size )
{
    QString s;
    s.sprintf( "createApplet!%d!%d!%s!%s!%s!%s!%s!%d!%d\n",
	       contextId, appletId,
	       name.latin1(), clazzName.latin1(),
	       baseURL.latin1(), 
               (codeBase.isNull() || codeBase.isEmpty())
	           ? "null" : codeBase.latin1(),
               (jarFile.isNull() || jarFile.isEmpty())
                   ? "null" : jarFile.latin1(),
               size.width(), size.height() );
    process->send( s );

}

void KJavaAppletServer::destroyApplet( int contextId, int appletId )
{
    QString s;
    s.sprintf( "destroyApplet!%d!%d\n",
	       contextId, appletId );
    process->send( s );
}


void KJavaAppletServer::setParameter( int contextId, int appletId,
				      const QString name, const QString value )
{
    QString s;
    s.sprintf( "setParameter!%d!%d!%s!%s\n",
	       contextId, appletId,
	       name.latin1(), value.latin1() );
    process->send( s );
}

void KJavaAppletServer::showApplet( int contextId, int appletId,
				    const QString title )
{
    QString s;
    s.sprintf( "showApplet!%d!%d!%s\n",
	       contextId, appletId,
	       title.latin1() );
    process->send( s );
}

void KJavaAppletServer::startApplet( int contextId, int appletId )
{
    QString s;
    s.sprintf( "startApplet!%d!%d\n",
	       contextId, appletId );
    process->send( s );
}

void KJavaAppletServer::stopApplet( int contextId, int appletId )
{
    QString s;
    s.sprintf( "stopApplet!%d!%d\n",
	       contextId, appletId );
    process->send( s );
}

void KJavaAppletServer::quit()
{
    QString s;
    s = "quit\n";
    process->send( s );
}

