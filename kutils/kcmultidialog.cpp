/*
   Copyright (c) 2000 Matthias Elter <elter@kde.org>
   Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (c) 2003 Matthias Kretz <kretz@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

#include <qhbox.h>
#include <qcursor.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <klibloader.h>
#include <krun.h>
#include <kprocess.h>
#include <kaboutdata.h>

#include "kcmultidialog.h"
#include "kcmultidialog.moc"
#include "kcmoduleloader.h"
#include <assert.h>

KCMultiDialog::KCMultiDialog(const QString& baseGroup, QWidget *parent, const char *name, bool modal)
  : KDialogBase(IconList, i18n("Configure"), Help | Default |Cancel | Apply | Ok, Ok,
                parent, name, modal, true)
  , _baseGroup(baseGroup)
  , createTreeList( false )
{
    init();
}

KCMultiDialog::KCMultiDialog( int dialogFace, const QString & caption, QWidget * parent, const char * name, bool modal )
    : KDialogBase( dialogFace, caption, Help | Default | Cancel | Apply | Ok |
            User1, Ok, parent, name, modal, true,
            KGuiItem( i18n( "&Reset" ), "undo" ) )
    , createTreeList( false )
{
    showButton( User1, false );;
    if( dialogFace == KDialogBase::TreeList )
        createTreeList = true;
    init();
}

inline void KCMultiDialog::init()
{
    d = 0L;
    enableButton(Apply, false);
    connect(this, SIGNAL(aboutToShowPage(QWidget *)), this, SLOT(slotAboutToShow(QWidget *)));
    setInitialSize(QSize(640,480));
    moduleParentComponents.setAutoDelete( true );
}

KCMultiDialog::~KCMultiDialog()
{
    moduleDict.setAutoDelete(true);
}

void KCMultiDialog::slotDefault()
{
    int curPageIndex = activePageIndex();

    QPtrListIterator<KCModule> it(modules);
    for (; it.current(); ++it)
    {
       if (pageIndex((QWidget *)(*it)->parent()) == curPageIndex)
       {
          (*it)->defaults();
          clientChanged(true);
          return;
       }
    }
}

void KCMultiDialog::slotUser1()
{
    int curPageIndex = activePageIndex();

    QPtrListIterator<KCModule> it( modules );
    for( ; it.current(); ++it )
    {
        if( pageIndex( ( QWidget* )( *it )->parent() ) == curPageIndex )
        {
            ( *it )->load();
            clientChanged( false );
            return;
        }
    }
}

void KCMultiDialog::apply()
{
    QStringList updatedModules;
    for( KCModule * m = modules.first(); m; m = modules.next() )
    {
        kdDebug(710) << k_funcinfo << m->name() << ' ' << ( m->aboutData() ? m->aboutData()->appName() : "" ) << endl;
        if( m->changed() )
        {
            m->save();
            // ### KDE4 remove setChanged workaround
            if( m->changed() )
            {
                kdWarning( 710 ) << "The KCModule says it is changed right "
                    "after it saved its configuration. A call to setChanged(false) in the save() method is missing." << endl;
                m->setChanged( false );
            }
            QStringList * names = moduleParentComponents[ m ];
            kdDebug(710) << k_funcinfo << *names << " saved and added to the list" << endl;
            for( QStringList::ConstIterator it = names->begin(); it != names->end(); ++it )
                if( updatedModules.find( *it ) == updatedModules.end() )
                    updatedModules.append( *it );
        }
    }
    for( QStringList::const_iterator it = updatedModules.begin(); it != updatedModules.end(); ++it )
    {
        kdDebug(710) << k_funcinfo << *it << " " << ( *it ).latin1() << endl;
        emit configCommitted( ( *it ).latin1() );
    }
}

void KCMultiDialog::slotApply()
{
    emit applyClicked();
    apply();
}


void KCMultiDialog::slotOk()
{
    emit okClicked();
    apply();
    accept();
}

void KCMultiDialog::slotHelp()
{
    KURL url( KURL("help:/"), _docPath );

    if (url.protocol() == "help" || url.protocol() == "man" || url.protocol() == "info") {
        KProcess process;
        process << "khelpcenter"
                << url.url();
        process.start(KProcess::DontCare);
		process.detach();
    } else {
        new KRun(url);
    }
}

void KCMultiDialog::clientChanged(bool state)
{
    kdDebug( 710 ) << k_funcinfo << state << endl;
    for( KCModule * m = modules.first(); m; m = modules.next() )
        if( m->changed() )
        {
            enableButton( Apply, true );
            return;
        }
    enableButton( Apply, false );
}

void KCMultiDialog::addModule(const QString& path, bool withfallback)
{
    kdDebug(710) << "KCMultiDialog::addModule " << path << endl;

    KService::Ptr s = KService::serviceByStorageId(path);
    if (!s) {
      kdError() << "Desktop file '" << path << "' not found!" << endl;
      return;
    }

    KCModuleInfo info(s);
    addModule(info, QStringList(), withfallback);
}

void KCMultiDialog::addModule(const KCModuleInfo& moduleinfo,
        QStringList parentmodulenames, bool withfallback)
{
    kdDebug(710) << "KCMultiDialog::addModule " << moduleinfo.moduleName() <<
        endl;

    QHBox* page = 0;
    if (!moduleinfo.service()->noDisplay())
        if( createTreeList )
        {
            parentmodulenames += moduleinfo.moduleName();;
            page = addHBoxPage(parentmodulenames, moduleinfo.comment(),
                    KGlobal::iconLoader()->loadIcon(moduleinfo.icon(), KIcon::Small));
        }
        else
            page = addHBoxPage(moduleinfo.moduleName(), moduleinfo.comment(),
                    KGlobal::iconLoader()->loadIcon(moduleinfo.icon(),
                        KIcon::Desktop, KIcon::SizeMedium));
    if(!page) {
        KCModuleLoader::unloadModule(moduleinfo);
        return;
    }
    moduleDict.insert(page, new LoadInfo(moduleinfo, withfallback));
    if (modules.isEmpty())
        slotAboutToShow(page);
}

void KCMultiDialog::removeModule( const KCModuleInfo& moduleinfo )
{
    kdDebug( 710 ) << k_funcinfo << moduleinfo.moduleName() << endl;
    QPtrDictIterator<LoadInfo> it( moduleDict );
    for( ; it.current(); ++it )
    {
        if( it.current()->info == moduleinfo )
        {
            kdDebug( 710 ) << "found module to remove" << endl;
            QWidget * page = ( QWidget* )it.currentKey();
            delete it.current();
            moduleDict.remove( page );
            delete page;
            break;
        }
    }
}

void KCMultiDialog::show()
{
    if( ! isVisible() )
    {
        // call load() method of all KCMs
        for( QPtrListIterator<KCModule> it( modules ); it.current(); ++it )
            ( *it )->load();
    }
    KDialogBase::show();
}

void KCMultiDialog::slotAboutToShow(QWidget *page)
{
    kdDebug( 710 ) << k_funcinfo << endl;
    LoadInfo *loadInfo = moduleDict[page];
    if (!loadInfo)
    {
        // honor KCModule::buttons
        QObject * obj = page->child( 0, "KCModule" );
        if( ! obj )
            return;
        KCModule * module = ( KCModule* )obj->qt_cast( "KCModule" );
        if( ! module )
            return;
        enableButton( KDialogBase::Help,
                module->buttons() & KCModule::Help );
        enableButton( KDialogBase::Default,
                module->buttons() & KCModule::Default );
       return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    moduleDict.remove(page);

    KCModule *module = KCModuleLoader::loadModule(loadInfo->info, loadInfo->withfallback);

    if (!module)
    {
        QApplication::restoreOverrideCursor();
        KCModuleLoader::showLastLoaderError(this);
        delete loadInfo;
        return;
    }

    QStringList parentComponents = loadInfo->info.service()->property(
            "X-KDE-ParentComponents" ).toStringList();
    kdDebug(710) << k_funcinfo << "ParentComponents=" << parentComponents
        << endl;
    moduleParentComponents.insert( module,
            new QStringList( parentComponents ) );

    connect(module, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));
    module->reparent(page,0,QPoint(0,0),true);

    // honor KCModule::buttons
    enableButton( KDialogBase::Help,
            module->buttons() & KCModule::Help );
    enableButton( KDialogBase::Default,
            module->buttons() & KCModule::Default );

    if( module->changed() )
    {
        kdWarning(710) << "Just loaded a KCModule but it's already changed."
            << endl;
        clientChanged( true );
    }
    //setHelp( docpath, QString::null );
    _docPath = loadInfo->info.docPath();
    modules.append(module);

    //KCGlobal::repairAccels( topLevelWidget() );

    delete loadInfo;

    QApplication::restoreOverrideCursor();
}

// vim: sw=4 et sts=4
