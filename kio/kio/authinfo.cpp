/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000-2001 Dawit Alemayehu <adawit@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <config.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <qdir.h>
#include <qfile.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <ksavefile.h>
#include <kstaticdeleter.h>
#include <kde_file.h>

#include "kio/authinfo.h"

#define NETRC_READ_BUF_SIZE 4096

using namespace KIO;

AuthInfo::AuthInfo()
{
    modified = false;
    readOnly = false;
    verifyPath = false;
    keepPassword = false;
}

AuthInfo::AuthInfo( const AuthInfo& info )
{
    (*this) = info;
}

AuthInfo& AuthInfo::operator= ( const AuthInfo& info )
{
    url = info.url;
    username = info.username;
    password = info.password;
    prompt = info.prompt;
    caption = info.caption;
    comment = info.comment;
    commentLabel = info.commentLabel;
    realmValue = info.realmValue;
    digestInfo = info.digestInfo;
    verifyPath = info.verifyPath;
    readOnly = info.readOnly;
    keepPassword = info.keepPassword;
    modified = info.modified;
    return *this;
}

QDataStream& KIO::operator<< (QDataStream& s, const AuthInfo& a)
{
    s << a.url << a.username << a.password << a.prompt << a.caption
      << a.comment << a.commentLabel << a.realmValue << a.digestInfo
      << Q_UINT8(a.verifyPath ? 1:0) << Q_UINT8(a.readOnly ? 1:0)
      << Q_UINT8(a.keepPassword ? 1:0) << Q_UINT8(a.modified ? 1:0);
    return s;
}

QDataStream& KIO::operator>> (QDataStream& s, AuthInfo& a)
{
    Q_UINT8 verify = 0;
    Q_UINT8 ro = 0;
    Q_UINT8 keep = 0;
    Q_UINT8 mod  = 0;

    s >> a.url >> a.username >> a.password >> a.prompt >> a.caption
      >> a.comment >> a.commentLabel >> a.realmValue >> a.digestInfo
      >> verify >> ro >> keep >> mod;
    a.verifyPath = (verify != 0);
    a.readOnly = (ro != 0);
    a.keepPassword = (keep != 0);
    a.modified = (mod != 0);
    return s;
}


NetRC* NetRC::instance = 0L;

NetRC::NetRC()
{
    isDirty = false;
}

NetRC::~NetRC()
{
    delete instance;
    instance = 0L;
}

NetRC* NetRC::self()
{
    if ( !instance )
        instance = new NetRC();
    return instance;
}

bool NetRC::lookup( const KURL& url, AutoLogin& login, bool userealnetrc,
                    QString type, int mode )
{
  // kdDebug() << "AutoLogin lookup for: " << url.host() << endl;
  if ( !url.isValid() )
    return false;

  if ( type.isEmpty() )
    type = url.protocol();

  if ( loginMap.isEmpty() || isDirty )
  {
    loginMap.clear();

    QString filename = locateLocal("config", "kionetrc");
    bool status = parse (openf (filename));

    if ( userealnetrc )
    {
      filename =  QDir::homeDirPath()+ QDir::separator() + ".netrc";
      status |= parse (openf(filename));
    }

    if ( !status )
      return false;
  }

  if ( !loginMap.contains( type ) )
    return false;

  LoginList l = loginMap[type];
  if ( l.isEmpty() )
    return false;

  for (LoginList::Iterator it = l.begin(); it != l.end(); ++it)
  {
    AutoLogin &log = *it;

    if ( (mode & defaultOnly) == defaultOnly &&
          log.machine == QString::fromLatin1("default") &&
          (login.login.isEmpty() || login.login == log.login) )
    {
      login.type = log.type;
      login.machine = log.machine;
      login.login = log.login;
      login.password = log.password;
      login.macdef = log.macdef;
    }

    if ( (mode & presetOnly) == presetOnly &&
          log.machine == QString::fromLatin1("preset") &&
          (login.login.isEmpty() || login.login == log.login) )
    {
      login.type = log.type;
      login.machine = log.machine;
      login.login = log.login;
      login.password = log.password;
      login.macdef = log.macdef;
    }

    if ( (mode & exactOnly) == exactOnly &&
          log.machine == url.host() &&
          (login.login.isEmpty() || login.login == log.login) )
    {
      login.type = log.type;
      login.machine = log.machine;
      login.login = log.login;
      login.password = log.password;
      login.macdef = log.macdef;
      break;
    }
  }

  return true;
}

int NetRC::openf( const QString& f )
{
  KDE_struct_stat sbuff;
  QCString ef = QFile::encodeName(f);
  if ( KDE_stat(ef, &sbuff) != 0 )
    return -1;

  // Security check!!
  if ( sbuff.st_mode != (S_IFREG|S_IRUSR|S_IWUSR) ||
       sbuff.st_uid != geteuid() )
    return -1;

  return KDE_open( ef, O_RDONLY );
}

QString NetRC::extract( const char* buf, const char* key, int& pos )
{
  int idx = pos;
  int m_len = strlen(key);
  int b_len = strlen(buf);

  while( idx < b_len )
  {
    while( buf[idx] == ' ' || buf[idx] == '\t' )
      idx++;

    if ( strncasecmp( buf+idx, key, m_len ) != 0 )
      idx++;
    else
    {
      idx += m_len;
      while( buf[idx] == ' ' || buf[idx] == '\t' )
        idx++;

      int start = idx;
      while( buf[idx] != ' ' && buf[idx] != '\t' &&
             buf[idx] != '\n' && buf[idx] != '\r' )
        idx++;

      if ( idx > start )
      {
        pos = idx;
        return QString::fromLatin1( buf+start, idx-start);
      }
    }
  }

  return QString::null;
}

bool NetRC::parse( int fd )
{
  if (fd == -1)
    return false;

  QString type;
  QString macro;

  uint index = 0;
  bool isMacro = false;
  char* buf = new char[NETRC_READ_BUF_SIZE];
  FILE* fstream = KDE_fdopen( fd,"rb" );

  while ( fgets (buf, NETRC_READ_BUF_SIZE, fstream) != 0L )
  {
    int pos = 0;

    while ( buf[pos] == ' ' || buf[pos] == '\t' )
      pos++;

    if ( buf[pos] == '#' || buf[pos] == '\n' ||
         buf[pos] == '\r' || buf[pos] == '\0' )
    {
      if ( buf[pos] != '#' && isMacro )
        isMacro = false;

      continue;
    }

    if ( isMacro )
    {
      int tail = strlen(buf);
      while( buf[tail-1] == '\n' || buf[tail-1] =='\r' )
        tail--;

      QString mac = QString::fromLatin1(buf, tail).stripWhiteSpace();
      if ( !mac.isEmpty() )
        loginMap[type][index].macdef[macro].append( mac );

      continue;
    }

    AutoLogin l;
    l.machine = extract( buf, "machine", pos );
    if ( l.machine.isEmpty() )
    {
      if (strncasecmp(buf+pos, "default", 7) == 0 )
      {
        pos += 7;
        l.machine = QString::fromLatin1("default");
      }
      else if (strncasecmp(buf+pos, "preset", 6) == 0 )
      {
        pos += 6;
        l.machine = QString::fromLatin1("preset");
      }
    }
    // kdDebug() << "Machine: " << l.machine << endl;

    l.login = extract( buf, "login", pos );
    // kdDebug() << "Login: " << l.login << endl;

    l.password = extract( buf, "password", pos );
    if ( l.password.isEmpty() )
      l.password = extract( buf, "account", pos );
    // kdDebug() << "Password: " << l.password << endl;

    type = l.type = extract( buf, "type", pos );
    if ( l.type.isEmpty() && !l.machine.isEmpty() )
      type = l.type = QString::fromLatin1("ftp");
    // kdDebug() << "Type: " << l.type << endl;

    macro = extract( buf, "macdef", pos );
    isMacro = !macro.isEmpty();
    // kdDebug() << "Macro: " << macro << endl;

    loginMap[l.type].append(l);
    index = loginMap[l.type].count()-1;
  }

  delete [] buf;
  close (fd);
  return true;
}
