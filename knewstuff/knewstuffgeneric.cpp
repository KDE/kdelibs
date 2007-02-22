/*
    This file is part of KDE.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <QtCore/QFile>
#include <qtextstream.h>
#include <qdir.h>
#include <qprocess.h>

#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <ktar.h>

#include "entry.h"

#include "knewstuffgeneric.h"

using namespace std;

KNewStuffGeneric::KNewStuffGeneric( const QString &type, QWidget *parent )
  : KNewStuff( type, parent )
{
  mConfig = KGlobal::config();
}

KNewStuffGeneric::~KNewStuffGeneric()
{
}

bool KNewStuffGeneric::install( const QString &fileName )
{
  kDebug() << "KNewStuffGeneric::install(): " << fileName << endl;
  QStringList list, list2;

  KConfigGroup cg(mConfig, "KNewStuff");

  QString uncompress = cg.readEntry( "Uncompress" );
  if ( !uncompress.isEmpty() ) {
    kDebug() << "Uncompression method: " << uncompress << endl;
    KTar tar(fileName, uncompress);
    tar.open(QIODevice::ReadOnly);
    const KArchiveDirectory *dir = tar.directory();
    dir->copyTo(destinationPath(0));
    tar.close();
    QFile::remove(fileName);
  }

  QString cmd = cg.readEntry( "InstallationCommand" );
  if ( !cmd.isEmpty() ) {
    kDebug() << "InstallationCommand: " << cmd << endl;
    // XXX use KMacroExpander && KShell
    list = cmd.split( " " );
    for ( QStringList::iterator it = list.begin(); it != list.end(); ++it ) {
        list2 << (*it).replace("%f", fileName);
    }
    QString exe( list2.takeFirst() );
    QProcess::execute( exe, list2 );
  }

  return true;
}

bool KNewStuffGeneric::createUploadFile( const QString & /*fileName*/ )
{
  return false;
}

QString KNewStuffGeneric::destinationPath( KNS::Entry *entry )
{
  QString path, file, target, ext;

  KConfigGroup cg(mConfig, "KNewStuff");

  if ( entry )
  {
    ext = entry->payload().fileName().section('.', 1);
    if ( ! ext.isEmpty() ) ext = '.' + ext;

    target = entry->fullName() + ext;
  }
  else target = "/";
  QString res = cg.readEntry( "StandardResource" );
  if ( res.isEmpty() )
  {
    target = cg.readEntry("TargetDir");
    if ( !target.isEmpty())
    {
      res = "data";
      if ( entry ) target.append('/' + entry->fullName() + ext);
      else target.append("/");
    }
  }
  if ( res.isEmpty() )
  {
    path = cg.readEntry( "InstallPath" );
  }
  if ( res.isEmpty() && path.isEmpty() )
  {
    if ( !entry ) return QString();
    else return KNewStuff::downloadDestination( entry );
  }

  if ( !path.isEmpty() )
  {
    file = QDir::home().path() + '/' + path + '/';
    if ( entry ) file += entry->fullName() + ext;
  }
  else file = KStandardDirs::locateLocal( res.toUtf8() , target );

  return file;
}

QString KNewStuffGeneric::downloadDestination( KNS::Entry *entry )
{
  QString file = destinationPath(entry);

  if ( KStandardDirs::exists( file ) ) {
    int result = KMessageBox::warningContinueCancel( parentWidget(),
        i18n("The file '%1' already exists. Do you want to overwrite it?",
          file ),
        QString(), KGuiItem(i18n("Overwrite")) );
    if ( result == KMessageBox::Cancel ) return QString();
  }

  return file;
}
