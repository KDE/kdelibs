/*  This file is part of the KDE project
    Copyright (C) 2002-2003 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "kpluginselector.h"
#include "kpluginselector_p.h"

#include <qtooltip.h>
#include <qlabel.h>
#include <qfile.h>
#include <qstring.h>
#include <qlayout.h>
#include <qcursor.h>
#include <qapplication.h>
#include <qobject.h>
#include <QEvent>
#include <QHelpEvent>
#include <QTreeWidget>

#include <kdebug.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <ktabctl.h>
#include <kcmoduleinfo.h>
#include <kservice.h>
#include <ktrader.h>
#include <ktabwidget.h>
#include <kiconloader.h>
#include <kcmodule.h>
#include "kcmoduleinfo.h"
#include "kcmoduleloader.h"
#include <qsplitter.h>
#include <qframe.h>
#include "kplugininfo.h"
#include <kinstance.h>
#include <qstringlist.h>
#include <QList>
#include "kcmoduleproxy.h"
#include <QStackedWidget>

/*
    QCheckListViewItem that holds a pointer to the KPluginInfo object.
    Used in the tooltip code to access additional fields
*/
class KPluginInfoLVI : public QTreeWidgetItem
{
    public:
        KPluginInfoLVI( KPluginInfo *pluginInfo, QTreeWidget *parent )
            : QTreeWidgetItem( parent, QStringList( pluginInfo->name() ) ),
              m_pluginInfo( pluginInfo )
        {
            QString toolTip = i18n( "<qt><table>"
                "<tr><td><b>Description:</b></td><td>%1</td></tr>"
                "<tr><td><b>Author:</b></td><td>%2</td></tr>"
                "<tr><td><b>Version:</b></td><td>%3</td></tr>"
                "<tr><td><b>License:</b></td><td>%4</td></tr></table></qt>" )
                .arg( m_pluginInfo->comment(),
                      m_pluginInfo->author(),
                      m_pluginInfo->version(),
                      m_pluginInfo->license() );
            setToolTip(0, toolTip);
        }

        KPluginInfo * pluginInfo() { return m_pluginInfo; }

    private:
        KPluginInfo *m_pluginInfo;
};

struct KPluginSelectionWidget::KPluginSelectionWidgetPrivate
{
    KPluginSelectionWidgetPrivate( KPluginSelector * _kps,
                                   const QString & _cat,
                                   KConfigGroup * _config )
     : widgetstack( 0 )
        , kps( _kps )
        , config( _config )
        , catname( _cat )
        , currentplugininfo( 0 )
        , visible( true )
        , currentchecked( false )
        , changed( 0 )
    {
    }

    ~KPluginSelectionWidgetPrivate()
    {
        delete config;
    }

    QMap<QTreeWidgetItem*, KPluginInfo*> pluginInfoMap;

    QTreeWidget     * listview;
    QStackedWidget * widgetstack;
    KPluginSelector * kps;
    KConfigGroup * config;

    QMap<QString, int> widgetIDs;
    QMap<KPluginInfo*, bool> plugincheckedchanged;
    QString catname;
    QList<KCModuleProxy*> modulelist;
    QMap<KCModuleProxy *, QStringList> moduleParentComponents;

    KPluginInfo * currentplugininfo;
    bool visible;
    bool currentchecked;
    int changed;
};


KPluginSelectionWidget::KPluginSelectionWidget(
        const QList<KPluginInfo*> & plugininfos, KPluginSelector * kps,
        QWidget * parent, const QString & catname, const QString & category,
        KConfigGroup * config )
    : QWidget( parent )
    , d( new KPluginSelectionWidgetPrivate( kps, catname, config ) )
{
    init( plugininfos, category );
}

inline QString KPluginSelectionWidget::catName() const
{
    return d->catname;
}

void KPluginSelectionWidget::init( const QList<KPluginInfo*> & plugininfos,
        const QString & category )
{
    // setup Widgets
//    ( new QVBoxLayout( this, 0, KDialog::spacingHint() ) )->setAutoAdd( true );
    QTreeWidget * listview = new QTreeWidget( this );
    d->listview = listview;

    connect( listview, SIGNAL( itemPressed( QTreeWidgetItem *, int ) ), this,
            SLOT( executed( QTreeWidgetItem *, int ) ) );
    connect( listview, SIGNAL( itemActivated( QTreeWidgetItem *, int ) ), this,
            SLOT( executed( QTreeWidgetItem *, int ) ) );
    connect( listview, SIGNAL( itemSelectionChanged( QTreeWidgetItem *, int ) ), this,
            SLOT( executed( QTreeWidgetItem *, int ) ) );
    listview->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
    listview->setAcceptDrops( false );
    listview->setHeaderLabels( QStringList( i18n( "Name" ) ) );
    for( QList<KPluginInfo*>::ConstIterator it = plugininfos.begin();
            it != plugininfos.end(); ++it )
    {
        d->plugincheckedchanged[ *it ] = false;
        if( !( *it )->isHidden() &&
                ( category.isNull() || ( *it )->category() == category ) )
        {
            QTreeWidgetItem * item = new KPluginInfoLVI( *it, listview );
            if( ! ( *it )->icon().isEmpty() )
                item->setIcon( 0, SmallIcon( ( *it )->icon(), IconSize( KIcon::Small ) ) );
            if ( ( *it )->isPluginEnabled() )
                item->setCheckState( 0, Qt::Checked );
            else
                item->setCheckState( 0, Qt::Unchecked );
            d->pluginInfoMap.insert( item, *it );
        }
    }

    // widgetstack
    d->widgetstack = d->kps->widgetStack();
    load();
    // select and highlight the first item in the plugin list
    if( listview->model()->rowCount() >= 1 ) {
        const QModelIndex index = listview->model()->index( 0, 0 );
        listview->selectionModel()->setCurrentIndex( index, QItemSelectionModel::Select );
    }
}

KPluginSelectionWidget::~KPluginSelectionWidget()
{
    delete d;
}

bool KPluginSelectionWidget::pluginIsLoaded( const QString & pluginName ) const
{
    QMapIterator<QTreeWidgetItem*, KPluginInfo*> it( d->pluginInfoMap );
    while ( it.hasNext() ) {
        it.next();
        if( it.value()->pluginName() == pluginName )
            return it.value()->isPluginEnabled();
    }

    return false;
}


QWidget * KPluginSelectionWidget::insertKCM( QWidget * parent,
        const KCModuleInfo & moduleinfo )
{
    KCModuleProxy * module = new KCModuleProxy( moduleinfo, false,
            parent );
    if( !module->realModule() )
    {
        //FIXME: not very verbose
        QLabel * label = new QLabel( i18n( "Error" ), parent );
        label->setAlignment( Qt::AlignCenter );

        return label;
    }
    // add the KCM to the list so that we can call load/save/defaults on it
    d->modulelist.append( module );
    QStringList parentComponents = moduleinfo.service()->property(
        "X-KDE-ParentComponents" ).toStringList();
    d->moduleParentComponents.insert( module, parentComponents );
    connect( module, SIGNAL( changed( bool ) ), SLOT( clientChanged( bool ) ) );
    return module;
}

void KPluginSelectionWidget::embeddPluginKCMs( KPluginInfo * plugininfo, bool checked )
{
    //if we have Services for the plugin we should be able to
    //create KCM(s)
    QApplication::setOverrideCursor( Qt::WaitCursor );
    if( plugininfo->kcmServices().size() > 1 )
    {
        // we need a tabwidget
        KTabWidget * tabwidget = new KTabWidget( d->widgetstack );
        tabwidget->setEnabled( checked );

        int id = d->widgetstack->addWidget( tabwidget );
        d->kps->configPage( id );
        d->widgetIDs[ plugininfo->pluginName() ] = id;

        for( QList<KService::Ptr>::ConstIterator it =
                plugininfo->kcmServices().begin();
                it != plugininfo->kcmServices().end(); ++it )
        {
            if( !( *it )->noDisplay() )
            {
                KCModuleInfo moduleinfo( *it );
                QWidget * module = insertKCM( tabwidget, moduleinfo );
                tabwidget->addTab( module, moduleinfo.moduleName() );
            }
        }
    }
    else
    {
        if( !plugininfo->kcmServices().front()->noDisplay() )
        {
            KCModuleInfo moduleinfo(
                    plugininfo->kcmServices().front() );
            QWidget * module = insertKCM( d->widgetstack, moduleinfo );
            module->setEnabled( checked );

            int id = d->widgetstack->addWidget( module );
            d->kps->configPage( id );
            d->widgetIDs[ plugininfo->pluginName() ] = id;
        }
    }
    QApplication::restoreOverrideCursor();
}

inline void KPluginSelectionWidget::updateConfigPage()
{
    updateConfigPage( d->currentplugininfo, d->currentchecked );
}

void KPluginSelectionWidget::updateConfigPage( KPluginInfo * plugininfo,
        bool checked )
{
    //kDebug( 702 ) << k_funcinfo << endl;
    d->currentplugininfo = plugininfo;
    d->currentchecked = checked;

    // if this widget is not currently visible (meaning that it's in a tabwidget
    // and another tab is currently opened) it's not allowed to change the
    // widgetstack
    if( ! d->visible )
        return;

    if( 0 == plugininfo )
    {
        d->kps->configPage( 1 );
        return;
    }

    if( plugininfo->kcmServices().empty() )
        d->kps->configPage( 1 );
    else
    {
        if( !d->widgetIDs.contains( plugininfo->pluginName() ) )
            // if no widget exists for the plugin create it
            embeddPluginKCMs( plugininfo, checked );
        else
        {
            // the page already exists
            int id = d->widgetIDs[ plugininfo->pluginName() ];
            d->kps->configPage( id );
            d->widgetstack->widget( id )->setEnabled( checked );
        }
    }
}

void KPluginSelectionWidget::clientChanged( bool didchange )
{
    kDebug( 702 ) << k_funcinfo << endl;
    d->changed += didchange ? 1 : -1;
    if( d->changed == 1 )
        emit changed( true );
    else if( d->changed == 0 )
        emit changed( false );
    else if( d->changed < 0 )
        kError( 702 ) << "negative changed value: " << d->changed << endl;
}

void KPluginSelectionWidget::tabWidgetChanged( QWidget * widget )
{
    if( widget == this )
    {
        d->visible = true;
        updateConfigPage();
    }
    else
        d->visible = false;
}

void KPluginSelectionWidget::executed( QTreeWidgetItem * item, int )
{
    kDebug( 702 ) << k_funcinfo << endl;
    if( item == 0 )
        return;

    bool checked = ( item->checkState( 0 ) == Qt::Checked );

    KPluginInfo * info = d->pluginInfoMap[ item ];
    Q_ASSERT( !info->isHidden() );

    if ( info->isPluginEnabled() != checked )
    {
        kDebug( 702 ) << "Item changed state, emitting changed()" << endl;

        if( ! d->plugincheckedchanged[ info ] )
        {
            ++d->changed;
            if ( d->changed == 1 )
                emit changed( true );
        }
        d->plugincheckedchanged[ info ] = true;

        checkDependencies( info );
    }
    else
    {
        if( d->plugincheckedchanged[ info ] )
        {
            --d->changed;
            if ( d->changed == 0 )
                emit changed( false );
        }
        d->plugincheckedchanged[ info ] = false;
        // FIXME: plugins that depend on this plugin need to be disabled, too
    }

    updateConfigPage( info, checked );
}

void KPluginSelectionWidget::load()
{
    //kDebug( 702 ) << k_funcinfo << endl;

    QMapIterator<QTreeWidgetItem*, KPluginInfo*> it( d->pluginInfoMap );
    while ( it.hasNext() ) {
        it.next();

        KPluginInfo * info = it.value();
        info->load( d->config );
        it.key()->setCheckState( 0, info->isPluginEnabled() ? Qt::Checked : Qt::Unchecked );
        if( d->visible && info == d->currentplugininfo )
            d->currentchecked = info->isPluginEnabled();
    }

    for( QList<KCModuleProxy*>::Iterator it = d->modulelist.begin();
            it != d->modulelist.end(); ++it )
        if( ( *it )->changed() )
            ( *it )->load();

    updateConfigPage();
    // TODO: update changed state
}

void KPluginSelectionWidget::save()
{
    kDebug( 702 ) << k_funcinfo << endl;

    QMapIterator<QTreeWidgetItem*, KPluginInfo*> it( d->pluginInfoMap );
    while ( it.hasNext() ) {
        it.next();

        KPluginInfo * info = it.value();
        bool checked = ( it.key()->checkState( 0 ) == Qt::Checked );
        info->setPluginEnabled( checked );
        info->save( d->config );
        d->plugincheckedchanged[ info ] = false;
    }
    QStringList updatedModules;
    for( QList<KCModuleProxy*>::Iterator it = d->modulelist.begin();
            it != d->modulelist.end(); ++it )
        if( ( *it )->changed() )
        {
            ( *it )->save();
            QStringList names = d->moduleParentComponents.value( *it );
            if( names.isEmpty() )
                names.append( QString() );
            for( QStringList::ConstIterator nameit = names.begin();
                    nameit != names.end(); ++nameit )
                if( !updatedModules.contains( *nameit ) )
                    updatedModules.append( *nameit );
        }
    for( QStringList::ConstIterator it = updatedModules.begin(); it != updatedModules.end(); ++it )
        emit configCommitted( ( *it ).toLatin1() );

    updateConfigPage();
    kDebug( 702 ) << "syncing config file" << endl;
    d->config->sync();
    d->changed = 0;
    emit changed( false );
}

void KPluginSelectionWidget::checkDependencies( const KPluginInfo * info )
{
    if( info->dependencies().isEmpty() )
        return;

    for( QStringList::ConstIterator it = info->dependencies().begin();
            it != info->dependencies().end(); ++it ) {

        QMapIterator<QTreeWidgetItem*, KPluginInfo*> infoIt( d->pluginInfoMap );
        while ( infoIt.hasNext() ) {
            infoIt.next();

            if( infoIt.value()->pluginName() == *it ) {
                if( infoIt.key()->checkState(0) == Qt::Unchecked )
                {
                    infoIt.key()->setCheckState( 0, Qt::Checked );
                    checkDependencies( infoIt.value() );
                }
                continue;
            }
        }
    }
}

class KPluginSelector::KPluginSelectorPrivate
{
    public:
        KPluginSelectorPrivate()
            : frame( 0 )
            , tabwidget( 0 )
            , widgetstack( 0 )
            , hideconfigpage( false )
            {
            }

        QFrame * frame;
        KTabWidget * tabwidget;
        QStackedWidget * widgetstack;
        QList<KPluginSelectionWidget *> pswidgets;
        bool hideconfigpage;
};

KPluginSelector::KPluginSelector( QWidget * parent )
    : QWidget( parent )
    , d( new KPluginSelectorPrivate )
{
    QBoxLayout * hbox = new QHBoxLayout( this );
    hbox->setMargin( 0 );
    hbox->setSpacing( KDialog::spacingHint() );

    QSplitter* splitter = new QSplitter( Qt::Horizontal, this );
    d->frame = new QFrame( splitter );
    d->frame->setObjectName( "KPluginSelector left frame" );
    d->frame->setFrameStyle( QFrame::NoFrame );
    hbox->addWidget( splitter );
//    ( new QVBoxLayout( d->frame, 0, KDialog::spacingHint() ) )->setAutoAdd( true );

    // widgetstack
    d->widgetstack = new QStackedWidget( splitter/*, "KPluginSelector Config Pages"*/ );
    d->widgetstack->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    d->widgetstack->setMinimumSize( 200, 200 );

    QLabel * label = new QLabel( i18n( "(This plugin is not configurable)" ),
            d->widgetstack );
//    ( new QVBoxLayout( label, 0, KDialog::spacingHint() ) )->setAutoAdd( true );
    label->setAlignment( Qt::AlignCenter );
    label->setMinimumSize( 200, 200 );

    d->widgetstack->insertWidget( 1,label );

    configPage( 1 );
}

KPluginSelector::~KPluginSelector()
{
    delete d;
}

void KPluginSelector::checkNeedForTabWidget()
{
    kDebug( 702 ) << k_funcinfo << endl;
    if( ! d->tabwidget && d->pswidgets.size() == 1 )
    {
        kDebug( 702 ) << "no TabWidget and one KPluginSelectionWidget" << endl;
        // there's only one KPluginSelectionWidget yet, we need a TabWidget
        KPluginSelectionWidget * w = d->pswidgets.first();
        if( w )
        {
            kDebug( 702 ) << "create TabWidget" << endl;
            d->tabwidget = new KTabWidget( d->frame  );
            d->tabwidget->setObjectName( "KPluginSelector TabWidget" );
            w->setParent( d->tabwidget );
            d->tabwidget->addTab( w, w->catName() );
            connect( d->tabwidget, SIGNAL( currentChanged( QWidget * ) ), w,
                    SLOT( tabWidgetChanged( QWidget * ) ) );
        }
    }
}

static QList<KPluginInfo*> kpartsPluginInfos( const QString& instanceName )
{
    if( instanceName.isNull() )
        return QList<KPluginInfo*>(); //nothing

    const QStringList desktopfilenames = KGlobal::dirs()->findAllResources( "data",
            instanceName + "/kpartplugins/*.desktop", true, false );
    return KPluginInfo::fromFiles( desktopfilenames );
}

void KPluginSelector::addPlugins( const QString & instanceName,
        const QString & catname, const QString & category, KConfig * config )
{
    const QList<KPluginInfo*> plugininfos = kpartsPluginInfos( instanceName );
    if ( plugininfos.isEmpty() )
        return;
    checkNeedForTabWidget();
    Q_ASSERT( config ); // please set config, or use addPlugins( instance, ... ) which takes care of it
    if ( !config ) // KDE4: ensure that config is always set; make it second in the arg list?
        config = new KSimpleConfig(  instanceName ); // memleak!
    KConfigGroup * cfgGroup = new KConfigGroup( config, "KParts Plugins" );
    kDebug( 702 ) << k_funcinfo << "cfgGroup = " << cfgGroup << endl;
    addPluginsInternal( plugininfos, catname, category, cfgGroup );
}

void KPluginSelector::addPluginsInternal( const QList<KPluginInfo*> plugininfos,
                                          const QString & catname, const QString & category,
                                          KConfigGroup* cfgGroup )
{
    KPluginSelectionWidget * w;
    if( d->tabwidget )
    {
        w = new KPluginSelectionWidget( plugininfos, this,
                d->tabwidget, catname, category, cfgGroup );
        d->tabwidget->addTab( w, catname );
        connect( d->tabwidget, SIGNAL( currentChanged( QWidget * ) ), w,
                SLOT( tabWidgetChanged( QWidget * ) ) );
    }
    else
        w = new KPluginSelectionWidget( plugininfos, this, d->frame,
                catname, category, cfgGroup );
    w->setMinimumSize( 200, 200 );
    connect( w, SIGNAL( changed( bool ) ), this, SIGNAL( changed( bool ) ) );
    connect( w, SIGNAL( configCommitted( const QByteArray & ) ), this,
            SIGNAL( configCommitted( const QByteArray & ) ) );
    d->pswidgets += w;
}

void KPluginSelector::addPlugins( const KInstance * instance, const QString &
        catname, const QString & category, KConfig * config )
{
    if ( !config )
        config = instance->config();
    addPlugins( instance->instanceName(), catname, category, config );
}

void KPluginSelector::addPlugins( const QList<KPluginInfo*> & plugininfos,
        const QString & catname, const QString & category, KConfig * config )
{
    checkNeedForTabWidget();
    // the KConfigGroup becomes owned by KPluginSelectionWidget
    KConfigGroup * cfgGroup = new KConfigGroup( config ? config : KGlobal::config(), "Plugins" );
    kDebug( 702 ) << k_funcinfo << "cfgGroup = " << cfgGroup << endl;
    addPluginsInternal( plugininfos, catname, category, cfgGroup );
}

QStackedWidget * KPluginSelector::widgetStack()
{
    return d->widgetstack;
}

inline void KPluginSelector::configPage( int id )
{
    if( id == 1 )
    {
        // no config page
        if( d->hideconfigpage )
        {
            d->widgetstack->hide();
            return;
        }
    }
    else
        d->widgetstack->show();

    d->widgetstack->setCurrentIndex( id );
}

void KPluginSelector::setShowEmptyConfigPage( bool show )
{
    d->hideconfigpage = !show;
    if( d->hideconfigpage )
        if( d->widgetstack->currentIndex () == 1 )
            d->widgetstack->hide();
}

void KPluginSelector::load()
{
    for( QList<KPluginSelectionWidget *>::Iterator it =
            d->pswidgets.begin(); it != d->pswidgets.end(); ++it )
    {
        ( *it )->load();
    }
}

void KPluginSelector::save()
{
    for( QList<KPluginSelectionWidget *>::Iterator it =
            d->pswidgets.begin(); it != d->pswidgets.end(); ++it )
    {
        ( *it )->save();
    }
}

void KPluginSelector::defaults()
{
    kDebug( 702 ) << k_funcinfo << endl;

    // what should defaults do? here's what I think:
    // Pressing a button in the dialog should not change any widgets that are
    // not visible for the user. Therefor we may only change the currently
    // visible plugin's KCM. Restoring the default plugin selections is therefor
    // not possible. (if the plugin has multiple KCMs they will be shown in a
    // tabwidget - defaults() will be called for all of them)

    QWidget * pluginconfig = d->widgetstack->currentWidget ();
    KCModuleProxy * kcm = qobject_cast<KCModuleProxy *>( pluginconfig );
    if( kcm )
    {
        kDebug( 702 ) << "call KCModule::defaults() for the plugins KCM"
            << endl;
        kcm->defaults();
        return;
    }

    // if we get here the visible Widget must be a tabwidget holding more than
    // one KCM
    QList<KCModuleProxy*> kcms = pluginconfig->findChildren<KCModuleProxy*>();
    QListIterator<KCModuleProxy*> it( kcms );
    while ( it.hasNext() )
        it.next()->defaults();

    // FIXME: update changed state
}

// vim: sw=4 sts=4 et

#include "kpluginselector.moc"
#include "kpluginselector_p.moc"
