#ifndef __kio_error_h__
#define __kio_error_h__

#include <qstring.h>

void kioErrorDialog( int _errid, const char *_errortext );
QString kioErrorString( int _errid, const char *_errortext );

#endif
