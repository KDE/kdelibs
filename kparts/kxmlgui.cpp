
#include "kxmlgui.h"


void KXMLGUIFactory::createGUI( KXMLGUIShellServant *shell, KXMLGUIServant *part )
{

  QDomDocument shellXML;
  shellXML.setContent( shell->xml() );
  QDomElement shellDoc = shellXML.documentElement();

  QDomDocument partXML;
  partXML.setContent( part->xml() );
  QDomElement partDoc = partXML.documentElement();

  qDebug( "starting recursive build" );

  buildRecursive( shellDoc, shell, partDoc, part );
}


void KXMLGUIFactory::buildRecursive( const QDomElement &shellElement, KXMLGUIShellServant *shellServant,
                                     const QDomElement &partElement, KXMLGUIServant *partServant,
                                     QObject *parent )
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
    qDebug( "parsing tag %s", e.tagName().ascii() );
    if ( e.tagName() == "Part" && servant == shellServant )
    {
      buildRecursive( QDomElement(), shellServant, e, partServant, parent );
    }
    else if ( e.tagName() == "Action" || e.tagName() == "PluginAction" )
    {
      QAction *action = servant->action( e );
     
      if ( action && parent && parent->inherits( "QWidget" ) )
        action->plug( (QWidget *)parent );

    }
    else
    {
      QWidget *container = shellServant->createContainer( parent, e );

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

        buildRecursive( e, shellServant, p, partServant, container );
      }
      else
      {
	buildRecursive( QDomElement(), shellServant, e, partServant, container );
      }
      
    }
  
  }  

}
