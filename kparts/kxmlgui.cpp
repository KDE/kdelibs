
#include "kxmlgui.h"

#include <qfile.h>

// #define DEBUG_BUILDER

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

void KXMLGUIFactory::mergeXML( QDomElement base, QDomElement additive )
{
  QDomElement e = base.firstChild().toElement();
  for (; !e.isNull(); e = e.nextSibling().toElement() )
  {
    QDomElement matchingElement = findMatchingElement( e, additive );

    if ( !matchingElement.isNull() )
      mergeXML( e, matchingElement );

  }

  e = additive.firstChild().toElement();
  while ( !e.isNull() )
  {
    QDomElement matchingElement = findMatchingElement( e, base );

    if ( matchingElement.isNull() )
    {
      QDomElement newChild = e;
      e = e.nextSibling().toElement();
      base.appendChild( newChild );
    }
    else
      e = e.nextSibling().toElement();

  }

}

void KXMLGUIFactory::createGUI( KXMLGUIServant *shell, KXMLGUIServant *part, KXMLGUIBuilder *builder )
{

#ifdef DEBUG_BUILDER
  qDebug( "starting recursive build" );
#endif

  KXMLGUIFactory factory( shell, part, builder );

  factory.buildRecursive( shell->document().documentElement(), part->document().documentElement() );
}

KXMLGUIFactory::KXMLGUIFactory( KXMLGUIServant *shellServant, KXMLGUIServant *partServant, KXMLGUIBuilder *builder )
{
  m_shellServant = shellServant;
  m_partServant = partServant;
  m_builder = builder;
}

void KXMLGUIFactory::buildRecursive( const QDomElement &shellElement,
                                     const QDomElement &partElement,
                                     QObject *parent )
{

  QDomElement servantElement = shellElement;
  KXMLGUIServant *servant = m_shellServant;

  QStringList shellElementChildNames;

  if ( servantElement.isNull() )
  {
#ifdef DEBUG_BUILDER
    qDebug( "switch to part servant" );
#endif
    servantElement = partElement;
    servant = m_partServant;
  }
  else
  {
    QDomElement e = shellElement.firstChild().toElement();
    for (; !e.isNull(); e = e.nextSibling().toElement() )
      shellElementChildNames.append( e.tagName() );
  }

  QDomElement e = servantElement.firstChild().toElement();
  for (; !e.isNull(); e = e.nextSibling().toElement() )
  {
#ifdef DEBUG_BUILDER
    qDebug( "parsing tag %s with possible name %s", e.tagName().ascii(), e.attribute( "name" ).ascii() );
#endif
    if ( e.tagName() == "Part" && servant == m_shellServant )
    {
      QDomElement p = findMatchingElement( e.parentNode().toElement(),
                                           partElement.parentNode().toElement() );

      buildRecursive( QDomElement(), p, parent );
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
      QWidget *container = m_builder->createContainer( parent, e );

      if ( !container )
        continue;

      if ( servant == m_shellServant )
        buildRecursive( e,
                        findMatchingElement( e, partElement ),
                        container );
      else
	buildRecursive( QDomElement(), e, container );

    }

  }

  if ( shellElementChildNames.count() > 0 && !parent )
  {
    QDomElement e = partElement.firstChild().toElement();
    for (; !e.isNull(); e = e.nextSibling().toElement() )
      if ( !shellElementChildNames.contains( e.tagName() ) )
      {
	QWidget *container = m_builder->createContainer( 0, e );

	if ( !container )
	  return;

	buildRecursive( QDomElement(), e, container );
      }
  }

}

QDomElement KXMLGUIFactory::findMatchingElement( const QDomElement &shellElement, const QDomElement &partElement )
{
  QDomElement p;
  QDomElement i = partElement.firstChild().toElement();

  QString name = shellElement.attribute( "name" );

  if ( !name.isEmpty() )
  {
    for (; !i.isNull(); i = i.nextSibling().toElement() )
      if ( i.attribute( "name" ) == name )
      {
        p = i;
        break;
      }
    return p;
  }

  name = shellElement.tagName();

  for (; !i.isNull(); i = i.nextSibling().toElement() )
    if ( i.tagName() == name )
      {
	p = i;
	break;
      }

  return p;
}
