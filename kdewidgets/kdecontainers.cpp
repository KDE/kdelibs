#include <qwidgetplugin.h>
#include <kjanuswidget.h>

class KDEContainerPlugin : public QWidgetContainerPlugin
{
    Q_OBJECT

public:
    KDEContainerPlugin(){};

    virtual QStringList keys()
    {
	QStringList list;
	list << "KJanusWidget";
	return list;
    }
    virtual QWidget *create( const QString &key, QWidget *parent = 0, const char *name = 0 )
    {
	if( key == "KJanusWidget")
		return new KJanusWidget( parent, name );
	return 0;
    }
    virtual QIconSet iconSet(const QString &key) const
    {
	if( key == "KJanusWidget")
		return QIconSet( QPixmap("janus.png"));
	return QIconSet(QPixmap());
    }
    virtual bool isContainer(const QString &key) const
    {
	if( key == "KJanusWidget")
		return true;
	return false;

    }
    virtual QString group(const QString &key) const
    {
	if( key == "KJanusWidget")
		return "Containers (KDE)";
	return QString::null;

    }
    virtual QString includeFile(const QString &key) const
    {
	if (key == "KJanusWidget")
		return "kjanuswidget.h"
	return QString::null;
    }
    virtual QString toolTip(const QString &key) const
    {
	if( key == "KJanusWidget")
		return "A flexable contianer widget.";
	return "";

    }
    virtual QString whatsThis(const QString &key) const
    {
	if( key == "KJanusWidget")
		return "A flexable contianer widget.";
	return "";
		
    }
    virtual QWidget* containerOfWidget( const QString &key, QWidget *container ) const
    {
        if( key == "KJanusWidget")
		return container->parent();
    }
    virtual bool isPassiveInteractor( const QString &key, QWidget *container ) const
    {
	if( key == "KJanusWidget")
		return true;
	return false;
    }

    virtual bool supportsPages( const QString &key ) const
    {
	if( key == "KJanusWidget")
		return true;
	return false;
    }

    virtual QWidget *addPage( const QString &key, QWidget *container,
                              const QString &name, int index ) const
    {
	if( key == "KJanusWidget")
		dynamic_cast<KJanusWidget*>(container)->addPage(name,name);
    }

    virtual void insertPage( const QString &key, QWidget *container,
                             const QString &name, int index, QWidget *page ) const
    {
	if( key == "KJanusWidget")
		dynamic_cast<KJanusWidget*>(container)->setSwallowedWidget(page);

    }

    virtual void removePage( const QString &key, QWidget *container, int index ) const
    {
		
    }

    virtual void movePage( const QString &key, QWidget *container, int fromIndex, int toIndex ) const
    {

    }

    virtual int count( const QString &key, QWidget *container ) const
    {
	
    }

    virtual int currentIndex( const QString &key, QWidget *container ) const
    {
	if( key == "KJanusWidget")
	{
		return dynamic_cast(KJanusWidget*>(container)->activePageIndex();
	}
    }

    virtual QString pageLabel( const QString &key, QWidget *container, int index ) const
    {
	return QString::null;
    }

    virtual QWidget *page( const QString &key, QWidget *container, int index ) const
    {

    }

    virtual void renamePage( const QString &key, QWidget *container,
                             int index, const QString &newName ) const
    {

    }

    virtual QWidgetList pages( const QString &key, QWidget *container ) const
    {
		
    }


};

Q_EXPORT_PLUGIN(KDEContainersPlugin)

