#ifndef __kio_paste_h__
#define __kio_paste_h__

#include <qstring.h>
#include <kurl.h>
#include <qarray.h>

namespace KIO {
  void pasteClipboard( const KURL& _dest_url, bool move = false );
  void pasteData( const KURL& _dest_url, const QByteArray& _data );
  bool isClipboardEmpty();
};

#endif
