#include "kio_error.h"
#include "kio_interface.h"

#include <qmessagebox.h>

#include <kapp.h>
#include <klocale.h>
QString kioErrorString( int _errid, const char *_errortext )
{
  QString result;
  
  switch( _errid )
    {
    case  ERR_CANNOT_OPEN_FOR_READING:
	result = i18n( "Could not read\n%1" ).arg( _errortext );      
      break;
    case  ERR_CANNOT_OPEN_FOR_WRITING:
      result = i18n( "Could not write to\n%1" ).arg( _errortext );
      break;
    case  ERR_CANNOT_LAUNCH_PROCESS:
      result = i18n( "Could not start process\n%1" ).arg( _errortext );
      break;
    case  ERR_INTERNAL:
      result = i18n( "Internal Error\nPlease send a full bugreport at http://bugs.kde.org\n\n%1" ).arg( _errortext );
      break;
    case  ERR_MALFORMED_URL:
      result = i18n( "Malformed URL\n%1" ).arg( _errortext );
      break;
    case  ERR_UNSUPPORTED_PROTOCOL:
      result = i18n( "The protocol %1\n is not supported" ).arg( _errortext );
      break;
    case  ERR_NO_SOURCE_PROTOCOL:
      result = i18n( "The protocol %1\nis only a filter protocol.\n").arg( _errortext );
      break;
    case  ERR_UNSUPPORTED_ACTION:
      result = i18n( "Unsupported action\n%1" ).arg( _errortext );
      break;
    case  ERR_DOES_NOT_EXIST:
      result = i18n( "The file or directory\n%1\ndoes not exist" ).arg( _errortext );
      break;
    case  ERR_IS_DIRECTORY:
      result = i18n( "%1\n is a directory,\nbut a file was expected" ).arg( _errortext );
      break;
    case  ERR_COULD_NOT_MKDIR:
      result = i18n( "Could not make directory\n%1" ).arg( _errortext );
      break;
    case  ERR_DOES_ALREADY_EXIST:
      result = i18n( "The file or directory\n%1\ndoes already exist" ).arg( _errortext );
      break;
    case  ERR_ACCESS_DENIED:
      result = i18n( "%1\nAccess denied" ).arg( _errortext );
      break;
    case  ERR_CANNOT_ENTER_DIRECTORY:
      result = i18n( "Could not enter directory\n%1" ).arg( _errortext );
      break;
    case  ERR_PROTOCOL_IS_NOT_A_FILESYSTEM:
      result = i18n( "The protocol %1\ndoes not implement a directory service" ).arg( _errortext );
      break;
    case  ERR_CYCLIC_LINK:
      result = i18n( "Found a cyclic link in\n%1" ).arg( _errortext );
      break;
    case  ERR_CANNOT_CHMOD:
      result = i18n( "Could not change permissions for\n%1" ).arg( _errortext );
      break;
    case  ERR_WRITE_ACCESS_DENIED:
      result = i18n( "Access denied\nCould not write to\n%1" ).arg( _errortext );
      break;
    case  ERR_USER_CANCELED:
      result = i18n( "Action has been\ncanceled by the user\n%1" ).arg( _errortext );
      break;
    case  ERR_CYCLIC_COPY:
      result = i18n( "Found a cyclic link while copying\n%1" ).arg( _errortext );
      break;
    case  ERR_COULD_NOT_CREATE_SOCKET:
      result = i18n( "Could not create socket for accessing\n%1" ).arg( _errortext );
      break;
    case  ERR_COULD_NOT_CONNECT:
      result = i18n( "Could not connect to host\n%1" ).arg( _errortext );
      break;
    case  ERR_UNKNOWN_HOST:
      result = i18n( "Unknown host\n%1" ).arg( _errortext );
      break;
    case  ERR_CONNECTION_BROKEN:
      result = i18n( "Connection to host\n%1\nis broken" ).arg( _errortext );
      break;
    case  ERR_NOT_FILTER_PROTOCOL:
      result = i18n( "The protocol %1\nis not a filter protocol" ).arg( _errortext );
      break;
    case  ERR_IS_FILE:
      result = i18n( "%1\nis a file,\nbut a directory was expected" ).arg( _errortext );
      break;
    case  ERR_COULD_NOT_MOUNT:
      result = i18n( "Could not mount device.\nThe reported error was:\n\n%1" ).arg( _errortext );
      break;
    case  ERR_COULD_NOT_UNMOUNT:
      result = i18n( "Could not unmount device.\nThe reported error was:\n\n%1" ).arg( _errortext );
      break;
    case  ERR_SLAVE_DIED:
      result = i18n( "The process for the\n%1 protocol\ndied unexpectedly" ).arg( _errortext );
      break;
    case  ERR_COULD_NOT_STAT:
      result = i18n( "Could not access\n%1" ).arg( _errortext );
      break;
    case  ERR_CANNOT_RESUME:
      result = i18n( "Could not resume file %1" ).arg( _errortext );
      break;
    case  ERR_CANNOT_DELETE:
      result = i18n( "Could not delete file %1" ).arg( _errortext );
      break;
    case  ERR_COULD_NOT_LOGIN:
      result = i18n( "Could not login into %1" ).arg ( _errortext );
      break;
    case  ERR_COULD_NOT_AUTHENTICATE:
      result = i18n( "Athorization failed, %1 authentication not supported" ).arg( _errortext );
    case  ERR_WARNING:
      result = i18n( "Warning: %1" ).arg( _errortext );
      break;
    default:
      result = i18n( "Unknown error code %1\n%2\n\nPlease send a full bugreport at http://bugs.kde.org" ).arg(_errid ).arg(_errortext);
      break;
    }

  return result;
}

void kioErrorDialog( int _errid, const char *_errortext )
{
  QString tmp = kioErrorString( _errid, _errortext );
  
  QMessageBox::critical( 0, i18n( "Error" ), tmp, i18n( "OK" ) );
}
