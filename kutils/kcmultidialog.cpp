/*
   Copyright (c) 2000 Matthias Elter <elter@kde.org>
   Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (c) 2003,2006 Matthias Kretz <kretz@kde.org>
   Copyright (c) 2004 Frans Englich <frans.englich@telia.com>
   Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#include <QStringList>
#include <QProcess>

#include <kauthorized.h>
#include <kguiitem.h>
#include <khbox.h>
#include <kicon.h>
#include <klocale.h>
#include <kpagewidgetmodel.h>
#include <kpushbutton.h>
#include <krun.h>

#include "kcmoduleloader.h"
#include "kcmoduleproxy.h"

#include "kcmultidialog.h"

class KCMultiDialog::Private
{
  public:

    Private( KCMultiDialog *_parent )
      : currentModule( 0 ),
        parent( _parent )
    {
    }

    KCModuleProxy* currentModule;
    KCMultiDialog* parent;

    struct CreatedModule
    {
      KCModuleProxy *kcm;
      KPageWidgetItem *item;
      QStringList componentNames;
    };

    typedef QList<CreatedModule> ModuleList;
    ModuleList modules;

    void _k_slotCurrentPageChanged( KPageWidgetItem* );
    void _k_clientChanged( bool state );
    void _k_dialogClosed();
};

void KCMultiDialog::Private::_k_slotCurrentPageChanged( KPageWidgetItem *item )
{
  kDebug(710) << k_funcinfo << endl;

  if ( !item )
    return;

  KCModuleProxy *module = 0;
  for ( int i = 0; i < modules.count(); ++i ) {
    if ( modules[ i ].item == item ) {
      module = modules[ i ].kcm;
      break;
    }
  }

  if ( !module )
    return;
  kDebug(710) << "found module for page: " << module->moduleInfo().moduleName() << endl;

  currentModule = module;

  parent->enableButton( KDialog::Help, currentModule->buttons() & KCModule::Help );
  parent->enableButton( KDialog::Default, currentModule->buttons() & KCModule::Default );
}

void KCMultiDialog::Private::_k_clientChanged( bool )
{
  for ( int i = 0; i < modules.count(); ++i ) {
    if ( modules[ i ].kcm->changed() ) {
      parent->enableButton( Apply, true );
      return;
    }
  }

  parent->enableButton( Apply, false );
}

void KCMultiDialog::Private::_k_dialogClosed()
{
  kDebug(710) << k_funcinfo << endl;

  /**
   * If we don't delete them, the DCOP registration stays, and trying to load the KCMs
   * in other situations will lead to "module already loaded in Foo," while to the user
   * doesn't appear so(the dialog is hidden)
   */
  for ( int i = 0; i < modules.count(); ++i )
    modules[ i ].kcm->deleteClient();
}


KCMultiDialog::KCMultiDialog( QWidget *parent )
  : KPageDialog( parent ),
    d( new Private( this ) )
{
  setFaceType( Auto );
  setCaption( i18n("Configure") );
  setButtons( Help | Default |Cancel | Apply | Ok | User1 );
  setButtonGuiItem( User1, KStdGuiItem::reset() );
  setDefaultButton( Ok );
  setModal( false );
  showButtonSeparator( true );

  connect( this, SIGNAL( finished() ), SLOT( _k_dialogClosed() ) );

  showButton( User1, false );
  enableButton( Apply, false );

  connect( this, SIGNAL( currentPageChanged( KPageWidgetItem*, KPageWidgetItem* ) ),
           this, SLOT( _k_slotCurrentPageChanged( KPageWidgetItem* ) ) );

  connect( this, SIGNAL( applyClicked() ), SLOT( slotApplyClicked() ) );
  connect( this, SIGNAL( okClicked() ), SLOT( slotOkClicked() ) );
  connect( this, SIGNAL( defaultClicked() ), SLOT( slotDefaultClicked() ) );
  connect( this, SIGNAL( helpClicked() ), SLOT( slotHelpClicked() ) );
  connect( this, SIGNAL( user1Clicked() ), SLOT( slotUser1Clicked() ) );

  setInitialSize( QSize( 640, 480 ) );
}

KCMultiDialog::~KCMultiDialog()
{
  delete d;
}

void KCMultiDialog::slotDefaultClicked()
{
  const KPageWidgetItem *item = currentPage();
  if ( !item )
    return;

  for ( int i = 0; i < d->modules.count(); ++i ) {
    if ( d->modules[ i ].item == item ) {
      d->modules[ i ].kcm->defaults();
      d->_k_clientChanged( true );
      return;
    }
  }
}

void KCMultiDialog::slotUser1Clicked()
{
  const KPageWidgetItem *item = currentPage();
  if ( !item )
    return;

  for ( int i = 0; i < d->modules.count(); ++i ) {
    if ( d->modules[ i ].item == item ) {
      d->modules[ i ].kcm->load();
      d->_k_clientChanged( false );
      return;
    }
  }
}

void KCMultiDialog::apply()
{
  QStringList updatedComponents;

  for ( int i = 0; i < d->modules.count(); ++i ) {
    KCModuleProxy *proxy = d->modules[ i ].kcm;

    if ( proxy->changed() ) {
      proxy->save();

      /**
       * Add name of the components the kcm belongs to the list
       * of updated components.
       */
      const QStringList componentNames = d->modules[ i ].componentNames;
      for ( int j = 0; j < componentNames.count(); ++j ) {
        if ( !updatedComponents.contains( componentNames[ j ] ) )
          updatedComponents.append( componentNames[ j ] );
      }
    }
  }

  /**
   * Send the configCommitted signal for every updated component.
   */
  for ( int i = 0; i < updatedComponents.count(); ++i )
    emit configCommitted( updatedComponents[ i ].toLatin1() );

  emit configCommitted();
}

void KCMultiDialog::slotApplyClicked()
{
  setButtonFocus( Apply );

  apply();
}


void KCMultiDialog::slotOkClicked()
{
  setButtonFocus( Ok );

  apply();
  accept();
}

void KCMultiDialog::slotHelpClicked()
{
  const KPageWidgetItem *item = currentPage();
  if ( !item )
    return;

  QString docPath;
  for ( int i = 0; i < d->modules.count(); ++i ) {
    if ( d->modules[ i ].item == item ) {
      docPath = d->modules[ i ].kcm->moduleInfo().docPath();
      break;
    }
  }

  KUrl docUrl( KUrl( "help:/" ), docPath );
  if ( docUrl.protocol() == "help" || docUrl.protocol() == "man" || docUrl.protocol() == "info" ) {
    QProcess::startDetached("khelpcenter", QStringList() << docUrl.url());
  } else {
    new KRun( docUrl, this );
  }
}


KPageWidgetItem* KCMultiDialog::addModule( const QString& path )
{
  QString complete = path;

  if ( !path.endsWith( ".desktop" ) )
    complete += ".desktop";

  KService::Ptr service = KService::serviceByStorageId( complete );

  return addModule( KCModuleInfo( service ), 0 );
}

KPageWidgetItem* KCMultiDialog::addModule( const KCModuleInfo& moduleInfo,
                                           KPageWidgetItem *parentItem )
{
  if ( !moduleInfo.service() )
    return 0;

  //KAuthorized::authorizeControlModule( moduleInfo.service()->menuId() ) is
  //checked in noDisplay already
  if ( moduleInfo.service()->noDisplay() )
    return 0;

  KHBox *widget = new KHBox();

  KPageWidgetItem *item = new KPageWidgetItem( widget, moduleInfo.moduleName() );
  item->setHeader( moduleInfo.comment() );
  item->setIcon( KIcon( moduleInfo.icon() ) );

  if ( parentItem )
    addSubPage( parentItem, item );
  else
    addPage( item );

  KCModuleProxy *kcm = new KCModuleProxy( moduleInfo, widget );

  connect( kcm, SIGNAL( changed( bool ) ), this, SLOT( _k_clientChanged( bool ) ) );


  Private::CreatedModule cm;
  cm.kcm = kcm;
  cm.item = item;
  cm.componentNames = moduleInfo.service()->property( "X-KDE-ParentComponents" ).toStringList();
  d->modules.append( cm );

  if ( d->modules.count() == 1 )
    setCurrentPage( item );

  return item;
}

void KCMultiDialog::clear()
{
  kDebug( 710 ) << k_funcinfo << endl;

  for ( int i = 0; i < d->modules.count(); ++i ) {
    removePage( d->modules[ i ].item );
    delete d->modules[ i ].kcm;
  }

  d->modules.clear();

  d->_k_clientChanged( false );
}



#include "kcmultidialog.moc"
