/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   $Id$

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
#include <stdio.h>

#include "kio/global.h"
#include "kio/job.h"

#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>

#ifdef HAVE_VOLMGT
#include <volmgt.h>
#endif

QString KIO::convertSize( KIO::filesize_t size )
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

QString KIO::convertSizeFromKB( KIO::filesize_t kbSize )
{
    return convertSize(kbSize * 1024);
}

QString KIO::number( KIO::filesize_t size )
{
    char charbuf[256];
    sprintf(charbuf, "%lld", size);
    return QString::fromLatin1(charbuf);
}

QTime KIO::calculateRemaining( KIO::filesize_t totalSize, KIO::filesize_t processedSize, KIO::filesize_t speed )
{
  QTime remainingTime;

  if ( speed != 0 ) {
    KIO::filesize_t secs;
    if ( totalSize == 0 ) {
      secs = 0;
    } else {
      secs = ( totalSize - processedSize ) / speed;
    }
    if (secs >= (24*60*60)) // Limit to 23:59:59
       secs = (24*60*60)-1;
    int hr = secs / ( 60 * 60 );
    int mn = ( secs - hr * 60 * 60 ) / 60;
    int sc = ( secs - hr * 60 * 60 - mn * 60 );

    remainingTime.setHMS( hr, mn, sc );
  }

  return remainingTime;
}

QString KIO::itemsSummaryString(uint items, uint files, uint dirs, KIO::filesize_t size, bool showSize)
{
    QString text = i18n( "One Item", "%n Items", items );
    text += " - ";
    text += i18n( "One File", "%n Files", files );
    if ( showSize && files > 0 )
    {
        text += " ";
        text += i18n("(%1 Total)").arg(KIO::convertSize( size ) );
    }
    text += " - ";
    text += i18n("One Directory", "%n Directories", dirs);
    return text;
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

QString KIO::Job::errorString() const
{
  return KIO::buildErrorString(m_error, m_errorText);
}

QString KIO::buildErrorString(int errorCode, const QString &errorText)
{
  QString result;

  switch( errorCode )
    {
    case  KIO::ERR_CANNOT_OPEN_FOR_READING:
      result = i18n( "Could not read\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_OPEN_FOR_WRITING:
      result = i18n( "Could not write to\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_LAUNCH_PROCESS:
      result = i18n( "Could not start process\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_INTERNAL:
      result = i18n( "Internal Error\nPlease send a full bug report at http://bugs.kde.org\n\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_MALFORMED_URL:
      result = i18n( "Malformed URL\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_UNSUPPORTED_PROTOCOL:
      result = i18n( "The protocol %1\n is not supported." ).arg( errorText );
      break;
    case  KIO::ERR_NO_SOURCE_PROTOCOL:
      result = i18n( "The protocol %1\nis only a filter protocol.\n").arg( errorText );
      break;
    case  KIO::ERR_UNSUPPORTED_ACTION:
      result = errorText;
//       result = i18n( "Unsupported action\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_IS_DIRECTORY:
      result = i18n( "%1\n is a directory,\nbut a file was expected." ).arg( errorText );
      break;
    case  KIO::ERR_IS_FILE:
      result = i18n( "%1\n is a file,\nbut a directory was expected." ).arg( errorText );
      break;
    case  KIO::ERR_DOES_NOT_EXIST:
      result = i18n( "The file or directory\n%1\ndoes not exist." ).arg( errorText );
      break;
    case  KIO::ERR_FILE_ALREADY_EXIST:
      result = i18n( "A file named\n%1\nalready exists." ).arg( errorText );
      break;
    case  KIO::ERR_DIR_ALREADY_EXIST:
      result = i18n( "A directory named\n%1\nalready exists." ).arg( errorText );
      break;
    case  KIO::ERR_UNKNOWN_HOST:
      result = i18n( "Unknown host\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_ACCESS_DENIED:
      result = i18n( "Access denied to\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_WRITE_ACCESS_DENIED:
      result = i18n( "Access denied\nCould not write to\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_ENTER_DIRECTORY:
      result = i18n( "Could not enter directory\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_PROTOCOL_IS_NOT_A_FILESYSTEM:
      result = i18n( "The protocol %1\ndoes not implement a directory service." ).arg( errorText );
      break;
    case  KIO::ERR_CYCLIC_LINK:
      result = i18n( "Found a cyclic link in\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_USER_CANCELED:
      // Do nothing in this case. The user doesn't need to be told what he just did.
      break;
    case  KIO::ERR_CYCLIC_COPY:
      result = i18n( "Found a cyclic link while copying\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_CREATE_SOCKET:
      result = i18n( "Could not create socket for accessing\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_CONNECT:
      result = i18n( "Could not connect to host\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_CONNECTION_BROKEN:
      result = i18n( "Connection to host\n%1\nis broken" ).arg( errorText );
      break;
    case  KIO::ERR_NOT_FILTER_PROTOCOL:
      result = i18n( "The protocol %1\nis not a filter protocol" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_MOUNT:
      result = i18n( "Could not mount device.\nThe reported error was:\n\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_UNMOUNT:
      result = i18n( "Could not unmount device.\nThe reported error was:\n\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_READ:
      result = i18n( "Could not read file\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_WRITE:
      result = i18n( "Could not write to file\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_BIND:
      result = i18n( "Could not bind\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_LISTEN:
      result = i18n( "Could not listen\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_ACCEPT:
      result = i18n( "Could not accept\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_LOGIN:
      result = errorText;
      break;
    case  KIO::ERR_COULD_NOT_STAT:
      result = i18n( "Could not access\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_CLOSEDIR:
      result = i18n( "Could not terminate listing\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_MKDIR:
      result = i18n( "Could not make directory\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_RMDIR:
      result = i18n( "Could not remove directory\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_RESUME:
      result = i18n( "Could not resume file %1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_RENAME:
      result = i18n( "Could not rename file %1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_CHMOD:
      result = i18n( "Could not change permissions for\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_DELETE:
      result = i18n( "Could not delete file %1" ).arg( errorText );
      break;
    case  KIO::ERR_SLAVE_DIED:
      result = i18n( "The process for the\n%1 protocol\ndied unexpectedly." ).arg( errorText );
      break;
    case  KIO::ERR_OUT_OF_MEMORY:
      result = i18n( "Error. Out of Memory.\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_UNKNOWN_PROXY_HOST:
      result = i18n( "Unknown proxy host\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_AUTHENTICATE:
      result = i18n( "Authorization failed, %1 authentication not supported" ).arg( errorText );
      break;
    case  KIO::ERR_ABORTED:
      result = i18n( "User canceled action\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_INTERNAL_SERVER:
      result = i18n( "Internal error in server\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_SERVER_TIMEOUT:
      result = i18n( "Timeout on server\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_UNKNOWN:
      result = i18n( "Unknown error\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_UNKNOWN_INTERRUPT:
      result = i18n( "Unknown interrupt\n%1" ).arg( errorText );
      break;
/*
    case  KIO::ERR_CHECKSUM_MISMATCH:
      if (errorText)
        result = i18n( "Warning: MD5 Checksum for %1 does not match checksum returned from server" ).arg(errorText);
      else
        result = i18n( "Warning: MD5 Checksum for %1 does not match checksum returned from server" ).arg("document");
      break;
*/
    case KIO::ERR_CANNOT_DELETE_ORIGINAL:
      result = i18n( "Could not delete original file %1.\nPlease check permissions." ).arg( errorText );
      break;
    case KIO::ERR_CANNOT_DELETE_PARTIAL:
      result = i18n( "Could not delete partial file %1.\nPlease check permissions." ).arg( errorText );
      break;
    case KIO::ERR_CANNOT_RENAME_ORIGINAL:
      result = i18n( "Could not rename original file %1.\nPlease check permissions." ).arg( errorText );
      break;
    case KIO::ERR_CANNOT_RENAME_PARTIAL:
      result = i18n( "Could not rename partial file %1.\nPlease check permissions." ).arg( errorText );
      break;
    case KIO::ERR_CANNOT_SYMLINK:
      result = i18n( "Could not create symlink %1.\nPlease check permissions." ).arg( errorText );
      break;
    case KIO::ERR_NO_CONTENT:
      result = errorText;
      break;
    case KIO::ERR_DISK_FULL:
      result = i18n( "Could not write file %1.\nDisk full." ).arg( errorText );
      break;
    case KIO::ERR_IDENTICAL_FILES:
      result = i18n( "The source and destination are the same file.\n%1" ).arg( errorText );
      break;
    default:
      result = i18n( "Unknown error code %1\n%2\n\nPlease send a full bug report at http://bugs.kde.org." ).arg( errorCode ).arg( errorText );
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
#ifdef HAVE_SYS_UCRED_H
#include <sys/ucred.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_FSTAB_H
#include <fstab.h>
#endif
#if defined(_AIX)
#include <sys/mntctl.h>
#include <sys/vmount.h>
#include <sys/vfs.h>
/* AIX does not prototype mntctl anywhere that I can find */
#ifndef mntctl
extern "C" {
int mntctl(int command, int size, void* buffer);
}
#endif
extern "C" struct vfs_ent *getvfsbytype(int vfsType);
extern "C" void endvfsent( );
#endif

/***************************************************************
 *
 * Utility functions
 *
 ***************************************************************/

#ifndef HAVE_GETMNTINFO

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

// There are (at least) four kind of APIs:
// setmntent + getmntent + struct mntent (linux...)
//             getmntent + struct mnttab
// mntctl                + struct vmount (AIX)
// getmntinfo + struct statfs&flags (BSD 4.4 and friends)
// getfsent + char* (BSD 4.3 and friends)

#ifdef HAVE_SETMNTENT
#define SETMNTENT setmntent
#define ENDMNTENT endmntent
#define STRUCT_MNTENT struct mntent *
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) ((var = getmntent(file)) != 0)
#define MOUNTPOINT(var) var->mnt_dir
#define MOUNTTYPE(var) var->mnt_type
#define FSNAME(var) var->mnt_fsname
#elif defined(_AIX)
/* we don't need this stuff */
#else
#define SETMNTENT fopen
#define ENDMNTENT fclose
#define STRUCT_MNTENT struct mnttab
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) (getmntent(file, &var) == 0)
#define MOUNTPOINT(var) var.mnt_mountp
#define MOUNTTYPE(var) var.mnt_fstype
#define FSNAME(var) var.mnt_special
#endif

#endif /* HAVE_GETMNTINFO */

QString KIO::findDeviceMountPoint( const QString& filename )
{
    //kdDebug( 7007 ) << "findDeviceMountPoint " << filename << endl;
    QString result;

#ifdef HAVE_VOLMGT
	/*
	 *  support for Solaris volume management
	 */
	const char *volpath;
	FILE *mnttab;
	struct mnttab mnt;
	int len;
	QCString devname;

	if( (volpath = volmgt_root()) == NULL ) {
		kdDebug( 7007 ) << "findDeviceMountPoint: "
			<< "VOLMGT: can't find volmgt root dir" << endl;
		return QString::null;
	}

	if( (mnttab = fopen( MNTTAB, "r" )) == NULL ) {
		kdDebug( 7007 ) << "findDeviceMountPoint: "
			<< "VOLMGT: can't open mnttab" << endl;
		return QString::null;
	}

	devname = volpath;
	devname += QFile::encodeName( filename.local8Bit() );
	devname += '/';
	len = devname.length();
//	kdDebug( 7007 ) << "findDeviceMountPoint: "
//		<< "VOLMGT: searching mountpoint for \"" << devname << "\""
//		<< endl;

	/*
	 *  find the mountpoint
	 *  floppies:
	 *	/dev/disketteN    => <volpath>/dev/disketteN
	 *  CDROM, ZIP, and other media:
	 *	/dev/dsk/cXtYdZs2 => <volpath>/dev/dsk/cXtYdZ  (without slice#)
	 */
	rewind( mnttab );
	result = QString::null;
	while( getmntent( mnttab, &mnt ) == 0 ) {
		/*
		 *  either match the exact device name (floppies),
		 *  or the device name without the slice#
		 */
		if( strncmp( devname.data(), mnt.mnt_special, len ) == 0
			|| (strncmp( devname.data(),
				mnt.mnt_special, len - 3 ) == 0
				&& mnt.mnt_special[len - 3] == '/' )) {
			result = mnt.mnt_mountp;
			break;
		}
	}
	fclose( mnttab );
	devname.~QCString();
#else

    char    realpath_buffer[MAXPATHLEN];
    QCString realname;

    realname = QFile::encodeName(filename);
    /* If the path contains symlinks, get the real name */
    if (realpath(realname, realpath_buffer) != 0)
      // succes, use result from realpath
      realname = realpath_buffer;

#ifdef HAVE_GETMNTINFO

    struct statfs *mounted;

    int num_fs = getmntinfo(&mounted, MNT_NOWAIT);

    for (int i=0;i<num_fs;i++) {

        QCString device_name = mounted[i].f_mntfromname;

        // If the path contains symlinks, get
        // the real name
        if (realpath(device_name, realpath_buffer) != 0)
            // succes, use result from realpath
            device_name = realpath_buffer;

        if (realname == device_name) {
            result = mounted[i].f_mntonname;
            break;
        }
    }

#elif defined(_AIX)

    struct vmount *mntctl_buffer;
    struct vmount *vm;
    char *mountedfrom;
    char *mountedto;
    int fsname_len, num;
    int buf_sz = 4096;

    /* mntctl can be used to query mounted file systems.
     * mntctl takes only the command MCTL_QUERY so far.
     * The buffer is filled with an array of vmount structures, but these
     * vmount structures have variable size.
     * mntctl return values:
     * -1 error
     *  0 look in first word of buffer for required bytes, 4096 may be
     *    a good starting size, but if tables grow too large, look here.
     * >0 number of vmount structures
     */
    mntctl_buffer = (struct vmount*)malloc(buf_sz);
    num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    if (num == 0)
    {
	buf_sz = *(int*)mntctl_buffer;
	free(mntctl_buffer);
	mntctl_buffer = (struct vmount*)malloc(buf_sz);
	num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    }

    if (num > 0)
    {
        /* iterate through items in the vmount structure: */
        vm = mntctl_buffer;
        for ( ; num > 0; num-- )
        {
            /* get the name of the mounted file systems: */
            fsname_len = vmt2datasize(vm, VMT_STUB);
            mountedto     = (char*)malloc(fsname_len + 1);
	    mountedto[fsname_len] = '\0';
            strncpy(mountedto, (char *)vmt2dataptr(vm, VMT_STUB), fsname_len);

            /* get the mount-from information: */
            fsname_len = vmt2datasize(vm, VMT_OBJECT);
            mountedfrom     = (char*)malloc(fsname_len + 1);
	    mountedfrom[fsname_len] = '\0';
            strncpy(mountedfrom, (char *)vmt2dataptr(vm, VMT_OBJECT), fsname_len);

            QCString device_name = mountedfrom;

            if (realpath(device_name, realpath_buffer) != 0)
                // success, use result from realpath
                device_name = realpath_buffer;

            free(mountedfrom);

            if (realname == device_name) {
                result = mountedto;
                free(mountedto);
                break;
            }

            free(mountedto);

            /* goto the next vmount structure: */
            vm = (struct vmount *)((char *)vm + vm->vmt_length);
        }
    }

    free( mntctl_buffer );

#else

    STRUCT_SETMNTENT mtab;

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

#endif /* GET_MNTINFO */
#endif /* HAVE_VOLMGT */

    //kdDebug( 7007 ) << "Returning result " << result << endl;
    return result;
}

// Don't just trust the return value, keep iterating to check for a better match (bigger max)
static bool is_my_mountpoint( const char *mountpoint, const char *realname, int &max )
{
    int length = strlen(mountpoint);

    if (!strncmp(mountpoint, realname, length)
        && length > max) {
        max = length;
        if (length == 1 || realname[length] == '/' || realname[length] == '\0')
            return true;
    }
    return false;
}

QString KIO::findPathMountPoint( const QString & filename )
{
    char realname[MAXPATHLEN];

    memset(realname, 0, MAXPATHLEN);

    /* If the path contains symlinks, get the real name */
    if (realpath(QFile::encodeName(filename), realname) == 0) {
        if (filename.length() >= sizeof(realname))
            return QString::null;
        strcpy(realname, QFile::encodeName(filename));
    }

    int max = 0;
    QString mountPoint;

#ifdef HAVE_GETMNTINFO

    struct statfs *mounted;
    char    realpath_buffer[MAXPATHLEN];

    int num_fs = getmntinfo(&mounted, MNT_NOWAIT);

    for (int i=0;i<num_fs;i++) {

        QCString device_name = mounted[i].f_mntfromname;

        // If the path contains symlinks, get
        // the real name
        if (realpath(device_name, realpath_buffer) != 0)
            // succes, use result from realpath
            device_name = realpath_buffer;
        if ( is_my_mountpoint( mounted[i].f_mntonname, realname, max ) )
            mountPoint = QFile::decodeName(mounted[i].f_mntonname);
    }

#elif defined(_AIX)

    struct vmount *mntctl_buffer;
    struct vmount *vm;
    char *mountedfrom;
    char *mountedto;
    int fsname_len, num;
    char realpath_buffer[MAXPATHLEN];
    int buf_sz = 4096;

    mntctl_buffer = (struct vmount*)malloc(buf_sz);
    num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    if (num == 0)
    {
	buf_sz = *(int*)mntctl_buffer;
	free(mntctl_buffer);
	mntctl_buffer = (struct vmount*)malloc(buf_sz);
	num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    }

    if (num > 0)
    {
        /* iterate through items in the vmount structure: */
        vm = (struct vmount *)mntctl_buffer;
        for ( ; num > 0; num-- )
        {
            /* get the name of the mounted file systems: */
            fsname_len = vmt2datasize(vm, VMT_STUB);
            mountedto     = (char*)malloc(fsname_len + 1);
	    mountedto[fsname_len] = '\0';
            strncpy(mountedto, (char *)vmt2dataptr(vm, VMT_STUB), fsname_len);

            fsname_len = vmt2datasize(vm, VMT_OBJECT);
            mountedfrom     = (char*)malloc(fsname_len + 1);
	    mountedfrom[fsname_len] = '\0';
            strncpy(mountedfrom, (char *)vmt2dataptr(vm, VMT_OBJECT), fsname_len);

            /* get the mount-from information: */
            QCString device_name = mountedfrom;

            if (realpath(device_name, realpath_buffer) != 0)
                // success, use result from realpath
                device_name = realpath_buffer;

            if ( is_my_mountpoint( mountedto, realname, max ) )
                mountPoint = QFile::decodeName(mountedto);

            free(mountedfrom);
            free(mountedto);

            /* goto the next vmount structure: */
            vm = (struct vmount *)((char *)vm + vm->vmt_length);
        }

	endvfsent( );
    }

    free( mntctl_buffer );

#else

    STRUCT_SETMNTENT mtab;
    /* Get the list of mounted file systems */

    if ((mtab = SETMNTENT(MNTTAB, "r")) == 0) {
        perror("setmntent");
        return QString::null;
    }

    STRUCT_MNTENT me;

    while (true) {
        if (!GETMNTENT(mtab, me))
            break;

        if ( is_my_mountpoint( MOUNTPOINT(me), realname, max ) )
            mountPoint = QFile::decodeName( MOUNTPOINT(me) );
    }

    ENDMNTENT(mtab);

#endif

    return mountPoint;
}

typedef enum { Unseen, Right, Wrong } MountState;

/**
 * Idea and code base by Olaf Kirch <okir@caldera.de>
 **/
static void check_mount_point(const char *mounttype,
                              const char *fsname,
                              MountState &isslow, MountState &isauto)
{
    bool nfs = !strcmp(mounttype, "nfs");
    bool autofs = !strcmp(mounttype, "autofs");
    bool pid = (strstr(fsname, ":(pid") != 0);

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

bool KIO::probably_slow_mounted(const QString& filename)
{
    char realname[MAXPATHLEN];

    memset(realname, 0, MAXPATHLEN);

    /* If the path contains symlinks, get the real name */
    if (realpath(QFile::encodeName(filename), realname) == 0) {
        if (filename.length() >= sizeof(realname))
            return false;
        strcpy(realname, QFile::encodeName(filename));
    }

    MountState isauto = Unseen, isslow = Unseen;
    int max = 0;

    /* Loop over all file systems and see if we can find our
     * mount point.
     * Note that this is the mount point with the longest match.
     * XXX: Fails if me->mnt_dir is not a realpath but goes
     * through a symlink, e.g. /foo/bar where /foo is a symlink
     * pointing to /local/foo.
     *
     * How kinky can you get with a filesystem?
     */

#ifdef HAVE_GETMNTINFO

    struct statfs *mounted;
    char    realpath_buffer[MAXPATHLEN];

    int num_fs = getmntinfo(&mounted, MNT_NOWAIT);

    for (int i=0;i<num_fs;i++) {

        QCString device_name = mounted[i].f_mntfromname;

        // If the path contains symlinks, get
        // the real name
        if (realpath(device_name, realpath_buffer) != 0)
            // succes, use result from realpath
            device_name = realpath_buffer;
#ifdef __osf__
        char * mounttype = mnt_names[mounted[i].f_type];
#else
        char * mounttype = mounted[i].f_fstypename;
#endif
        if ( is_my_mountpoint( mounted[i].f_mntonname, realname, max ) )
        {
            check_mount_point( mounttype, mounted[i].f_mntfromname,
                               isauto, isslow );
            // keep going, looking for a potentially better one
        }
    }

#elif defined(_AIX)

    struct vmount *mntctl_buffer;
    struct vmount *vm;
    char *mountedfrom;
    char *mountedto;
    int fsname_len, num;
    char    realpath_buffer[MAXPATHLEN];
    int buf_sz = 4096;

    mntctl_buffer = (struct vmount*)malloc(buf_sz);
    num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    if (num == 0)
    {
	buf_sz = *(int*)mntctl_buffer;
	free(mntctl_buffer);
	mntctl_buffer = (struct vmount*)malloc(buf_sz);
	num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    }

    if (num > 0)
    {
        /* iterate through items in the vmount structure: */
        vm = (struct vmount *)mntctl_buffer;
        for ( ; num > 0; num-- )
        {
            /* get the name of the mounted file systems: */
            fsname_len = vmt2datasize(vm, VMT_STUB);
            mountedto     = (char*)malloc(fsname_len + 1);
	    mountedto[fsname_len] = '\0';
            strncpy(mountedto, (char *)vmt2dataptr(vm, VMT_STUB), fsname_len);

            fsname_len = vmt2datasize(vm, VMT_OBJECT);
            mountedfrom     = (char*)malloc(fsname_len + 1);
	    mountedfrom[fsname_len] = '\0';
            strncpy(mountedfrom, (char *)vmt2dataptr(vm, VMT_OBJECT), fsname_len);

            /* get the mount-from information: */
            QCString device_name = mountedfrom;

            if (realpath(device_name, realpath_buffer) != 0)
                // success, use result from realpath
                device_name = realpath_buffer;

	    /* Look up the string for the file system type,
             * as listed in /etc/vfs.
             * ex.: nfs,jfs,afs,cdrfs,sfs,cachefs,nfs3,autofs
             */
            struct vfs_ent* ent = getvfsbytype(vm->vmt_gfstype);

            if ( is_my_mountpoint( mountedto, realname, max ) )
                check_mount_point(ent->vfsent_name, device_name, isauto, isslow);

            free(mountedfrom);
            free(mountedto);

            /* goto the next vmount structure: */
            vm = (struct vmount *)((char *)vm + vm->vmt_length);
        }

	endvfsent( );
    }

    free( mntctl_buffer );

#else

    STRUCT_SETMNTENT mtab;
    /* Get the list of mounted file systems */

    if ((mtab = SETMNTENT(MNTTAB, "r")) == 0) {
        perror("setmntent");
        return false;
    }

    STRUCT_MNTENT me;

    while (true) {
        if (!GETMNTENT(mtab, me))
            break;

        if ( is_my_mountpoint( MOUNTPOINT(me), realname, max ) )
            check_mount_point(MOUNTTYPE(me), FSNAME(me), isauto, isslow);
    }

    ENDMNTENT(mtab);

#endif

    if (isauto == Right && isslow == Unseen)
        isslow = Right;

    return (isslow == Right);
}

KIO::CacheControl KIO::parseCacheControl(const QString &cacheControl)
{
  QString tmp = cacheControl.lower();

  if (tmp == "cacheonly")
     return KIO::CC_CacheOnly;
  if (tmp == "cache")
     return KIO::CC_Cache;
  if (tmp == "verify")
     return KIO::CC_Verify;
  if (tmp == "reload")
     return KIO::CC_Reload;

  kdWarning() << "unrecognized Cache control option:"<<cacheControl<<endl;
  return KIO::CC_Verify;
}

QString KIO::getCacheControlString(KIO::CacheControl cacheControl)
{
    if (cacheControl == KIO::CC_CacheOnly)
	return "CacheOnly";
    if (cacheControl == KIO::CC_Cache)
	return "Cache";
    if (cacheControl == KIO::CC_Verify)
	return "Verify";
    if (cacheControl == KIO::CC_Reload)
	return "Reload";
    kdFatal() << "unrecognized Cache control enum value:"<<cacheControl<<endl;
    return QString::null;
}
