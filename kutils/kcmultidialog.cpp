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
#include "kcmoduleproxy.h"
#include <assert.h>
#include <qlayout.h>

KCMultiDialog::KCMultiDialog(QWidget *parent, const char *name, bool modal)
    : KDialogBase(IconList, i18n("Configure"), Help | Default |Cancel | Apply |
            Ok | User1, Ok, parent, name, modal, true,
            KGuiItem( i18n( "&Reset" ), "undo" ) )
    , dialogface( IconList )
{
    showButton( User1, false );;
    init();
}

KCMultiDialog::KCMultiDialog( int dialogFace, const QString & caption, QWidget * parent, const char * name, bool modal )
    : KDialogBase( dialogFace, caption, Help | Default | Cancel | Apply | Ok |
            User1, Ok, parent, name, modal, true,
            KGuiItem( i18n( "&Reset" ), "undo" ) )
    , dialogface( dialogFace )
{
    showButton( User1, false );;
    init();
}

KCMultiDialog::KCMultiDialog( int dialogFace, const KGuiItem &user2,
        const KGuiItem &user3, int buttonMask, const QString &caption,
        QWidget *parent, const char *name, bool modal )
    : KDialogBase( dialogFace, caption, buttonMask | Help | Default | Cancel |
            Apply | Ok | User1, Ok, parent, name, modal, true,
            KGuiItem( i18n( "&Reset" ), "undo" ), user2, user3 )
    , dialogface( dialogFace )
{
   showButton( User1, false );;
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
    OrphanMap::Iterator end2 = m_orphanModules.end();
    for( OrphanMap::Iterator it = m_orphanModules.begin(); it != end2; ++it )
        delete ( *it );
}

void KCMultiDialog::slotDefault()
{
    int curPageIndex = activePageIndex();

    ModuleList::Iterator end = m_modules.end();
    for( ModuleList::Iterator it = m_modules.begin(); it != end; ++it )
        if( pageIndex( ( QWidget * )( *it ).kcm->parent() ) == curPageIndex )
        {
          ( *it ).kcm->defaults();
          clientChanged( true );
          return;
        }
}

void KCMultiDialog::slotUser1()
{
    int curPageIndex = activePageIndex();

    ModuleList::Iterator end = m_modules.end();
    for( ModuleList::Iterator it = m_modules.begin(); it != end; ++it )
        if( pageIndex( ( QWidget * )( *it ).kcm->parent() ) == curPageIndex )
        {
            ( *it ).kcm->load();
            clientChanged( false );
            return;
        }
}

void KCMultiDialog::apply()
{
    QStringList updatedModules;
    ModuleList::Iterator end = m_modules.end();
    for( ModuleList::Iterator it = m_modules.begin(); it != end; ++it )
    {
        KCModuleProxy * m = ( *it ).kcm;
        //kdDebug(710) << k_funcinfo << m->name() << ' ' <<
        //    ( m->aboutData() ? m->aboutData()->appName() : "" ) << endl;
        if( m->changed() )
        {
            m->save();
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
    emit configCommitted();
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
    QString docPath;

    int curPageIndex = activePageIndex();
    ModuleList::Iterator end = m_modules.end();
    for( ModuleList::Iterator it = m_modules.begin(); it != end; ++it )
        if( pageIndex( ( QWidget * )( *it ).kcm->parent() ) == curPageIndex )
        {
            docPath = ( *it ).kcm->moduleInfo().docPath();
            break;
        }

    KURL url( KURL("help:/"), docPath );

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
    ModuleList::Iterator end = m_modules.end();
    for( ModuleList::Iterator it = m_modules.begin(); it != end; ++it )
        if( ( *it ).kcm->changed() )
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

    QFrame* page = 0;
    if (!moduleinfo.service()->noDisplay())
        switch( dialogface )
        {
            case TreeList:
                parentmodulenames += moduleinfo.moduleName();
                page = addHBoxPage( parentmodulenames, moduleinfo.comment(),
                        SmallIcon( moduleinfo.icon(),
                            IconSize( KIcon::Small ) ) );
                break;
            case IconList:
                page = addHBoxPage( moduleinfo.moduleName(),
                        moduleinfo.comment(), DesktopIcon( moduleinfo.icon(),
                            KIcon::SizeMedium ) );
                break;
            case Plain:
                page = plainPage();
                ( new QHBoxLayout( page ) )->setAutoAdd( true );
                break;
            default:
                kdError( 710 ) << "unsupported dialog face for KCMultiDialog"
                    << endl;
                break;
        }
    if(!page) {
        KCModuleLoader::unloadModule(moduleinfo);
        return;
    }
    KCModuleProxy * module;
    if( m_orphanModules.contains( moduleinfo.service() ) )
    {
        // the KCModule already exists - it was removed from the dialog in
        // removeAllModules
        module = m_orphanModules[ moduleinfo.service() ];
        m_orphanModules.remove( moduleinfo.service() );
        kdDebug( 710 ) << "use KCModule from the list of orphans for " <<
            moduleinfo.moduleName() << ": " << module << endl;

        module->reparent( page, 0, QPoint( 0, 0 ), true );

        if( module->changed() )
            clientChanged( true );

        if( activePageIndex() == -1 )
            showPage( pageIndex( page ) );
    }
    else
    {
        module = new KCModuleProxy( moduleinfo, withfallback, page );
        QStringList parentComponents = moduleinfo.service()->property(
                "X-KDE-ParentComponents" ).toStringList();
        kdDebug(710) << k_funcinfo << "ParentComponents=" << parentComponents
            << endl;
        moduleParentComponents.insert( module,
                new QStringList( parentComponents ) );

        connect(module, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));

        if( m_modules.count() == 0 )
            aboutToShowPage( page );
    }
    CreatedModule cm;
    cm.kcm = module;
    cm.service = moduleinfo.service();
    m_modules.append( cm );
}

void KCMultiDialog::removeAllModules()
{
    kdDebug( 710 ) << k_funcinfo << endl;
    ModuleList::Iterator end = m_modules.end();
    for( ModuleList::Iterator it = m_modules.begin(); it != end; ++it )
    {
        kdDebug( 710 ) << "remove 2" << endl;
        KCModuleProxy * kcm = ( *it ).kcm;
        QObject * page = kcm->parent();
        kcm->hide();
        if( page )
        {
            // I hate this
            kcm->reparent( 0, QPoint( 0, 0 ), false );
            delete page;
        }
        m_orphanModules[ ( *it ).service ] = kcm;
        kdDebug( 710 ) << "added KCModule to the list of orphans: " <<
            kcm << endl;
    }
    m_modules.clear();
    // all modules are gone, none can be changed
    clientChanged( false );
}

void KCMultiDialog::show()
{
    if( ! isVisible() )
    {
        // call load() method of all KCMs
        ModuleList::Iterator end = m_modules.end();
        for( ModuleList::Iterator it = m_modules.begin(); it != end; ++it )
            ( *it ).kcm->load();
    }
    KDialogBase::show();
}

void KCMultiDialog::slotAboutToShow(QWidget *page)
{
    kdDebug( 710 ) << k_funcinfo << endl;
    // honor KCModule::buttons
    QObject * obj = page->child( 0, "KCModuleProxy" );
    if( ! obj )
        return;
    KCModuleProxy * module = ( KCModuleProxy* )obj->qt_cast(
            "KCModuleProxy" );
    if( ! module )
        return;
    // TODO: if the dialogface is Plain we should hide the buttons instead of
    // disabling
    enableButton( KDialogBase::Help,
            module->buttons() & KCModule::Help );
    enableButton( KDialogBase::Default,
            module->buttons() & KCModule::Default );
}

// vim: sw=4 et sts=4
