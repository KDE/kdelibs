// -*- c++ -*-

#ifndef KJAVAAPPLETSERVER_H
#define KJAVAAPPLETSERVER_H

#include <qobject.h>
#include <kjavaprocess.h>

/**
 * @short Provides a context for KJavaAppletWidgets
 *
 *
 *
 * <H3>Change Log</H3>
 * <PRE>
 * $Log$
 * </PRE>
 *
 * @version $Id$
 * @author Richard J. Moore, rich@kde.org
 */
class KJavaAppletServer : public QObject
{
Q_OBJECT

public:
    KJavaAppletServer();

    static KJavaAppletServer *getDefaultServer();
    
    void createContext( int contextId );
    void createApplet( int contextId, int appletId,
		       QString name, QString clazzName,
		       QString base );
    void setParameter( int contextId, int appletId,
		       QString name, QString value );
    void showApplet( int contextId, int appletId,
		     QString title );
    void quit();

protected:
	KJavaProcess *process;
};

#endif // KJAVAAPPLETSERVER_H
