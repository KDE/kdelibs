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

QString KIO::Job::htmlErrorString() const
{
  return KIO::buildHTMLErrorString(m_error, m_errorText);
}

QString KIO::buildErrorString(int errorCode, const QString &errorText)
{
  QString result;

  switch( errorCode )
    {
    case  KIO::ERR_CANNOT_OPEN_FOR_READING:
      result = i18n( "Could not read %1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_OPEN_FOR_WRITING:
      result = i18n( "Could not write to %1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_LAUNCH_PROCESS:
      result = i18n( "Could not start process %1" ).arg( errorText );
      break;
    case  KIO::ERR_INTERNAL:
      result = i18n( "Internal Error\nPlease send a full bug report at http://bugs.kde.org\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_MALFORMED_URL:
      result = i18n( "Malformed URL %1" ).arg( errorText );
      break;
    case  KIO::ERR_UNSUPPORTED_PROTOCOL:
      result = i18n( "The protocol %1 is not supported." ).arg( errorText );
      break;
    case  KIO::ERR_NO_SOURCE_PROTOCOL:
      result = i18n( "The protocol %1 is only a filter protocol.").arg( errorText );
      break;
    case  KIO::ERR_UNSUPPORTED_ACTION:
      result = errorText;
//       result = i18n( "Unsupported action %1" ).arg( errorText );
      break;
    case  KIO::ERR_IS_DIRECTORY:
      result = i18n( "%1 is a directory, but a file was expected." ).arg( errorText );
      break;
    case  KIO::ERR_IS_FILE:
      result = i18n( "%1 is a file, but a directory was expected." ).arg( errorText );
      break;
    case  KIO::ERR_DOES_NOT_EXIST:
      result = i18n( "The file or directory %1 does not exist." ).arg( errorText );
      break;
    case  KIO::ERR_FILE_ALREADY_EXIST:
      result = i18n( "A file named %1 already exists." ).arg( errorText );
      break;
    case  KIO::ERR_DIR_ALREADY_EXIST:
      result = i18n( "A directory named %1 already exists." ).arg( errorText );
      break;
    case  KIO::ERR_UNKNOWN_HOST:
      result = i18n( "Unknown host %1" ).arg( errorText );
      break;
    case  KIO::ERR_ACCESS_DENIED:
      result = i18n( "Access denied to %1" ).arg( errorText );
      break;
    case  KIO::ERR_WRITE_ACCESS_DENIED:
      result = i18n( "Access denied\nCould not write to %1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_ENTER_DIRECTORY:
      result = i18n( "Could not enter directory %1" ).arg( errorText );
      break;
    case  KIO::ERR_PROTOCOL_IS_NOT_A_FILESYSTEM:
      result = i18n( "The protocol %1 does not implement a directory service." ).arg( errorText );
      break;
    case  KIO::ERR_CYCLIC_LINK:
      result = i18n( "Found a cyclic link in %1" ).arg( errorText );
      break;
    case  KIO::ERR_USER_CANCELED:
      // Do nothing in this case. The user doesn't need to be told what he just did.
      break;
    case  KIO::ERR_CYCLIC_COPY:
      result = i18n( "Found a cyclic link while copying %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_CREATE_SOCKET:
      result = i18n( "Could not create socket for accessing %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_CONNECT:
      result = i18n( "Could not connect to host %1" ).arg( errorText );
      break;
    case  KIO::ERR_CONNECTION_BROKEN:
      result = i18n( "Connection to host %1 is broken" ).arg( errorText );
      break;
    case  KIO::ERR_NOT_FILTER_PROTOCOL:
      result = i18n( "The protocol %1 is not a filter protocol" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_MOUNT:
      result = i18n( "Could not mount device.\nThe reported error was:\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_UNMOUNT:
      result = i18n( "Could not unmount device.\nThe reported error was:\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_READ:
      result = i18n( "Could not read file %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_WRITE:
      result = i18n( "Could not write to file %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_BIND:
      result = i18n( "Could not bind %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_LISTEN:
      result = i18n( "Could not listen %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_ACCEPT:
      result = i18n( "Could not accept %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_LOGIN:
      result = errorText;
      break;
    case  KIO::ERR_COULD_NOT_STAT:
      result = i18n( "Could not access %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_CLOSEDIR:
      result = i18n( "Could not terminate listing %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_MKDIR:
      result = i18n( "Could not make directory %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_RMDIR:
      result = i18n( "Could not remove directory %1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_RESUME:
      result = i18n( "Could not resume file %1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_RENAME:
      result = i18n( "Could not rename file %1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_CHMOD:
      result = i18n( "Could not change permissions for %1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_DELETE:
      result = i18n( "Could not delete file %1" ).arg( errorText );
      break;
    case  KIO::ERR_SLAVE_DIED:
      result = i18n( "The process for the %1 protocol died unexpectedly." ).arg( errorText );
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
      result = i18n( "Unknown error code %1\n%2\nPlease send a full bug report at http://bugs.kde.org." ).arg( errorCode ).arg( errorText );
      break;
    }

  return result;
}

QString KIO::buildHTMLErrorString(int errorCode, const QString &errorText)
{
  /**
   * % markers used in these strings...
   * 1: URL
   * 2: hostname
   * 3: protocol
   * 4: date and time, locale-adjusted
   */

  QString errorName, action, description;
  QStringList causes, solutions;

  QString sysadmin = i18n( "Contact your system administrator, "
    "techincal support group, or Internet Service Provider (ISP) (in the case of "
    "internet problems) for further assistance." );
  QString serveradmin = i18n( "Contact the administrator of the server, "
    "typically at <a href=\"mailto:webmaster@%3\">webmaster@%3</a>, for further "
    "assistance." );
  // FIXME active link to permissions dialog
  QString access = i18n( "Check your access permissions on this resource." );
  QString locked = i18n( "The file may be in use (and thus locked) by "
    "another user or application." );
  QString querylock = i18n( "Check to make sure that no other "
    "application or user is using the file or has locked the file." );
  QString hardware = i18n( "Although unlikely, a hardware error may have "
    "occurred." );
  QString bug = i18n( "You may have encountered a bug in the program." );
  QString update = i18n( "Update your software to the latest version. "
    "Your distribution should provide tools to update your software." );
  QString bugreport = i18n( "Please consider helping the KDE team by "
    "submitting a high quality bug report. First, look to see if the same bug has "
    "been submitted by someone else by searching at the "
    "<a href=\"http://bugs.kde.org/\">KDE bug reporting website</a>. If not, take "
    "note of the details given above, and include them in your bug report, along "
    "with as many other details as you think might help." );
  QString network = i18n( "There may have been a problem with your network "
    "connection." );
  // FIXME netconf kcontrol link
  QString netconf = i18n( "There may have been a problem with your network "
    "configuration. If you have been accessing the internet with no problems "
    "recently, this is unlikely." );
  QString writeperm = i18n( "You may not have permissions to write to "
    "the file." );

  switch( errorCode ){
    case  KIO::ERR_CANNOT_OPEN_FOR_READING:
      errorName = i18n( "Cannot open resource for reading." );
      description = i18n( "This means that the contents of the requested file "
        "or directory could not be retrieved, as read access could not be obtained." );
      causes << i18n( "You may not have permissions to read the file or open "
        "the directory.") << locked << hardware;
      solutions << access << querylock << sysadmin;
      break;

    case  KIO::ERR_CANNOT_OPEN_FOR_WRITING:
      errorName = i18n( "Cannot open resource for writing." );
      description = i18n( "This means that the file could not be written to "
        "as requested, because access with permission to write could not be "
        "obtained." );
      causes << writeperm << locked << hardware;
      solutions << access << querylock << sysadmin;
      break;

    case  KIO::ERR_CANNOT_LAUNCH_PROCESS:
      errorName = i18n( "Cannot initiate the %3 protocol (unable to launch "
        "process)." );
      description = i18n( "The program on your computer which provides access "
        "to the %3 protocol could not be started. This is usually due to "
        "technical reasons." );
      causes << i18n( "The program which provides compatability with this "
        "protocol may have not been updated with your last update of KDE. "
        "This can cause the program to be incompatible with the current version "
        "and thus not start." ) << bug;
      solutions << update << sysadmin;
      break;

    case  KIO::ERR_INTERNAL:
      errorName = i18n( "Internal error" );
      description = i18n( "The program on your computer which provides access "
        "to the %3 protocol has reported an internal error." );
      causes << i18n( "This is most likely to be caused by a bug in the program. "
        "Please consider submitting a full bug report as detailed below." );
      solutions << update << bugreport;
      break;

    case  KIO::ERR_MALFORMED_URL:
      errorName = i18n( "Inproperly Formatted URL" );
      description = i18n( "The <b>U</b>niversal <b>R</b>esource <b>L</b>ocation "
        "(URL) that you entered was not properly formatted. The format of a URL "
        "is generally as follows:"
        "<blockquote><b>protocol://user@password:www.example.org:port/directory/"
        "filename.extension?query=value</b></blockquote>" );
      solutions << i18n( "Double-check that you have entered a properly formatted "
        "URL and try again." ) << sysadmin;
      break;

    case  KIO::ERR_UNSUPPORTED_PROTOCOL:
      errorName = i18n( "Unsupported Protocol %3" );
      description = i18n( "The protocol %3 is not supported by the programs "
        "currently installed on this computer." );
      causes << i18n( "A protocol was requested that is not supported." )
        << i18n( "The versions of the %3 protocol supported by this computer and "
        "the server are incompatible." );
      solutions << i18n( "You may perform a search on the internet for a KDE "
        "program (called a kioslave or ioslave) which supports this protocol. "
        "Places to search include <a href=\"http://apps.kde.com/\">"
        "http://apps.kde.com/</a> and <a href=\"http://freshmeat.net/\">"
        "http://freshmeat.net/</a>. Click on the following to perform a search "
        "for \"%3 ioslave\" at <a href=\"fm:%3 ioslave\">"
        "freshmeat.net</a>." ) << update << sysadmin;
      break;

    case  KIO::ERR_NO_SOURCE_PROTOCOL:
      errorName = i18n( "URL Does Not Refer to a Resource." );
      description = i18n( "The <b>U</b>niversal <b>R</b>esource <b>L</b>ocation "
        "(URL) that you entered did not refer to a specific resource." );
      causes << i18n( "KDE is able to communicate through a protocol within a "
        "protocol; the protocol specified is only for use in such situations, "
        "however this is not one of these situations. This is a rare event, and "
        "is likely to indicate a programming error." );
      solutions << i18n( "Double-check that you have entered the correct URL "
        "and try again." ) << sysadmin;
      break;

    case  KIO::ERR_UNSUPPORTED_ACTION:
      errorName = i18n( "Unsupported Action: %1" ).arg( errorText );
      description = i18n( "The requested action is not supported by the KDE "
        "program which is implementing the %3 protocol." );
      causes << i18n( "This error is very much dependant on the KDE program. The "
        "additional information should give you more information than is available"
        "to the KDE input / output architecture." );
      solutions << i18n( "Attempt to find another way to accomplish the same outcome." )
        << sysadmin;
      break;

    case  KIO::ERR_IS_DIRECTORY:
      errorName = i18n( "File Expected" );
      description = i18n( "The request expected to return a file, however the "
        "directory %1 was returned instead." ).arg( errorText );
      causes << i18n( "This may be an error on the server side." );
      solutions << sysadmin;
      break;

    case  KIO::ERR_IS_FILE:
      errorName = i18n( "Directory Expected" );
      description = i18n( "The request expected to return a directory, however "
        "the file %1 was returned instead." ).arg( errorText );
      causes << i18n( "This may be an error on the server side." );
      solutions << sysadmin;
      break;

    case  KIO::ERR_DOES_NOT_EXIST:
      errorName = i18n( "File or Directory Does Not Exist" );
      description = i18n( "The request expected to return a directory, however "
        "the file %1 was returned instead." ).arg( errorText );
      causes << i18n( "This may be an error on the server side." );
      solutions << sysadmin;
      break;

    case  KIO::ERR_FILE_ALREADY_EXIST:
      errorName = i18n( "File Already Exists" );
      description = i18n( "The requested file could not be created because a "
        "file with the same name already exists." );
      solutions << i18n ( "Try moving the current file out of the way first, "
        "and then try again." )
        << i18n ( "Delete the current file and try again." )
        << i18n( "Choose an alternate filename for the new file." );
      break;

    case  KIO::ERR_DIR_ALREADY_EXIST:
      errorName = i18n( "Directory Already Exists" );
      description = i18n( "The requested directory could not be created because "
        "a directory with the same name already exists." );
      solutions << i18n( "Try moving the current directory out of the way first, "
        "and then try again." )
        << i18n( "Delete the current directory and try again." )
        << i18n( "Choose an alternate name for the new directory." );
      break;

    case  KIO::ERR_UNKNOWN_HOST:
      errorName = i18n( "Unknown Host" );
      description = i18n( "An unknown host error indicates that the server with "
        "the requested name, for example <b>www.example.org</b>, could not be "
        "located on the internet." );
      causes << i18n( "The name that you typed, %2, may not exist: it may be "
        "incorrectly typed." ) << network << netconf;
      solutions << sysadmin << serveradmin;
      break;

    case  KIO::ERR_ACCESS_DENIED:
      errorName = i18n( "Access Denied" );
      description = i18n( "Access was denied to the specified resource." );
      causes << i18n( "You may have supplied incorrect authentication details or "
        "none at all." )
        << i18n( "Your account may not have access to the specified resource." )
        << i18n( "On occasion this may be caused by an incorrectly configured "
        "server, for example where a script does not have execution permissions." );
      solutions << i18n( "Retry the request and ensure your authentication details "
        "are entered correctly." ) << sysadmin << serveradmin;
      break;

    case  KIO::ERR_WRITE_ACCESS_DENIED:
      errorName = i18n( "Write Access Denied" );
      description = i18n( "This means that an attempt to write to the file "
        "was rejected." );
      causes << writeperm << locked << hardware;
      solutions << access << querylock << sysadmin;
      break;

    case  KIO::ERR_CANNOT_ENTER_DIRECTORY:
      errorName = i18n( "Unable to Enter Directory" );
      description = i18n( "This means that an attempt to enter (in other words, "
        "to open) the requested directory was rejected." );
      causes << writeperm << locked;
      solutions << access << querylock << sysadmin;
      break;

    case  KIO::ERR_PROTOCOL_IS_NOT_A_FILESYSTEM:
      errorName = i18n( "Directory Listing Unavailable" );
      description = i18n( "This means that a request was made which requires "
        "determining the contents of the directory, and the KDE program supporting "
        "this protocol is unable to do so." );
      causes << bug;
      solutions << update << bugreport;
      break;

    case  KIO::ERR_CYCLIC_LINK:
      errorName = i18n( "Cyclic Link Detected" );
      description = i18n( "Unix environments are commonly able to link a file or "
        "directory to a separate name and/or location. KDE detected a link or "
        "series of links that results in an infinite loop - i.e., the file was "
        "(perhaps in a roundabout way) linked to itself." );
      solutions << i18n( "Delete one part of the loop in order that it does not "
        "cause an infinite loop, and try again." ) << sysadmin;
      break;

    case  KIO::ERR_USER_CANCELED:
      // Do nothing in this case. The user doesn't need to be told what he just did.
      break;

    case  KIO::ERR_CYCLIC_COPY:
      errorName = i18n( "Cyclic Link Detected During Copy" );
      description = i18n( "Unix environments are commonly able to link a file or "
        "directory to a separate name and/or location. During the requested copy "
        "operation, KDE detected a link or series of links that results in an "
        "infinite loop - i.e., the file was (perhaps in a roundabout way) linked "
        "to itself." );
      solutions << i18n( "Delete one part of the loop in order that it does not "
        "cause an infinite loop, and try again." ) << sysadmin;
      break;

    case  KIO::ERR_COULD_NOT_CREATE_SOCKET:
      errorName = i18n( "Could Not Create Network Connection (Socket)" );
      description = i18n( "This is a fairly technical error in which a required "
        "device for network communications (a socket) could not be created." );
      causes << i18n( "The network connection may be incorrectly configured, or "
        "the network interface may not be enabled." )
        << i18n( "You may not have permissions to create the device. This occurs "
        "typically in UNIX environments where a non-privileged user trys to open "
        "a port in the privileged range, 1-1024." );
      solutions << i18n( "Check your network connection status." )
        << i18n( "Start konqueror as the root user if required." ) << sysadmin;
      break;

    case  KIO::ERR_COULD_NOT_CONNECT:
      errorName = i18n( "Connection to Server Refused" );
      description = i18n( "An unknown host error indicates that the server with "
        "the requested name, for example <b>www.example.org</b>, could not be "
        "located on the internet." );
      causes << i18n( "The name that you typed, %2, may not exist: it may be "
        "incorrectly typed." ) << network << netconf;
      solutions << sysadmin << serveradmin;
      break;

    case  KIO::ERR_CONNECTION_BROKEN:
      //result = i18n( "Connection to host %1 is broken" ).arg( errorText );
      break;
    case  KIO::ERR_NOT_FILTER_PROTOCOL:
      //result = i18n( "The protocol %1 is not a filter protocol" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_MOUNT:
      //result = i18n( "Could not mount device.\nThe reported error was:\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_UNMOUNT:
      //result = i18n( "Could not unmount device.\nThe reported error was:\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_READ:
      //result = i18n( "Could not read file %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_WRITE:
      //result = i18n( "Could not write to file %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_BIND:
      //result = i18n( "Could not bind %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_LISTEN:
      //result = i18n( "Could not listen %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_ACCEPT:
      //result = i18n( "Could not accept %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_LOGIN:
      //result = errorText;
      break;
    case  KIO::ERR_COULD_NOT_STAT:
      //result = i18n( "Could not access %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_CLOSEDIR:
      //result = i18n( "Could not terminate listing %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_MKDIR:
      //result = i18n( "Could not make directory %1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_RMDIR:
      //result = i18n( "Could not remove directory %1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_RESUME:
      //result = i18n( "Could not resume file %1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_RENAME:
      //result = i18n( "Could not rename file %1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_CHMOD:
      //result = i18n( "Could not change permissions for %1" ).arg( errorText );
      break;
    case  KIO::ERR_CANNOT_DELETE:
      //result = i18n( "Could not delete file %1" ).arg( errorText );
      break;
    case  KIO::ERR_SLAVE_DIED:
      //result = i18n( "The process for the %1 protocol died unexpectedly." ).arg( errorText );
      break;
    case  KIO::ERR_OUT_OF_MEMORY:
      //result = i18n( "Error. Out of Memory.\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_UNKNOWN_PROXY_HOST:
      //result = i18n( "Unknown proxy host\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_COULD_NOT_AUTHENTICATE:
      //result = i18n( "Authorization failed, %1 authentication not supported" ).arg( errorText );
      break;
    case  KIO::ERR_ABORTED:
      //result = i18n( "User canceled action\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_INTERNAL_SERVER:
      //result = i18n( "Internal error in server\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_SERVER_TIMEOUT:
      //result = i18n( "Timeout on server\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_UNKNOWN:
      //result = i18n( "Unknown error\n%1" ).arg( errorText );
      break;
    case  KIO::ERR_UNKNOWN_INTERRUPT:
      //result = i18n( "Unknown interrupt\n%1" ).arg( errorText );
      break;
    case KIO::ERR_CANNOT_DELETE_ORIGINAL:
      //result = i18n( "Could not delete original file %1.\nPlease check permissions." ).arg( errorText );
      break;
    case KIO::ERR_CANNOT_DELETE_PARTIAL:
      //result = i18n( "Could not delete partial file %1.\nPlease check permissions." ).arg( errorText );
      break;
    case KIO::ERR_CANNOT_RENAME_ORIGINAL:
      //result = i18n( "Could not rename original file %1.\nPlease check permissions." ).arg( errorText );
      break;
    case KIO::ERR_CANNOT_RENAME_PARTIAL:
      //result = i18n( "Could not rename partial file %1.\nPlease check permissions." ).arg( errorText );
      break;
    case KIO::ERR_CANNOT_SYMLINK:
      //result = i18n( "Could not create symlink %1.\nPlease check permissions." ).arg( errorText );
      break;
    case KIO::ERR_NO_CONTENT:
      //result = errorText;
      break;
    case KIO::ERR_DISK_FULL:
      //result = i18n( "Could not write file %1.\nDisk full." ).arg( errorText );
      break;
    case KIO::ERR_IDENTICAL_FILES:
      //result = i18n( "The source and destination are the same file.\n%1" ).arg( errorText );
      break;
    default:
      // fall back to the plain error...
      return buildErrorString( errorCode, errorText );
  }

  QString doc = "<html><head><title>";
  doc += i18n( "Error: " );
  doc += errorName;
  doc += " (%1)</title></head><body><h1>";
  doc += i18n( "The requested page could not be retrieved" );
  doc += "</h1><h2>";
  doc += errorName;
  doc += "</h2><h3>";
  doc += i18n( "Details of the Request:" );
  doc += "</h3><ul><li>URL: %1</li><li>";
  doc += i18n( "Date and Time:" );
  doc += " %4</li><li>";
  doc += i18n( "Additional Information:" );
  doc += " %5</li></ul><h3>";
  doc += i18n( "Description:" );
  doc += "</h3><p>";
  doc += description;
  doc += "</p><h3>";
  if ( causes.count() ) {
    doc += i18n( "Possible Causes:" );
    doc += "</h3><ul><li>";
    doc += causes.join( "</li><li>" );
    doc += "</li></ul><h3>";
  }
  doc += i18n( "Possible Solutions:" );
  doc += "</h3><ul><li>";
  doc += solutions.join( "</li><li>" );
  doc += "</li></ul></body></html>";
  return doc;
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
