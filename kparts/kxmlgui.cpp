
#include "kxmlgui.h"

#include <qfile.h>

QString KXMLGUIFactory::readConfigFile( const QString &filename )
{
  QFile file( filename );
  if ( !file.open( IO_ReadOnly ) )
    return QString::null;

  uint size = file.size();
  char* buffer = new char[ size + 1 ];
  file.readBlock( buffer, size );
  buffer[ size ] = 0;
  file.close();

  QString text = QString::fromUtf8( buffer, size );
  delete[] buffer;

  return text;
}

void KXMLGUIFactory::createGUI( KXMLGUIServant *shell, KXMLGUIServant *part, KXMLGUIBuilder *builder )
{

  QDomDocument shellXML;
  shellXML.setContent( shell->xml() );
  QDomElement shellDoc = shellXML.documentElement();

  QDomDocument partXML;
  partXML.setContent( part->xml() );
  QDomElement partDoc = partXML.documentElement();

  qDebug( "starting recursive build" );

  buildRecursive( shellDoc, shell, partDoc, part, builder );
}


void KXMLGUIFactory::buildRecursive( const QDomElement &shellElement, KXMLGUIServant *shellServant,
                                     const QDomElement &partElement, KXMLGUIServant *partServant,
                                     KXMLGUIBuilder *builder, QObject *parent )
{

  QDomElement servantElement = shellElement;
  KXMLGUIServant *servant = shellServant;

  if ( servantElement.isNull() )
  {
    qDebug( "switch to part servant" );
    servantElement = partElement;
    servant = partServant;
  }

  QDomElement e = servantElement.firstChild().toElement();
  for (; !e.isNull(); e = e.nextSibling().toElement() )
  {
    qDebug( "parsing tag %s with possible name %s", e.tagName().ascii(), e.attribute( "name" ).ascii() );
    if ( e.tagName() == "Part" && servant == shellServant )
    {
      QDomElement p;

      QDomElement shellParent = e.parentNode().toElement();
      QDomElement partParent = partElement.parentNode().toElement();

      if ( !shellParent.attribute( "name" ).isEmpty() )
      {
        QDomElement i = partParent.firstChild().toElement();
        for (; !i.isNull(); i = i.nextSibling().toElement() )
          if ( i.attribute( "name" ) == shellParent.attribute( "name" ) )
  	  {
	    p = i;
	    break;
          }
      }
      else
      {
	QDomElement i = partParent.firstChild().toElement();
	for (; !i.isNull(); i = i.nextSibling().toElement() )
	  if ( i.tagName() == shellParent.tagName() )
	  {
	    p = i;
	    break;
	  }
      }

      buildRecursive( QDomElement(), shellServant, p, partServant, builder, parent );
    }
    else if ( e.tagName() == "Action" )
    {
      QAction *action = servant->action( e );
     
      if ( action && parent && parent->inherits( "QWidget" ) )
        action->plug( (QWidget *)parent );

      if ( !action )
        qDebug( "cannot find action %s", e.attribute( "name" ).ascii() );

    }
    else
    {
      QWidget *container = builder->createContainer( parent, e );

      if ( !container )
        continue;

      if ( servant == shellServant )
      {
        QDomElement p;
 
	if ( !e.attribute( "name" ).isEmpty() )
	{
          QDomElement i = partElement.firstChild().toElement();
          for (; !i.isNull(); i = i.nextSibling().toElement() )
            if ( i.attribute( "name" ) == e.attribute( "name" ) )
  	    {
	      p = i;
	      break;
            }
        }
	else
        {
	  QDomElement i = partElement.firstChild().toElement();
	  for (; !i.isNull(); i = i.nextSibling().toElement() )
	    if ( i.tagName() == e.tagName() )
	    {
	      p = i;
	      break;
	    }
	}

        buildRecursive( e, shellServant, p, partServant, builder, container );
      }
      else
      {
	buildRecursive( QDomElement(), shellServant, e, partServant, builder, container );
      }
      
    }
  
  }  

}
