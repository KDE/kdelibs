#include "kio_error.h"
#include "kio_interface.h"

#include <qmsgbox.h>

#include <kstring.h>
#include <kapp.h>

QString kioErrorString( int _errid, const char *_errortext )
{
  QString result;
  
  switch( _errid )
    {
    case  ERR_CANNOT_OPEN_FOR_READING:
      ksprintf( &result, i18n( "Could not read\n%s" ), _errortext );      
      break;
    case  ERR_CANNOT_OPEN_FOR_WRITING:
      ksprintf( &result, i18n( "Could not write to\n%s" ), _errortext );
      break;
    case  ERR_CANNOT_LAUNCH_PROCESS:
      ksprintf( &result, i18n( "Could not start process\n%s" ), _errortext );
      break;
    case  ERR_INTERNAL:
      ksprintf( &result, i18n( "Internal Error\nPlease send a full bugreport to kde-bugs@kde.org\n\n%s" ), _errortext );
      break;
    case  ERR_MALFORMED_URL:
      ksprintf( &result, i18n( "Malformed URL\n%s" ), _errortext );
      break;
    case  ERR_UNSUPPORTED_PROTOCOL:
      ksprintf( &result, i18n( "The protocol %s\n is not supported" ), _errortext );
      break;
    case  ERR_NO_SOURCE_PROTOCOL:
      ksprintf( &result, i18n( "The protocol %s\nis only a filter protocol.\n"), _errortext );
      break;
    case  ERR_UNSUPPORTED_ACTION:
      ksprintf( &result, i18n( "Unsupported action\n%s" ), _errortext );
      break;
    case  ERR_DOES_NOT_EXIST:
      ksprintf( &result, i18n( "The file or directory\n%s\ndoes not exist" ), _errortext );
      break;
    case  ERR_IS_DIRECTORY:
      ksprintf( &result, i18n( "%s\n is a directory,\nbut a file was expected" ), _errortext );
      break;
    case  ERR_COULD_NOT_MKDIR:
      ksprintf( &result, i18n( "Could not make directory\n%s" ), _errortext );
      break;
    case  ERR_DOES_ALREADY_EXIST:
      ksprintf( &result, i18n( "The file or directory\n%s\ndoes already exist" ), _errortext );
      break;
    case  ERR_ACCESS_DENIED:
      ksprintf( &result, i18n( "%s\nAccess denied" ), _errortext );
      break;
    case  ERR_CANNOT_ENTER_DIRECTORY:
      ksprintf( &result, i18n( "Could not enter directory\n%s" ), _errortext );
      break;
    case  ERR_PROTOCOL_IS_NOT_A_FILESYSTEM:
      ksprintf( &result, i18n( "The protocol %s\ndoes not implement a directory service" ), _errortext );
      break;
    case  ERR_CYCLIC_LINK:
      ksprintf( &result, i18n( "Found a cyclic link in\n%s" ), _errortext );
      break;
    case  ERR_CANNOT_CHMOD:
      ksprintf( &result, i18n( "Could not change permissions for\n%s" ), _errortext );
      break;
    case  ERR_WRITE_ACCESS_DENIED:
      ksprintf( &result, i18n( "Access denied\nCould not write to\n%s" ), _errortext );
      break;
    case  ERR_USER_CANCELED:
      ksprintf( &result, i18n( "Action has been\ncanceled by the user" ), _errortext );
      break;
    case  ERR_CYCLIC_COPY:
      ksprintf( &result, i18n( "Found a cyclic link while copying\n%s" ), _errortext );
      break;
    case  ERR_COULD_NOT_CREATE_SOCKET:
      ksprintf( &result, i18n( "Could not create socket for accessing\n%s" ), _errortext );
      break;
    case  ERR_COULD_NOT_CONNECT:
      ksprintf( &result, i18n( "Could not connect to host\n%s" ), _errortext );
      break;
    case  ERR_UNKNOWN_HOST:
      ksprintf( &result, i18n( "Unknown host\n%s" ), _errortext );
      break;
    case  ERR_CONNECTION_BROKEN:
      ksprintf( &result, i18n( "Connection to host\n%s\nis broken" ), _errortext );
      break;
    case  ERR_NOT_FILTER_PROTOCOL:
      ksprintf( &result, i18n( "The protocol %s\nis not a filter protocol" ), _errortext );
      break;
    case  ERR_IS_FILE:
      ksprintf( &result, i18n( "%s\nis a file,\nbut a directory was expected" ), _errortext );
      break;
    case  ERR_COULD_NOT_MOUNT:
      ksprintf( &result, i18n( "Could not mount device.\nThe reported error was:\n\n%s" ), _errortext );
      break;
    case  ERR_COULD_NOT_UNMOUNT:
      ksprintf( &result, i18n( "Could not unmount device.\nThe reported error was:\n\n%s" ), _errortext );
      break;
    case  ERR_SLAVE_DIED:
      ksprintf( &result, i18n( "The process for the\n%s protocol\ndied unexpectedly" ), _errortext );
      break;
    case  ERR_COULD_NOT_STAT:
      ksprintf( &result, i18n( "Could not access\n%s" ), _errortext );
      break;
    case  ERR_CANNOT_RESUME:
      ksprintf( &result, i18n( "Could not resume file %s" ), _errortext );
      break;
    case  ERR_CANNOT_DELETE:
      ksprintf( &result, i18n( "Could not delete file %s" ), _errortext );
      break;
    default:
      ksprintf( &result, i18n( "Unknown error code %i\n%s\n\nPlease send a full bugreport to kde-bugs@kde.org" ),
		_errortext, _errid );
      break;
    }

  return result;
}

void kioErrorDialog( int _errid, const char *_errortext )
{
  QString tmp = kioErrorString( _errid, _errortext );
  
  QMessageBox::critical( (QWidget*)0L, i18n( "Error" ), tmp, i18n( "Ok" ) );
}
