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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include "kpluginselector.h"
#include "kpluginselector_p.h"

#include <qvbox.h>
#include <qlabel.h>
#include <qstrlist.h>
#include <qfile.h>
#include <qstring.h>
#include <qlayout.h>
#include <qptrlist.h>
#include <qwidgetstack.h>
#include <qcursor.h>
#include <qapplication.h>
#include <qobjectlist.h>
#include <qcstring.h>

#include <kdebug.h>
#include <klocale.h>
#include <klistview.h>
#include <ksimpleconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <ktabctl.h>
#include <kcmoduleinfo.h>
#include <qvaluelist.h>
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
#include <qptrdict.h>
#include <qstringlist.h>

struct KPluginSelectionWidget::KPluginSelectionWidgetPrivate
{
    KPluginSelectionWidgetPrivate( const QString & _instanceName,
            KPluginSelector * _kps, const QString & _cat,
            KConfigGroup * _config )
        : instanceName( _instanceName )
        , widgetstack( 0 )
        , kps( _kps )
        , config( _config )
        , catname( _cat )
        , currentplugininfo( 0 )
        , currentchecked( false )
        , changed( 0 )
    {
        moduleParentComponents.setAutoDelete( true );
    }

    ~KPluginSelectionWidgetPrivate()
    {
        delete config;
    }

    QMap<QCheckListItem*, KPluginInfo*> pluginInfoMap;

    QString instanceName; // isNull() for non-KParts plugins
    QWidgetStack * widgetstack;
    KPluginSelector * kps;
    KConfigGroup * config;

    QDict<KCModuleInfo> pluginconfigmodules;
    QMap<QString, int> widgetIDs;
    QString catname;
    QValueList<KCModule*> modulelist;
    QPtrDict<QStringList> moduleParentComponents;

    KPluginInfo * currentplugininfo;
    bool currentchecked;
    int changed;
};

KPluginSelectionWidget::KPluginSelectionWidget( const QString & instanceName,
        KPluginSelector * kps, QWidget * parent, const QString & catname,
        const QString & category, KConfigGroup * config, const char * name )
    : QWidget( parent, name )
, d( new KPluginSelectionWidgetPrivate( instanceName, kps, catname, config ) )
{
    init( kpartsPluginInfos(), category );
}

KPluginSelectionWidget::KPluginSelectionWidget(
        const QValueList<KPluginInfo*> & plugininfos, KPluginSelector * kps,
        QWidget * parent, const QString & catname, const QString & category,
        KConfigGroup * config, const char * name )
    : QWidget( parent, name )
, d( new KPluginSelectionWidgetPrivate( 0, kps, catname, config ) )
{
    init( plugininfos, category );
}

inline QString KPluginSelectionWidget::catName() const
{
    return d->catname;
}

QValueList<KPluginInfo*> KPluginSelectionWidget::kpartsPluginInfos() const
{
    if( d->instanceName.isNull() )
    {
        QValueList<KPluginInfo*> list;
        return list; //nothing
    }

    QStringList desktopfilenames = KGlobal::dirs()->findAllResources( "data",
            d->instanceName + "/kpartplugins/*.desktop", true, false );
    return KPluginInfo::fromFiles( desktopfilenames );
}

void KPluginSelectionWidget::init( const QValueList<KPluginInfo*> & plugininfos,
        const QString & category )
{
    // setup Widgets
    ( new QVBoxLayout( this, 0, KDialog::spacingHint() ) )->setAutoAdd( true );
    KListView * listview = new KListView( this );
    connect( listview, SIGNAL( pressed( QListViewItem * ) ), this,
            SLOT( executed( QListViewItem * ) ) );
    connect( listview, SIGNAL( spacePressed( QListViewItem * ) ), this,
            SLOT( executed( QListViewItem * ) ) );
    connect( listview, SIGNAL( returnPressed( QListViewItem * ) ), this,
            SLOT( executed( QListViewItem * ) ) );
    connect( listview, SIGNAL( selectionChanged( QListViewItem * ) ), this,
            SLOT( executed( QListViewItem * ) ) );
    listview->setAcceptDrops( false );
    listview->setSelectionModeExt( KListView::Single );
    listview->setAllColumnsShowFocus( true );
    listview->addColumn( i18n( "Name" ) );
    listview->addColumn( i18n( "Description" ) );
    listview->addColumn( i18n( "Author" ) );
    listview->addColumn( i18n( "Version" ) );
    listview->addColumn( i18n( "License" ) );
    for( QValueList<KPluginInfo*>::ConstIterator it = plugininfos.begin();
            it != plugininfos.end(); ++it )
    {
        if( !( *it )->isHidden() &&
                ( category.isNull() || ( *it )->category() == category ) )
        {
            QCheckListItem * item = new QCheckListItem( listview,
                    ( *it )->name(), QCheckListItem::CheckBox );
            if( ! ( *it )->icon().isEmpty() )
                item->setPixmap( 0, SmallIcon( ( *it )->icon(), IconSize( KIcon::Small ) ) );
            item->setText( 1, ( *it )->comment() );
            item->setText( 2, ( *it )->author()  );
            item->setText( 3, ( *it )->version() );
            item->setText( 4, ( *it )->license() );
            item->setOn( ( *it )->pluginEnabled() );
            d->pluginInfoMap.insert( item, *it );
        }
    }

    // widgetstack
    d->widgetstack = d->kps->widgetStack();
    load();
}

KPluginSelectionWidget::~KPluginSelectionWidget()
{
    delete d;
}

bool KPluginSelectionWidget::pluginIsLoaded( const QString & pluginname ) const
{
    for( QMap<QCheckListItem*, KPluginInfo*>::ConstIterator it =
            d->pluginInfoMap.begin(); it != d->pluginInfoMap.end(); ++it )
        if( it.data()->pluginname() == pluginname )
            return it.data()->pluginEnabled();
    return false;
}

QWidget * KPluginSelectionWidget::insertKCM( QWidget * parent,
        const KCModuleInfo & moduleinfo )
{
    KCModule * module = KCModuleLoader::loadModule( moduleinfo, false, parent );
    if( !module )
    {
        //FIXME: not very verbose
        QLabel * label = new QLabel( i18n( "Error" ), parent );
        label->setAlignment( Qt::AlignCenter );

        QApplication::restoreOverrideCursor();
        KCModuleLoader::showLastLoaderError( this );
        QApplication::setOverrideCursor( Qt::WaitCursor );

        return label;
    }
    // add the KCM to the list so that we can call load/save/defaults on it
    d->modulelist.append( module );
    d->moduleParentComponents.insert( module, new QStringList( moduleinfo.parentComponents() ) );
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
        d->widgetIDs[ plugininfo->pluginname() ] = id;

        for( QValueList<KService::Ptr>::ConstIterator it =
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
            d->widgetIDs[ plugininfo->pluginname() ] = id;
        }
    }
    QApplication::restoreOverrideCursor();
}

void KPluginSelectionWidget::updateConfigPage( KPluginInfo * plugininfo,
        bool checked )
{
    kdDebug( 702 ) << k_funcinfo << endl;
    d->currentplugininfo = plugininfo;
    d->currentchecked = checked;

    if( 0 == plugininfo )
    {
        d->kps->configPage( 1 );
        return;
    }

    // if no widget exists for the plugin (yet)
    if( !d->widgetIDs.contains( plugininfo->pluginname() ) )
    {
        if( !plugininfo->kcmServices().empty() )
            embeddPluginKCMs( plugininfo, checked );
        else
            //else no config...
            d->kps->configPage( 1 );
    }
    else
    {
        // the page already exists
        int id = d->widgetIDs[ plugininfo->pluginname() ];
        d->kps->configPage( id );
        d->widgetstack->widget( id )->setEnabled( checked );
    }
}

void KPluginSelectionWidget::clientChanged( bool didchange )
{
    kdDebug( 702 ) << k_funcinfo << endl;
    d->changed += didchange ? 1 : -1;
    if( d->changed == 1 )
        emit changed( true );
    else if( d->changed == 0 )
        emit changed( false );
    else if( d->changed < 0 )
        kdError( 702 ) << "negative changed value: " << d->changed << endl;
}

void KPluginSelectionWidget::executed( QListViewItem * item )
{
    kdDebug( 702 ) << k_funcinfo << endl;
    if( item == 0 )
        return;
    if( item->rtti() != 1 ) //check for a QCheckListItem
        return;

    //FIXME:
    ++d->changed;
    if( d->changed == 1 )
        emit changed( true );

    QCheckListItem * citem = static_cast<QCheckListItem *>( item );
    bool checked = citem->isOn();
    kdDebug( 702 ) << "it's a " << ( checked ? "checked" : "unchecked" )
        << " QCheckListItem" << endl;

    KPluginInfo * info = d->pluginInfoMap[ citem ];
    if( info->isHidden() )
        kdFatal( 702 ) << "bummer" << endl;
    checkDependencies( info );

    updateConfigPage( info, checked );
}

void KPluginSelectionWidget::load()
{
    kdDebug( 702 ) << k_funcinfo << endl;

    for( QMap<QCheckListItem*, KPluginInfo*>::Iterator it =
            d->pluginInfoMap.begin(); it != d->pluginInfoMap.end(); ++it )
    {
        KPluginInfo * info = it.data();
        info->load( d->config );
        it.key()->setOn( info->pluginEnabled() );
    }
    updateConfigPage( d->currentplugininfo, d->currentchecked );
    // TODO: update changed state
}

void KPluginSelectionWidget::save()
{
    kdDebug( 702 ) << k_funcinfo << endl;

    for( QMap<QCheckListItem*, KPluginInfo*>::Iterator it =
            d->pluginInfoMap.begin(); it != d->pluginInfoMap.end(); ++it )
    {
        KPluginInfo * info = it.data();
        bool checked = it.key()->isOn();
        info->setPluginEnabled( checked );
        info->save( d->config );
    }
    QStringList updatedModules;
    for( QValueList<KCModule*>::Iterator it = d->modulelist.begin();
            it != d->modulelist.end(); ++it )
        if( ( *it )->changed() )
        {
            ( *it )->save();
            QStringList * names = d->moduleParentComponents[ *it ];
            for( QStringList::ConstIterator nameit = names->begin();
                    nameit != names->end(); ++nameit )
                if( updatedModules.find( *nameit ) == updatedModules.end() )
                    updatedModules.append( *nameit );
        }
    for( QStringList::ConstIterator it = updatedModules.begin(); it != updatedModules.end(); ++it )
        emit configCommitted( ( *it ).latin1() );

    updateConfigPage( d->currentplugininfo, d->currentchecked );
    kdDebug( 702 ) << "syncing config file" << endl;
    d->config->sync();
    d->changed = 0;
    emit changed( false );
}

void KPluginSelectionWidget::defaults()
{
    kdDebug( 702 ) << k_funcinfo << endl;

    for( QMap<QCheckListItem*, KPluginInfo*>::Iterator it =
            d->pluginInfoMap.begin(); it != d->pluginInfoMap.end(); ++it )
    {
        it.data()->defaults();
        it.key()->setOn( it.data()->pluginEnabled() );
    }
    updateConfigPage( d->currentplugininfo, d->currentchecked );
    // TODO: update changed state
}

void KPluginSelectionWidget::checkDependencies( const KPluginInfo * info )
{
    if( info->dependencies().isEmpty() )
        return;

    for( QStringList::ConstIterator it = info->dependencies().begin();
            it != info->dependencies().end(); ++it )
        for( QMap<QCheckListItem*,
                KPluginInfo*>::Iterator infoIt = d->pluginInfoMap.begin();
                infoIt != d->pluginInfoMap.end(); ++infoIt )
            if( infoIt.data()->pluginname() == *it )
            {
                if( !infoIt.key()->isOn() )
                {
                    infoIt.key()->setOn( true );
                    checkDependencies( infoIt.data() );
                }
                continue;
            }
}

class KPluginSelector::KPluginSelectorPrivate
{
    public:
        KPluginSelectorPrivate()
            : frame( 0 )
            , tabwidget( 0 )
            , widgetstack( 0 )
            {
            }

        QFrame * frame;
        KTabWidget * tabwidget;
        QWidgetStack * widgetstack;
        QValueList<KPluginSelectionWidget *> pswidgets;
};

    KPluginSelector::KPluginSelector( QWidget * parent, const char * name )
    : QWidget( parent, name )
, d( new KPluginSelectorPrivate )
{
    QBoxLayout * vbox = new QVBoxLayout( this, 0, KDialog::spacingHint() );
    vbox->setAutoAdd( true );
    QSplitter * splitter = new QSplitter( Qt::Vertical, this, "PluginSelectionWidget splitter" );
    d->frame = new QFrame( splitter, "KPluginSelector top frame" );
    d->frame->setFrameStyle( QFrame::NoFrame );
    ( new QVBoxLayout( d->frame, 0, KDialog::spacingHint() ) )->setAutoAdd( true );

    // widgetstack
    d->widgetstack = new QWidgetStack( splitter,
            "KPluginSelector Config Pages" );
    d->widgetstack->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    d->widgetstack->setMinimumSize( 200, 200 );

    splitter->setOpaqueResize( true );

    QLabel * label = new QLabel( i18n( "Select a plugin to configure it" ),
            d->widgetstack );
    label->setAlignment( Qt::AlignCenter );
    d->widgetstack->addWidget( label, 1 );

    configPage( 1 );
}

KPluginSelector::~KPluginSelector()
{
    delete d;
}

void KPluginSelector::checkNeedForTabWidget()
{
    kdDebug( 702 ) << k_funcinfo << endl;
    if( ! d->tabwidget && d->pswidgets.size() == 1 )
    {
        kdDebug( 702 ) << "no TabWidget and one KPluginSelectionWidget" << endl;
        // there's only one KPluginSelectionWidget yet, we need a TabWidget
        KPluginSelectionWidget * w = d->pswidgets.first();
        if( w )
        {
            kdDebug( 702 ) << "create TabWidget" << endl;
            d->tabwidget = new KTabWidget( d->frame, "KPluginSelector TabWidget" );
            w->reparent( d->tabwidget, QPoint( 0, 0 ) );
            d->tabwidget->addTab( w, w->catName() );
        }
    }
}

void KPluginSelector::addPlugins( const QString & instanceName,
        const QString & catname, const QString & category, KConfig * config )
{
    checkNeedForTabWidget();
    // FIXME: mem leak: the KSimpleConfig object needs to be deleted
    KConfigGroup * cfgGroup = new KConfigGroup( config ? config :
            new KSimpleConfig( instanceName ), "KParts Plugins" );
    kdDebug( 702 ) << k_funcinfo << "cfgGroup = " << cfgGroup << endl;
    KPluginSelectionWidget * w;
    if( d->tabwidget )
    {
        w = new KPluginSelectionWidget( instanceName, this,
                d->tabwidget, catname, category, cfgGroup );
        d->tabwidget->addTab( w, catname );
    }
    else
        w = new KPluginSelectionWidget( instanceName, this, d->frame,
                catname, category, cfgGroup );
    connect( w, SIGNAL( changed( bool ) ), this, SIGNAL( changed( bool ) ) );
    connect( w, SIGNAL( configCommitted( const QCString & ) ), this,
            SIGNAL( configCommitted( const QCString & ) ) );
    d->pswidgets += w;
}

void KPluginSelector::addPlugins( const KInstance * instance, const QString &
        catname, const QString & category, KConfig * config )
{
    addPlugins( instance->instanceName(), catname, category, config );
}

void KPluginSelector::addPlugins( const QValueList<KPluginInfo*> & plugininfos,
        const QString & catname, const QString & category, KConfig * config )
{
    checkNeedForTabWidget();
    KConfigGroup * cfgGroup = new KConfigGroup( config ? config : KGlobal::config(), "Plugins" );
    kdDebug( 702 ) << k_funcinfo << "cfgGroup = " << cfgGroup << endl;
    KPluginSelectionWidget * w;
    if( d->tabwidget )
    {
        w = new KPluginSelectionWidget( plugininfos, this,
                d->tabwidget, catname, category, cfgGroup );
        d->tabwidget->addTab( w, catname );
    }
    else
        w = new KPluginSelectionWidget( plugininfos, this, d->frame,
                catname, category, cfgGroup );
    connect( w, SIGNAL( changed( bool ) ), this, SIGNAL( changed( bool ) ) );
    connect( w, SIGNAL( configCommitted( const QCString & ) ), this,
            SIGNAL( configCommitted( const QCString & ) ) );
    d->pswidgets += w;
}

QWidgetStack * KPluginSelector::widgetStack()
{
    return d->widgetstack;
}

inline void KPluginSelector::configPage( int id )
{
    if( id == 1 )
        // no config page
        d->widgetstack->hide();
    else
        d->widgetstack->show();

    d->widgetstack->raiseWidget( id );
}

void KPluginSelector::load()
{
    for( QValueList<KPluginSelectionWidget *>::Iterator it =
            d->pswidgets.begin(); it != d->pswidgets.end(); ++it )
    {
        ( *it )->load();
    }
}

void KPluginSelector::save()
{
    for( QValueList<KPluginSelectionWidget *>::Iterator it =
            d->pswidgets.begin(); it != d->pswidgets.end(); ++it )
    {
        ( *it )->save();
    }
}

void KPluginSelector::defaults()
{
    for( QValueList<KPluginSelectionWidget *>::Iterator it =
            d->pswidgets.begin(); it != d->pswidgets.end(); ++it )
    {
        ( *it )->defaults();
    }
}

// vim: sw=4 sts=4 et

#include "kpluginselector.moc"
#include "kpluginselector_p.moc"
