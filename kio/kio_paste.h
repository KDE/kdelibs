#ifndef __kio_paste_h__
#define __kio_paste_h__

#include <qstring.h>

void pasteClipboard( const char *_dest_url );
void pasteData( const char *_dest_url, QByteArray _data );

#endif
