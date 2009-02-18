/* This file is part of the KDE project
   Copyright (c) 2004 Jan Schaefer <j_schaef@informatik.uni-kl.de>

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

#include "ksambashare.h"

#include <QtCore/QSet>
#include <QtCore/QFile>
#include <QtCore/QMutableStringListIterator>
#include <QtCore/QTextIStream>

#include <kdirwatch.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>

// Default smb.conf locations
// sorted by priority, most priority first
static const char * const DefaultSambaConfigFilePathList[] =
{
  "/etc/samba/smb.conf",
  "/etc/smb.conf",
  "/usr/local/etc/smb.conf",
  "/usr/local/samba/lib/smb.conf",
  "/usr/samba/lib/smb.conf",
  "/usr/lib/smb.conf",
  "/usr/local/lib/smb.conf"
};
static const int DefaultSambaConfigFilePathListSize = sizeof( DefaultSambaConfigFilePathList ) / sizeof(char*);

class KSambaShare::KSambaSharePrivate
{
public:
  KSambaSharePrivate(KSambaShare *parent);

  void _k_slotFileChange(const QString&);

  bool readSmbConf();
  bool findSmbConf();
  bool load();

  KSambaShare *q;
  QSet<QString> sharedPaths;
  QString smbConf;
};

KSambaShare::KSambaSharePrivate::KSambaSharePrivate(KSambaShare *parent)
    : q(parent)
{
    load();
}


#define FILESHARECONF "/etc/security/fileshare.conf"

bool KSambaShare::KSambaSharePrivate::load()
{
  if (!findSmbConf())
      return false;

  return readSmbConf();
}

/**
 * Try to find the samba config file path
 * First tries the kconfig, then checks
 * several well-known paths
 * @return whether a smb.conf was found.
 **/
bool KSambaShare::KSambaSharePrivate::findSmbConf()
{
  KConfig config( QLatin1String( FILESHARECONF ) );
  const KConfigGroup group( &config, QString() );
  smbConf = group.readEntry( "SMBCONF" );

  if ( QFile::exists( smbConf ) )
    return true;

  bool success = false;
  for( int i = 0; i<DefaultSambaConfigFilePathListSize; ++i )
  {
    const QString filePath( DefaultSambaConfigFilePathList[i] );
    if( QFile::exists( filePath ) )
    {
        smbConf = filePath;
        success = true;
        break;
    }
  }
  
  if( ! success )
    kDebug(7000) << "KSambaShare: Could not find smb.conf!";

  return success;
  
}


/**
 * Reads all path= entries from the smb.conf file
 * and fills the sharedPaths dict with the values
 */
bool KSambaShare::KSambaSharePrivate::readSmbConf()
{
  QFile f(smbConf);

  //kDebug(7000) << smbConf;

  if (!f.open(QIODevice::ReadOnly)) {
    kError() << "KSambaShare: Could not open" << smbConf;
    return false;
  }

  sharedPaths.clear();

  QTextStream s(&f);

  bool continuedLine = false; // is true if the line before ended with a backslash
  QString completeLine;

  while (!s.atEnd())
  {
    QString currentLine = s.readLine().trimmed();

    if (continuedLine) {
      completeLine += currentLine;
      continuedLine = false;
    }
    else
      completeLine = currentLine;

    // is the line continued in the next line ?
    if ( !completeLine.isEmpty() && completeLine[completeLine.length()-1] == '\\' )
    {
      continuedLine = true;
      // remove the ending backslash
      completeLine.truncate( completeLine.length()-1 );
      continue;
    }

    // comments or empty lines
    if (completeLine.isEmpty() ||
        '#' == completeLine[0] ||
        ';' == completeLine[0])
    {
      continue;
    }

    // parameter
    const int i = completeLine.indexOf('=');

    if (i>-1)
    {
      QString name = completeLine.left(i).trimmed().toLower();
      QString value = completeLine.mid(i+1).trimmed();

      if (name == KGlobal::staticQString("path") && !value.isEmpty()) {
        // Handle quotation marks
        if ( value[0] == '"' )
          value.remove(0,1);

        if ( value[value.length()-1] == '"' )
          value.truncate(value.length()-1);

        // Normalize path
        if ( value[value.length()-1] != '/' )
             value += '/';

        sharedPaths.insert(value);
        //kDebug(7000) << "KSambaShare: Found path: " << value;
      }
    }
  }

  f.close();

  return true;

}

KSambaShare::KSambaShare()
  : d(new KSambaSharePrivate(this))
{
  if (QFile::exists(d->smbConf)) {
    KDirWatch::self()->addFile(d->smbConf);
    KDirWatch::self()->addFile(FILESHARECONF);
    connect(KDirWatch::self(), SIGNAL(dirty (const QString&)),this,
   	        SLOT(_k_slotFileChange(const QString&)));
  }
}

KSambaShare::~KSambaShare()
{
  if (QFile::exists(d->smbConf)) {
        KDirWatch::self()->removeFile(d->smbConf);
        KDirWatch::self()->removeFile(FILESHARECONF);
  }
  delete d;
}

QString KSambaShare::smbConfPath() const
{
  return d->smbConf;
}

bool KSambaShare::isDirectoryShared( const QString & path ) const
{
  if(path.isEmpty())
    return false;
  QString fixedPath = path;
  if ( path[path.length()-1] != '/' )
       fixedPath += '/';

  return d->sharedPaths.contains(fixedPath);
}

QStringList KSambaShare::sharedDirectories() const
{
  return d->sharedPaths.values();
}

void KSambaShare::KSambaSharePrivate::_k_slotFileChange( const QString & path )
{
  if (path == smbConf)
     readSmbConf();
  else
  if (path == FILESHARECONF)
     load();

  emit q->changed();
}

KSambaShare* KSambaShare::instance()
{
  K_GLOBAL_STATIC(KSambaShare, _instance)
  return _instance;
}

#include "ksambashare.moc"

