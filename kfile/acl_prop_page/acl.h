// $Id$

#ifndef KACLPLUGIN_H
#define KACLPLUGIN_H "$Id$"

#include <qobject.h>
#include <klibloader.h>
#include <kpropsdlg.h>

class KACLPrivate;

class KACLPlugin
	: public KPropsDlgPlugin
{
public:
	KACLPlugin (KPropertiesDialog *_props);
	static bool supports (KFileItemList _items);

protected:
	void LoadACL ();

private:
	KACLPrivate *pr;
};

class KACLPluginFactory : public KLibFactory
{
	Q_OBJECT
public:
	KACLPluginFactory( QObject *parent = 0, const char *name = 0 );
	~KACLPluginFactory();
	virtual QObject *create( QObject *parent = 0, const char *name = 0,
				 const char *classname = "QObject",
				 const QStringList &args = QStringList() );
};


#endif
