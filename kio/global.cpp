/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include <config.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_FSTAB_H
#include <fstab.h>
#endif

#include "kio/global.h"
#include "kio/job.h"

#include <kdebug.h>
#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>

using namespace KIO;

QString KIO::convertSize( unsigned long size )
{
    float fsize;
    QString s;
    // Giga-byte
    if ( size >= 1073741824 )
    {
        fsize = (float) size / (float) 1073741824;
        if ( fsize > 1024 ) // Tera-byte
            s = i18n( "%1 TB" ).arg( KGlobal::locale()->formatNumber(fsize / (float)1024, 1));
        else
            s = i18n( "%1 GB" ).arg( KGlobal::locale()->formatNumber(fsize, 1));
    }
    // Mega-byte
    else if ( size >= 1048576 )
    {
        fsize = (float) size / (float) 1048576;
        s = i18n( "%1 MB" ).arg( KGlobal::locale()->formatNumber(fsize, 1));
    }
    // Kilo-byte
    else if ( size > 1024 )
    {
        fsize = (float) size / (float) 1024;
        s = i18n( "%1 KB" ).arg( KGlobal::locale()->formatNumber(fsize, 1));
    }
    // Just byte
    else
    {
        fsize = (float) size;
        s = i18n( "%1 B" ).arg( KGlobal::locale()->formatNumber(fsize, 0));
    }
    return s;
}

QTime KIO::calculateRemaining( unsigned long totalSize, unsigned long processedSize, unsigned long speed )
{
  QTime remainingTime;

  if ( speed != 0 ) {
    unsigned long secs;
    if ( totalSize == 0 ) {
      secs = 0;
    } else {
      secs = ( totalSize - processedSize ) / speed;
    }
    int hr = secs / ( 60 * 60 );
    int mn = ( secs - hr * 60 * 60 ) / 60;
    int sc = ( secs - hr * 60 * 60 - mn * 60 );

    remainingTime.setHMS( hr, mn, sc );
  }

  return remainingTime;
}

QString KIO::encodeFileName( const QString & _str )
{
  QString str( _str );

  int i = 0;
  while ( ( i = str.find( "%", i ) ) != -1 )
  {
    str.replace( i, 1, "%%");
    i += 2;
  }
  while ( ( i = str.find( "/" ) ) != -1 )
      str.replace( i, 1, "%2f");
  return str;
}

QString KIO::decodeFileName( const QString & _str )
{
  QString str;

  unsigned int i = 0;
  for ( ; i < _str.length() ; ++i )
  {
    if ( _str[i]=='%' )
    {
      if ( _str[i+1]=='%' ) // %% -> %
      {
        str.append('%');
        ++i;
      }
      else if ( _str[i+1]=='2' && (i+2<_str.length()) && _str[i+2].lower()=='f' ) // %2f -> /
      {
        str.append('/');
        i += 2;
      }
      else
        str.append('%');
    } else
      str.append(_str[i]);
  }

  return str;
}

QString Job::errorString()
{
  QString result;

  switch( m_error )
    {
    case  KIO::ERR_CANNOT_OPEN_FOR_READING:
      result = i18n( "Could not read\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_CANNOT_OPEN_FOR_WRITING:
      result = i18n( "Could not write to\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_CANNOT_LAUNCH_PROCESS:
      result = i18n( "Could not start process\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_INTERNAL:
      result = i18n( "Internal Error\nPlease send a full bugreport at http://bugs.kde.org\n\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_MALFORMED_URL:
      result = i18n( "Malformed URL\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_UNSUPPORTED_PROTOCOL:
      result = i18n( "The protocol %1\n is not supported" ).arg( m_errorText );
      break;
    case  KIO::ERR_NO_SOURCE_PROTOCOL:
      result = i18n( "The protocol %1\nis only a filter protocol.\n").arg( m_errorText );
      break;
    case  KIO::ERR_UNSUPPORTED_ACTION:
      result = i18n( "Unsupported action\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_IS_DIRECTORY:
      result = i18n( "%1\n is a directory,\nbut a file was expected" ).arg( m_errorText );
      break;
    case  KIO::ERR_IS_FILE:
      result = i18n( "%1\n is a file,\nbut a directory was expected" ).arg( m_errorText );
      break;
    case  KIO::ERR_DOES_NOT_EXIST:
      result = i18n( "The file or directory\n%1\ndoes not exist" ).arg( m_errorText );
      break;
    case  KIO::ERR_FILE_ALREADY_EXIST:
      result = i18n( "A file named\n%1\nalready exists" ).arg( m_errorText );
      break;
    case  KIO::ERR_DIR_ALREADY_EXIST:
      result = i18n( "A directory named\n%1\nalready exists" ).arg( m_errorText );
      break;
    case  KIO::ERR_UNKNOWN_HOST:
      result = i18n( "Unknown host\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_ACCESS_DENIED:
      result = i18n( "Access denied to\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_WRITE_ACCESS_DENIED:
      result = i18n( "Access denied\nCould not write to\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_CANNOT_ENTER_DIRECTORY:
      result = i18n( "Could not enter directory\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_PROTOCOL_IS_NOT_A_FILESYSTEM:
      result = i18n( "The protocol %1\ndoes not implement a directory service" ).arg( m_errorText );
      break;
    case  KIO::ERR_CYCLIC_LINK:
      result = i18n( "Found a cyclic link in\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_USER_CANCELED:
      // Do nothing in this case. The user doesn't need to be told what he just did.
      break;
    case  KIO::ERR_CYCLIC_COPY:
      result = i18n( "Found a cyclic link while copying\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_COULD_NOT_CREATE_SOCKET:
      result = i18n( "Could not create socket for accessing\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_COULD_NOT_CONNECT:
      result = i18n( "Could not connect to host\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_CONNECTION_BROKEN:
      result = i18n( "Connection to host\n%1\nis broken" ).arg( m_errorText );
      break;
    case  KIO::ERR_NOT_FILTER_PROTOCOL:
      result = i18n( "The protocol %1\nis not a filter protocol" ).arg( m_errorText );
      break;
    case  KIO::ERR_COULD_NOT_MOUNT:
      result = i18n( "Could not mount device.\nThe reported error was:\n\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_COULD_NOT_UNMOUNT:
      result = i18n( "Could not unmount device.\nThe reported error was:\n\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_COULD_NOT_READ:
      result = i18n( "Could not read file\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_COULD_NOT_WRITE:
      result = i18n( "Could not write to file\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_COULD_NOT_BIND:
      result = i18n( "Could not bind\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_COULD_NOT_LISTEN:
      result = i18n( "Could not listen\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_COULD_NOT_ACCEPT:
      result = i18n( "Could not accept\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_COULD_NOT_LOGIN:
      result = m_errorText;
      break;
    case  KIO::ERR_COULD_NOT_STAT:
      result = i18n( "Could not access\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_COULD_NOT_CLOSEDIR:
      result = i18n( "Could not terminate listing\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_COULD_NOT_MKDIR:
      result = i18n( "Could not make directory\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_COULD_NOT_RMDIR:
      result = i18n( "Could not remove directory\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_CANNOT_RESUME:
      result = i18n( "Could not resume file %1" ).arg( m_errorText );
      break;
    case  KIO::ERR_CANNOT_RENAME:
      result = i18n( "Could not rename file %1" ).arg( m_errorText );
      break;
    case  KIO::ERR_CANNOT_CHMOD:
      result = i18n( "Could not change permissions for\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_CANNOT_DELETE:
      result = i18n( "Could not delete file %1" ).arg( m_errorText );
      break;
    case  KIO::ERR_SLAVE_DIED:
      result = i18n( "The process for the\n%1 protocol\ndied unexpectedly" ).arg( m_errorText );
      break;
    case  KIO::ERR_OUT_OF_MEMORY:
      result = i18n( "Error. Out of Memory.\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_UNKNOWN_PROXY_HOST:
      result = i18n( "Unknown proxy host\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_COULD_NOT_AUTHENTICATE:
      result = i18n( "Authorization failed, %1 authentication not supported" ).arg( m_errorText );
      break;
    case  KIO::ERR_ABORTED:
      result = i18n( "User canceled action\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_INTERNAL_SERVER:
      result = i18n( "Internal error in server\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_SERVER_TIMEOUT:
      result = i18n( "Timeout on server\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_UNKNOWN:
      result = i18n( "Unknown error\n%1" ).arg( m_errorText );
      break;
    case  KIO::ERR_UNKNOWN_INTERRUPT:
      result = i18n( "Unknown interrupt\n%1" ).arg( m_errorText );
      break;
/*
    case  KIO::ERR_CHECKSUM_MISMATCH:
      if (m_errorText)
        result = i18n( "Warning: MD5 Checksum for %1 does not match checksum returned from server" ).arg(m_errorText);
      else
        result = i18n( "Warning: MD5 Checksum for %1 does not match checksum returned from server" ).arg("document");
      break;
*/
    case KIO::ERR_CANNOT_DELETE_ORIGINAL:
      result = i18n( "Could not delete original file %1.\nPlease check permissions." ).arg( m_errorText );
      break;
    case KIO::ERR_CANNOT_DELETE_PARTIAL:
      result = i18n( "Could not delete partial file %1.\nPlease check permissions." ).arg( m_errorText );
      break;
    case KIO::ERR_CANNOT_RENAME_ORIGINAL:
      result = i18n( "Could not rename original file %1.\nPlease check permissions." ).arg( m_errorText );
      break;
    case KIO::ERR_CANNOT_RENAME_PARTIAL:
      result = i18n( "Could not rename partial file %1.\nPlease check permissions." ).arg( m_errorText );
      break;
    case KIO::ERR_CANNOT_SYMLINK:
      result = i18n( "Could not create symlink %1.\nPlease check permissions." ).arg( m_errorText );
      break;
    default:
      result = i18n( "Unknown error code %1\n%2\n\nPlease send a full bugreport at http://bugs.kde.org" ).arg( m_error ).arg( m_errorText );
      break;
    }

  return result;
}

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <qfile.h>

#include <config.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/param.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_SYS_MNTTAB_H
#include <sys/mnttab.h>
#endif
#ifdef HAVE_MNTENT_H
#include <mntent.h>
#endif

/***************************************************************
 *
 * Utility functions
 *
 ***************************************************************/

#ifdef _PATH_MOUNTED
// On some Linux, MNTTAB points to /etc/fstab !
# undef MNTTAB
# define MNTTAB _PATH_MOUNTED
#else
# ifndef MNTTAB
#  ifdef MTAB_FILE
#   define MNTTAB MTAB_FILE
#  else
#   define MNTTAB "/etc/mnttab"
#  endif
# endif
#endif

// hopefully there are only two kind of APIs. If not we need a configure test
#ifdef HAVE_SETMNTENT
#define SETMNTENT setmntent
#define ENDMNTENT endmntent
#define STRUCT_MNTENT struct mntent *
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) ((var = getmntent(file)) != 0)
#define MOUNTPOINT(var) var->mnt_dir
#define MOUNTTYPE(var) var->mnt_type
#define FSNAME(var) var->mnt_fsname
#elif HAVE_SETFSENT
#define SETMNTENT(x, y) setfsent()
#define ENDMNTENT(x) /* nope */
#define STRUCT_MNTENT struct fstab *
#define STRUCT_SETMNTENT int
#define GETMNTENT(file, var) ((var=getfsent()) != 0)
#define MOUNTPOINT(var) var->fs_file
#if defined(_AIX)
#define MOUNTTYPE(var) var->fs_type
#else
#define MOUNTTYPE(var) var->fs_vfstype
#endif
#define FSNAME(var) var->fs_spec
#else /* no setmntent and no setfsent */
#define SETMNTENT fopen
#define ENDMNTENT fclose
#define STRUCT_MNTENT struct mnttab
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) (getmntent(file, &var) == 0)
#define MOUNTPOINT(var) var.mnt_mountp
#define MOUNTTYPE(var) var.mnt_fstype
#define FSNAME(var) var.mnt_special
#endif


QString KIO::findDeviceMountPoint( const QString& filename )
{
    STRUCT_SETMNTENT mtab;
    //kdDebug( 7007 ) << "findDeviceMountPoint " << filename << endl;
    char    realpath_buffer[MAXPATHLEN];
    QCString realname;

    realname = QFile::encodeName(filename);
    /* If the path contains symlinks, get the real name */
    if (realpath(realname, realpath_buffer) != 0)
      // succes, use result from realpath
      realname = realpath_buffer;

    //kdDebug( 7007 ) << "realname " << realname << endl;

    /* Get the list of mounted file systems */

    if ((mtab = SETMNTENT(MNTTAB, "r")) == 0) {
        perror("setmntent");
        return QString::null;
    }

    /* Loop over all file systems and see if we can find our
     * mount point.
     * Note that this is the mount point with the longest match.
     * XXX: Fails if me->mnt_dir is not a realpath but goes
     * through a symlink, e.g. /foo/bar where /foo is a symlink
     * pointing to /local/foo.
     *
     * How kinky can you get with a filesystem?
     */

    STRUCT_MNTENT me;

    QString result;

    while (GETMNTENT(mtab, me))
    {
      // There may be symbolic links into the /etc/mnttab
      // So we have to find the real device name here as well!
      QCString device_name = FSNAME(me);

      //kdDebug( 7007 ) << "device_name=" << device_name << endl;

      // If the path contains symlinks, get
      // the real name
      if (realpath(device_name, realpath_buffer) != 0)
          // succes, use result from realpath
         device_name = realpath_buffer;

      //kdDebug( 7007 ) << "device_name after realpath =" << device_name << endl;

      if (realname == device_name)
      {
          result = MOUNTPOINT(me);
          break;
      }
    }

    ENDMNTENT(mtab);
    //kdDebug( 7007 ) << "Returning result " << result << endl;
    return result;
}

/**
 * Idea and code by Olaf Kirch <okir@caldera.de>
 **/
bool probably_slow_mounted(const QString& filename)
{
    STRUCT_SETMNTENT    mtab;
    char                realname[MAXPATHLEN];
    int                 length, max;

    memset(realname, 0, MAXPATHLEN);

    /* If the path contains symlinks, get the real name */
    if (realpath(QFile::encodeName(filename), realname) == 0) {
        if (filename.length() >= sizeof(realname))
            return false;
        strcpy(realname, QFile::encodeName(filename));
    }

    /* Get the list of mounted file systems */

    if ((mtab = SETMNTENT(MNTTAB, "r")) == 0) {
        perror("setmntent");
        return false;
    }

    /* Loop over all file systems and see if we can find our
     * mount point.
     * Note that this is the mount point with the longest match.
     * XXX: Fails if me->mnt_dir is not a realpath but goes
     * through a symlink, e.g. /foo/bar where /foo is a symlink
     * pointing to /local/foo.
     *
     * How kinky can you get with a filesystem?
     */

    max = 0;
    STRUCT_MNTENT me;

    enum { Unseen, Right, Wrong } isauto = Unseen, isslow = Unseen;

    while (true) {
      if (!GETMNTENT(mtab, me))
        break;

      length = strlen(MOUNTPOINT(me));

      if (!strncmp(MOUNTPOINT(me), realname, length)
          && length > max) {
          max = length;
          if (length == 1 || realname[length] == '/' || realname[length] == '\0') {

              bool nfs = !strcmp(MOUNTTYPE(me), "nfs");
              bool autofs = !strcmp(MOUNTTYPE(me), "autofs");
              bool pid = (strstr(FSNAME(me), ":(pid") != 0);

              if (nfs && !pid)
                  isslow = Right;
              else if (isslow == Right)
                  isslow = Wrong;

              /* Does this look like automounted? */
              if (autofs || (nfs && pid)) {
                  isauto = Right;
                  isslow = Right;
              }
          }
      }
    }

    if (isauto == Right && isslow == Unseen)
        isslow = Right;

    ENDMNTENT(mtab);
    return (isslow == Right);
}


