#ifndef TYPE_H
#define TYPE_H

#include <qtextstream.h>
#include <qdom.h>

static QString writeType( QTextStream& str, const QDomElement& r )
{
  Q_ASSERT( r.tagName() == "TYPE" );
  if ( r.hasAttribute( "qleft" ) )
    str << r.attribute("qleft") << " ";
  QString t = r.firstChild().toText().data();
  t = t.replace( ">>", "> >" );
  str << t;
  if ( r.hasAttribute( "qright" ) )
    str << r.attribute("qright") << " ";
  else
    str << " ";
  return t;
}

/**
 Adjust type for proper prototype
*/
inline QString remapType( const QString& type )
{
    if ( type == "DCOPCString" )
	return QLatin1String("QCString");
    else if ( type == "DCOPCStringList" )
	return QLatin1String("QCStringList");
    else
	return type;
}

#endif
