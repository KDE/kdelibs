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
    : KDialogBase( dialogFace, caption, Help | Default | Cancel | Apply | Ok, Ok,
                   parent, name, modal, true )
    , createTreeList( false )
{
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
    modulePrefParent.setAutoDelete( true );
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

void KCMultiDialog::apply()
{
    QStringList updatedModules;
    for( KCModule * m = modules.first(); m; m = modules.next() )
    {
        kdDebug() << k_funcinfo << m->name() << ' ' << ( m->aboutData() ? m->aboutData()->appName() : "" ) << endl;
        if( m->changed() )
        {
            m->save();
            QStringList * names = modulePrefParent[ m ];
            kdDebug() << k_funcinfo << *names << " saved and added to the list" << endl;
            for( QStringList::ConstIterator it = names->begin(); it != names->end(); ++it )
                if( updatedModules.find( *it ) == updatedModules.end() )
                    updatedModules.append( *it );
        }
    }
    for( QStringList::const_iterator it = updatedModules.begin(); it != updatedModules.end(); ++it )
    {
        kdDebug() << k_funcinfo << *it << " " << ( *it ).latin1() << endl;
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
    kdDebug( 1208 ) << k_funcinfo << state << endl;
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
    kdDebug(1208) << "KCMultiDialog::addModule " << path << endl;

    if (!KService::serviceByDesktopPath(path)) {
      kdError() << "Desktop file '" << path << "' not found!" << endl;
      return;
    }

    KCModuleInfo info(path, _baseGroup);
    addModule(info, withfallback);
}

void KCMultiDialog::addModule(const KCModuleInfo& moduleinfo, bool withfallback)
{
    kdDebug(1208) << "KCMultiDialog::addModule " << moduleinfo.moduleName() <<
        " for ParentComponents=" << moduleinfo.parentComponents() << endl;

    QHBox* page = 0;
    if (!moduleinfo.service()->noDisplay())
        if( createTreeList )
            page = addHBoxPage(moduleinfo.moduleNames(), moduleinfo.comment(),
                    KGlobal::iconLoader()->loadIcon(moduleinfo.icon(), KIcon::Small));
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
    {
        kdDebug() << k_funcinfo << moduleinfo.parentComponents() << endl;
        slotAboutToShow(page);
    }
}

void KCMultiDialog::removeModule( const KCModuleInfo& moduleinfo )
{
    kdDebug( 1208 ) << k_funcinfo << moduleinfo.moduleName() << endl;
    QPtrDictIterator<LoadInfo> it( moduleDict );
    for( ; it.current(); ++it )
    {
        if( it.current()->info == moduleinfo )
        {
            kdDebug( 1208 ) << "found module to remove" << endl;
            QWidget * page = ( QWidget* )it.currentKey();
            delete it.current();
            moduleDict.remove( page );
            delete page;
            break;
        }
    }
}

void KCMultiDialog::slotAboutToShow(QWidget *page)
{
    LoadInfo *loadInfo = moduleDict[page];
    if (!loadInfo)
       return;

    kdDebug() << k_funcinfo << loadInfo->info.parentComponents() << endl;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    moduleDict.remove(page);

    KCModule *module = KCModuleLoader::loadModule(loadInfo->info, loadInfo->withfallback);

    kdDebug() << k_funcinfo << loadInfo->info.parentComponents() << endl;

    if (!module)
    {
        QApplication::restoreOverrideCursor();
        KCModuleLoader::showLastLoaderError(this);
        delete loadInfo;
        return;
    }

    kdDebug() << k_funcinfo << "ParentComponents=" << loadInfo->info.parentComponents() << endl;
    modulePrefParent.insert( module, new QStringList( loadInfo->info.parentComponents() ) );
    module->reparent(page,0,QPoint(0,0),true);
    connect(module, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));
    if( module->changed() )
    {
        kdWarning() << "Just loaded a KCModule but it's already changed.";
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
