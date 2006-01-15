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

#include <QHash>
#include <qfile.h>
#include <qstringlist.h>
#include <qtextstream.h>

#include <kdirwatch.h>
#include <kstaticdeleter.h>
#include <kdebug.h>
#include <ksimpleconfig.h>

#include "ksambashare.h"

class KSambaSharePrivate
{
public:
  KSambaSharePrivate();
  
  bool readSmbConf();
  bool findSmbConf();
  bool load();
  
  QHash<QString,bool> sharedPaths;
  QString smbConf;
};

KSambaSharePrivate::KSambaSharePrivate() 
{
    load();
}  


#define FILESHARECONF "/etc/security/fileshare.conf"

bool KSambaSharePrivate::load() {
  if (!findSmbConf())
      return false;
      
  return readSmbConf();
}

/**
 * Try to find the samba config file path
 * First tries the kconfig, then checks
 * several well-known paths
 * @return wether a smb.conf was found.
 **/
bool KSambaSharePrivate::findSmbConf() {
  KSimpleConfig config(QLatin1String(FILESHARECONF),true);
  smbConf = config.readEntry("SMBCONF");

  if ( QFile::exists(smbConf) )
    return true;

  if ( QFile::exists("/etc/samba/smb.conf") )
    smbConf = "/etc/samba/smb.conf";
  else
  if ( QFile::exists("/etc/smb.conf") )
    smbConf = "/etc/smb.conf";
  else
  if ( QFile::exists("/usr/local/samba/lib/smb.conf") )
    smbConf = "/usr/local/samba/lib/smb.conf";
  else
  if ( QFile::exists("/usr/samba/lib/smb.conf") )
    smbConf = "/usr/samba/lib/smb.conf";
  else
  if ( QFile::exists("/usr/lib/smb.conf") )
    smbConf = "/usr/lib/smb.conf";
  else
  if ( QFile::exists("/usr/local/lib/smb.conf") )
    smbConf = "/usr/local/lib/smb.conf";
  else {
    kdDebug(7000) << "KSambaShare: Could not found smb.conf!" << endl;
    return false;
  }
      
  return true;
}


/**
 * Reads all path= entries from the smb.conf file
 * and fills the sharedPaths dict with the values
 */
bool KSambaSharePrivate::readSmbConf() {
  QFile f(smbConf);

  kdDebug(7000) << "KSambaShare::readSmbConf " << smbConf << endl;
  
  if (!f.open(QIODevice::ReadOnly)) {
    kdError() << "KSambaShare: Could not open " << smbConf << endl;
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
    int i = completeLine.find('=');

    if (i>-1)
    {
      QString name = completeLine.left(i).trimmed().toLower();
      QString value = completeLine.mid(i+1).trimmed();

      if (name == KGlobal::staticQString("path")) {
        // Handle quotation marks
        if ( value[0] == '"' )
          value.remove(0,1);
         
        if ( value[value.length()-1] == '"' )
          value.truncate(value.length()-1);        
        
        // Normalize path
        if ( value[value.length()-1] != '/' )
             value += '/';
             
        bool b = true;             
        sharedPaths.insert(value,&b);
        kdDebug(7000) << "KSambaShare: Found path: " << value << endl;
      }
    }
  }

  f.close();

  return true;  

}

KSambaShare::KSambaShare() {
  d = new KSambaSharePrivate();
  if (QFile::exists(d->smbConf)) {
    KDirWatch::self()->addFile(d->smbConf);
    KDirWatch::self()->addFile(FILESHARECONF);
    connect(KDirWatch::self(), SIGNAL(dirty (const QString&)),this,
   	        SLOT(slotFileChange(const QString&)));
  } 
}

KSambaShare::~KSambaShare() {
  if (QFile::exists(d->smbConf)) {
        KDirWatch::self()->removeFile(d->smbConf);
        KDirWatch::self()->removeFile(FILESHARECONF);
  }
  delete d;
}

QString KSambaShare::smbConfPath() const {
  return d->smbConf;
}

bool KSambaShare::isDirectoryShared( const QString & path ) const {
  QString fixedPath = path;
  if ( path[path.length()-1] != '/' )
       fixedPath += '/';
  
  return d->sharedPaths.find(fixedPath) > 0;
}

QStringList KSambaShare::sharedDirectories() const {
  QStringList result;
  QHash<QString, bool>::const_iterator i = d->sharedPaths.constBegin();
  while (i != d->sharedPaths.constEnd()) 
		result << i.key();
      
  return result;       
}

void KSambaShare::slotFileChange( const QString & path ) {
  if (path == d->smbConf)
     d->readSmbConf();
  else
  if (path == FILESHARECONF)
     d->load();
              
  emit changed();     
}

KSambaShare* KSambaShare::_instance = 0L; 
static KStaticDeleter<KSambaShare> ksdSambaShare;

KSambaShare* KSambaShare::instance() {
  if (! _instance ) 
      _instance = ksdSambaShare.setObject(_instance, new KSambaShare());
      
  return _instance;      
}

#include "ksambashare.moc"

