/* This file is part of the KDE libraries
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>

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

#include "kxmlguiclient.h"
#include "kxmlguifactory.h"
#include "kxmlguibuilder.h"

#include <qdir.h>
#include <qfile.h>
#include <qdom.h>
#include <qtextstream.h>
#include <qregexp.h>

#include <kinstance.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kaction.h>
#include <kapplication.h>

#include <assert.h>

class KXMLGUIClientPrivate
{
public:
  KXMLGUIClientPrivate()
  {
    m_instance = KGlobal::instance();
    m_factory = 0L;
    m_parent = 0L;
    m_builder = 0L;
    m_actionCollection = 0;
  }
  ~KXMLGUIClientPrivate()
  {
  }

  KInstance *m_instance;

  QDomDocument m_doc;
  KActionCollection *m_actionCollection;
  QDomDocument m_buildDocument;
  KXMLGUIFactory *m_factory;
  KXMLGUIClient *m_parent;
  //QPtrList<KXMLGUIClient> m_supers;
  QPtrList<KXMLGUIClient> m_children;
  KXMLGUIBuilder *m_builder;
  QString m_xmlFile;
  QString m_localXMLFile;
};

KXMLGUIClient::KXMLGUIClient()
{
  d = new KXMLGUIClientPrivate;
}

KXMLGUIClient::KXMLGUIClient( KXMLGUIClient *parent )
{
  d = new KXMLGUIClientPrivate;
  d->m_parent = parent;
}

KXMLGUIClient::~KXMLGUIClient()
{
  if ( d->m_parent )
    d->m_parent->removeChildClient( this );

  /*QPtrListIterator<KXMLGUIClient> superIt( d->m_supers );
  for (; superIt.current(); ++superIt )
    superIt.current()->d->m_children.removeRef( this );

  QPtrListIterator<KXMLGUIClient> childIt( d->m_children );
  for (; childIt.current(); ++childIt )
    childIt.current()->d->m_supers.removeRef( this );
  */

  /*
  d->m_children.setAutoDelete( true );
  d->m_children.clear();
  */
  delete d->m_actionCollection;
  delete d;
}

KAction *KXMLGUIClient::action( const char *name ) const
{
  KAction* act = actionCollection()->action( name );
  if ( !act ) {
    QPtrListIterator<KXMLGUIClient> childIt( d->m_children );
    for (; childIt.current(); ++childIt ) {
      act = childIt.current()->actionCollection()->action( name );
      if ( act )
        break;
    }
  }
  return act;
}

KActionCollection *KXMLGUIClient::actionCollection() const
{
  if ( !d->m_actionCollection )
    d->m_actionCollection = new KActionCollection( 0, 0,
      "KXMLGUILClient-KActionCollection" );
  return d->m_actionCollection;
}

KAction *KXMLGUIClient::action( const QDomElement &element ) const
{
  static const QString &attrName = KGlobal::staticQString( "name" );
  return actionCollection()->action( element.attribute( attrName ).latin1() );
}

KInstance *KXMLGUIClient::instance() const
{
  return d->m_instance;
}

QDomDocument KXMLGUIClient::domDocument() const
{
  return d->m_doc;
}

QString KXMLGUIClient::xmlFile() const
{
  return d->m_xmlFile;
}

QString KXMLGUIClient::localXMLFile() const
{
  if ( !d->m_localXMLFile.isEmpty() )
    return d->m_localXMLFile;

  if ( d->m_xmlFile[0] == '/' )
      return QString::null; // can't save anything here

  return locateLocal( "data", QString::fromLatin1( instance()->instanceName() + '/' ) + d->m_xmlFile );
}


void KXMLGUIClient::reloadXML()
{
    QString file( xmlFile() );
    if ( !file.isEmpty() )
        setXMLFile( file );
}

void KXMLGUIClient::setInstance( KInstance *instance )
{
  d->m_instance = instance;
  actionCollection()->setInstance( instance );
  if ( d->m_builder )
    d->m_builder->setBuilderClient( this );
}

void KXMLGUIClient::setXMLFile( const QString& _file, bool merge, bool setXMLDoc )
{
  // store our xml file name
  if ( !_file.isNull() ) {
    d->m_xmlFile = _file;
    actionCollection()->setXMLFile( _file );
  }

  if ( !setXMLDoc )
    return;

  QString file = _file;
  if ( file[0] != '/' )
  {
    QString doc;

    QString filter = QString::fromLatin1( instance()->instanceName() + '/' ) + _file;

    QStringList allFiles = instance()->dirs()->findAllResources( "data", filter ) + instance()->dirs()->findAllResources( "data", _file );

    file = findMostRecentXMLFile( allFiles, doc );

    if ( file.isEmpty() )
    {
      // this might or might not be an error.  for the time being,
      // let's treat this as if it isn't a problem and the user just
      // wants the global standards file
      setXML( QString::null, true );
      return;
    }
    else if ( !doc.isEmpty() )
    {
      setXML( doc, merge );
      return;
    }
  }

  QString xml = KXMLGUIFactory::readConfigFile( file );
  setXML( xml, merge );
}

void KXMLGUIClient::setLocalXMLFile( const QString &file )
{
    d->m_localXMLFile = file;
}

void KXMLGUIClient::setXML( const QString &document, bool merge )
{
  QDomDocument doc;
  doc.setContent( document );
  setDOMDocument( doc, merge );
}

void KXMLGUIClient::setDOMDocument( const QDomDocument &document, bool merge )
{
  if ( merge )
  {
    QDomElement base = d->m_doc.documentElement();

    QDomElement e = document.documentElement();
    KXMLGUIFactory::removeDOMComments( e );

    // merge our original (global) xml with our new one
    mergeXML(base, e, actionCollection());

    // reassign our pointer as mergeXML might have done something
    // strange to it
    base = d->m_doc.documentElement();

    // we want some sort of failsafe.. just in case
    if ( base.isNull() )
      d->m_doc = document;
  }
  else
  {
    d->m_doc = document;
    KXMLGUIFactory::removeDOMComments( d->m_doc );
  }

  setXMLGUIBuildDocument( QDomDocument() );
}

bool KXMLGUIClient::mergeXML( QDomElement &base, const QDomElement &additive, KActionCollection *actionCollection )
{
  static const QString &tagAction = KGlobal::staticQString( "Action" );
  static const QString &tagMerge = KGlobal::staticQString( "Merge" );
  static const QString &tagSeparator = KGlobal::staticQString( "Separator" );
  static const QString &attrName = KGlobal::staticQString( "name" );
  static const QString &attrAppend = KGlobal::staticQString( "append" );
  static const QString &attrWeakSeparator = KGlobal::staticQString( "weakSeparator" );
  static const QString &tagMergeLocal = KGlobal::staticQString( "MergeLocal" );
  static const QString &tagText = KGlobal::staticQString( "text" );
  static const QString &attrAlreadyVisited = KGlobal::staticQString( "alreadyVisited" );
  static const QString &attrNoMerge = KGlobal::staticQString( "noMerge" );
  static const QString &attrOne = KGlobal::staticQString( "1" );

  // there is a possibility that we don't want to merge in the
  // additive.. rather, we might want to *replace* the base with the
  // additive.  this can be for any container.. either at a file wide
  // level or a simple container level.  we look for the 'noMerge'
  // tag, in any event and just replace the old with the new
  if ( additive.attribute(attrNoMerge) == attrOne ) // ### use toInt() instead? (Simon)
  {
    base.parentNode().replaceChild(additive, base);
    return true;
  }

  QString tag;

  QDomElement e = base.firstChild().toElement();
  // iterate over all elements in the container (of the global DOM tree)
  while ( !e.isNull() )
  {
    tag = e.tagName();

    // if there's an action tag in the global tree and the action is
    // not implemented, then we remove the element
    if ( tag == tagAction )
    {
      QCString name =  e.attribute( attrName ).utf8(); // WABA
      if ( !actionCollection->action( name ) ||
           (kapp && !kapp->authorizeKAction(name)))
      {
        // remove this child as we aren't using it
        QDomElement oldChild = e;
        e = e.nextSibling().toElement();
        base.removeChild( oldChild );
        continue;
      }
    }

    // if there's a separator defined in the global tree, then add an
    // attribute, specifying that this is a "weak" separator
    else if ( tag == tagSeparator )
    {
      e.setAttribute( attrWeakSeparator, (uint)1 );

      // okay, hack time. if the last item was a weak separator OR
      // this is the first item in a container, then we nuke the
      // current one
      QDomElement prev = e.previousSibling().toElement();
      if ( prev.isNull() || 
	 ( prev.tagName() == tagSeparator && !prev.attribute( attrWeakSeparator ).isNull() ) ||
	 ( prev.tagName() == tagText ) )
      {
        // the previous element was a weak separator or didn't exist
        QDomElement oldChild = e;
        e = e.nextSibling().toElement();
        base.removeChild( oldChild );
        continue;
      }
    }

    // the MergeLocal tag lets us specify where non-standard elements
    // of the local tree shall be merged in.  After inserting the
    // elements we delete this element
    else if ( tag == tagMergeLocal )
    {
      QDomElement currElement = e;

      // switch our iterator "e" to the next sibling, so that we don't
      // process the local tree's inserted items!
      e = e.nextSibling().toElement();

      QDomElement it = additive.firstChild().toElement();
      while ( !it.isNull() )
      {
        QDomElement newChild = it;

        it = it.nextSibling().toElement();

        if ( newChild.tagName() == tagText )
          continue;

        if ( newChild.attribute( attrAlreadyVisited ) == attrOne )
          continue;

        QString itAppend( newChild.attribute( attrAppend ) );
        QString elemName( currElement.attribute( attrName ) );

        if ( ( itAppend.isNull() && elemName.isEmpty() ) ||
             ( itAppend == elemName ) )
        {
          // first, see if this new element matches a standard one in
          // the global file.  if it does, then we skip it as it will
          // be merged in, later
          QDomElement matchingElement = findMatchingElement( newChild, base );
          if ( matchingElement.isNull() || newChild.tagName() == tagSeparator )
            base.insertBefore( newChild, currElement );
        }
      }

      base.removeChild( currElement );
      continue;
    }

    // in this last case we check for a separator tag and, if not, we
    // can be sure that its a container --> proceed with child nodes
    // recursively and delete the just proceeded container item in
    // case its empty (if the recursive call returns true)
    else if ( tag != tagMerge )
    {
      // handle the text tag
      if ( tag == tagText )
      {
        e = e.nextSibling().toElement();
        continue;
      }

      QDomElement matchingElement = findMatchingElement( e, additive );

      QDomElement currElement = e;
      e = e.nextSibling().toElement();

      if ( !matchingElement.isNull() )
      {
        matchingElement.setAttribute( attrAlreadyVisited, (uint)1 );

        if ( mergeXML( currElement, matchingElement, actionCollection ) )
        {
          base.removeChild( currElement );
          continue;
        }

        // Merge attributes
        QDomNamedNodeMap attribs = matchingElement.attributes();
        for(uint i = 0; i < attribs.count(); i++)
        {
          QDomNode node = attribs.item(i);
          currElement.setAttribute(node.nodeName(), node.nodeValue());
        }
        
        continue;
      }
      else
      {
        // this is an important case here! We reach this point if the
        // "local" tree does not contain a container definition for
        // this container. However we have to call mergeXML recursively
        // and make it check if there are actions implemented for this
        // container. *If* none, then we can remove this container now
        if ( mergeXML( currElement, QDomElement(), actionCollection ) )
          base.removeChild( currElement );
        continue;
      }
    }

    //I think this can be removed ;-)
    e = e.nextSibling().toElement();
  }

  //here we append all child elements which were not inserted
  //previously via the LocalMerge tag
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

  // do one quick check to make sure that the last element was not
  // a weak separator
  QDomElement last = base.lastChild().toElement();
  if ( (last.tagName() == tagSeparator) && (!last.attribute( attrWeakSeparator ).isNull()) )
  {
    base.removeChild( base.lastChild() );
  }

  // now we check if we are empty (in which case we return "true", to
  // indicate the caller that it can delete "us" (the base element
  // argument of "this" call)
  bool deleteMe = true;
  e = base.firstChild().toElement();
  for ( ; !e.isNull(); e = e.nextSibling().toElement() )
  {
    tag = e.tagName();

    if ( tag == tagAction )
    {
      // if base contains an implemented action, then we must not get
      // deleted (note that the actionCollection contains both,
      // "global" and "local" actions
      if ( actionCollection->action( e.attribute( attrName ).utf8() ) )
      {
        deleteMe = false;
        break;
      }
    }
    else if ( tag == tagSeparator )
    {
      // if we have a separator which has *not* the weak attribute
      // set, then it must be owned by the "local" tree in which case
      // we must not get deleted either
      QString weakAttr = e.attribute( attrWeakSeparator );
      if ( weakAttr.isEmpty() || weakAttr.toInt() != 1 )
      {
        deleteMe = false;
        break;
      }
    }

    // in case of a merge tag we have unlimited lives, too ;-)
    else if ( tag == tagMerge )
    {
      deleteMe = false;
      break;
    }

    // a text tag is NOT enough to spare this container
    else if ( tag == tagText )
    {
      continue;
    }

    // what's left are non-empty containers! *don't* delete us in this
    // case (at this position we can be *sure* that the container is
    // *not* empty, as the recursive call for it was in the first loop
    // which deleted the element in case the call returned "true"
    else
    {
      deleteMe = false;
      break;
    }
  }

  return deleteMe;
}

QDomElement KXMLGUIClient::findMatchingElement( const QDomElement &base, const QDomElement &additive )
{
  static const QString &tagAction = KGlobal::staticQString( "Action" );
  static const QString &tagMergeLocal = KGlobal::staticQString( "MergeLocal" );
  static const QString &attrName = KGlobal::staticQString( "name" );

  QDomElement e = additive.firstChild().toElement();
  for ( ; !e.isNull(); e = e.nextSibling().toElement() )
  {
    // skip all action and merge tags as we will never use them
    if ( ( e.tagName() == tagAction ) || ( e.tagName() == tagMergeLocal ) )
    {
      continue;
    }

    // now see if our tags are equivalent
    if ( ( e.tagName() == base.tagName() ) &&
         ( e.attribute( attrName ) == base.attribute( attrName ) ) )
    {
        return e;
    }
  }

  // nope, return a (now) null element
  return e;
}

void KXMLGUIClient::conserveMemory()
{
  d->m_doc = QDomDocument();
  d->m_buildDocument = QDomDocument();
}

void KXMLGUIClient::setXMLGUIBuildDocument( const QDomDocument &doc )
{
  d->m_buildDocument = doc;
}

QDomDocument KXMLGUIClient::xmlguiBuildDocument() const
{
  return d->m_buildDocument;
}

void KXMLGUIClient::setFactory( KXMLGUIFactory *factory )
{
  d->m_factory = factory;
}

KXMLGUIFactory *KXMLGUIClient::factory() const
{
  return d->m_factory;
}

KXMLGUIClient *KXMLGUIClient::parentClient() const
{
  return d->m_parent;
}

void KXMLGUIClient::insertChildClient( KXMLGUIClient *child )
{
  if (  child->d->m_parent )
    child->d->m_parent->removeChildClient( child );
   d->m_children.append( child );
   child->d->m_parent = this;
}

void KXMLGUIClient::removeChildClient( KXMLGUIClient *child )
{
  assert( d->m_children.containsRef( child ) );
  d->m_children.removeRef( child );
  child->d->m_parent = 0;
}

/*bool KXMLGUIClient::addSuperClient( KXMLGUIClient *super )
{
  if ( d->m_supers.contains( super ) )
    return false;
  d->m_supers.append( super );
  return true;
}*/

const QPtrList<KXMLGUIClient> *KXMLGUIClient::childClients()
{
  return &d->m_children;
}

void KXMLGUIClient::setClientBuilder( KXMLGUIBuilder *builder )
{
  d->m_builder = builder;
  if ( builder )
    builder->setBuilderInstance( instance() );
}

KXMLGUIBuilder *KXMLGUIClient::clientBuilder() const
{
  return d->m_builder;
}

void KXMLGUIClient::plugActionList( const QString &name, const QPtrList<KAction> &actionList )
{
  if ( !d->m_factory )
    return;

  d->m_factory->plugActionList( this, name, actionList );
}

void KXMLGUIClient::unplugActionList( const QString &name )
{
  if ( !d->m_factory )
    return;

  d->m_factory->unplugActionList( this, name );
}

QString KXMLGUIClient::findMostRecentXMLFile( const QStringList &files, QString &doc )
{

  QValueList<DocStruct> allDocuments;

  QStringList::ConstIterator it = files.begin();
  QStringList::ConstIterator end = files.end();
  for (; it != end; ++it )
  {
    //kdDebug() << "KXMLGUIClient::findMostRecentXMLFile " << *it << endl;
    QString data = KXMLGUIFactory::readConfigFile( *it );
    DocStruct d;
    d.file = *it;
    d.data = data;
    allDocuments.append( d );
  }

  QValueList<DocStruct>::Iterator best = allDocuments.end();
  uint bestVersion = 0;

  QValueList<DocStruct>::Iterator docIt = allDocuments.begin();
  QValueList<DocStruct>::Iterator docEnd = allDocuments.end();
  for (; docIt != docEnd; ++docIt )
  {
    QString versionStr = findVersionNumber( (*docIt).data );
    if ( versionStr.isEmpty() )
      continue;

    bool ok = false;
    uint version = versionStr.toUInt( &ok );
    if ( !ok )
      continue;
    //kdDebug() << "FOUND VERSION " << version << endl;

    if ( version > bestVersion )
    {
      best = docIt;
      //kdDebug() << "best version is now " << version << endl;
      bestVersion = version;
    }
  }

  if ( best != docEnd )
  {
    if ( best != allDocuments.begin() )
    {
      QValueList<DocStruct>::Iterator local = allDocuments.begin();

      // load the local document and extract the action properties
      QDomDocument document;
      document.setContent( (*local).data );

      ActionPropertiesMap properties = extractActionProperties( document );

      // in case the document has a ActionProperties section
      // we must not delete it but copy over the global doc
      // to the local and insert the ActionProperties section
      if ( !properties.isEmpty() )
      {
          // now load the global one with the higher version number
          // into memory
          document.setContent( (*best).data );
          // and store the properties in there
          storeActionProperties( document, properties );

          (*local).data = document.toString();
          // make sure we pick up the new local doc, when we return later
          best = local;

          // write out the new version of the local document
          QFile f( (*local).file );
          if ( f.open( IO_WriteOnly ) )
          {
            QCString utf8data = (*local).data.utf8();
            f.writeBlock( utf8data.data(), utf8data.length() );
            f.close();
          }
      }
      else
      {
        QString f = (*local).file;
        QString backup = f + QString::fromLatin1( ".backup" );
        QDir dir;
        dir.rename( f, backup );
      }
    }
    doc = (*best).data;
    return (*best).file;
  }
  else if ( files.count() > 0 )
  {
    //kdDebug() << "returning first one..." << endl;
    doc = (*allDocuments.begin()).data;
    return (*allDocuments.begin()).file;
  }

  return QString::null;
}

QString KXMLGUIClient::findVersionNumber( const QString &_xml )
{
  QString xml = _xml;

  QRegExp versionExpr( ".*<kpartgui.+version=\"([0-9]+)\".*>.*" );
  versionExpr.setCaseSensitive( false );

  QTextStream stream( xml, IO_ReadOnly );
  stream.setEncoding( QTextStream::UnicodeUTF8 );
  QString line = stream.readLine();
  for (; !line.isNull(); line = stream.readLine() )
  {
    if ( line.isEmpty() )
      continue;

    if ( versionExpr.search( line ) > -1 )
        return versionExpr.cap( 1 );
  }

  return QString::null;
}

KXMLGUIClient::ActionPropertiesMap KXMLGUIClient::extractActionProperties( const QDomDocument &doc )
{
  ActionPropertiesMap properties;

  QDomElement actionPropElement = doc.documentElement().namedItem( "ActionProperties" ).toElement();

  if ( actionPropElement.isNull() )
    return properties;

  QDomNode n = actionPropElement.firstChild();
  for (; !n.isNull(); n = n.nextSibling() )
  {
    QDomElement e = n.toElement();
    if ( e.isNull() )
      continue;

    if ( e.tagName().lower() != "action" )
      continue;

    QString actionName = e.attribute( "name" );

    if ( actionName.isEmpty() )
      continue;

    QMap<QString, QMap<QString, QString> >::Iterator propIt = properties.find( actionName );
    if ( propIt == properties.end() )
      propIt = properties.insert( actionName, QMap<QString, QString>() );

    QDomNamedNodeMap attributes = e.attributes();
    for ( uint i = 0; i < attributes.length(); ++i )
    {
      QDomAttr attr = attributes.item( i ).toAttr();

      if ( attr.isNull() )
        continue;

      QString name = attr.name();

      if ( name == "name" || name.isEmpty() )
        continue;

      (*propIt)[ name ] = attr.value();
    }

  }

  return properties;
}

void KXMLGUIClient::storeActionProperties( QDomDocument &doc, const ActionPropertiesMap &properties )
{
  QDomElement actionPropElement = doc.documentElement().namedItem( "ActionProperties" ).toElement();

  if ( actionPropElement.isNull() )
  {
    actionPropElement = doc.createElement( "ActionProperties" );
    doc.documentElement().appendChild( actionPropElement );
  }

  while ( !actionPropElement.firstChild().isNull() )
    actionPropElement.removeChild( actionPropElement.firstChild() );

  ActionPropertiesMap::ConstIterator it = properties.begin();
  ActionPropertiesMap::ConstIterator end = properties.end();
  for (; it != end; ++it )
  {
    QDomElement action = doc.createElement( "Action" );
    action.setAttribute( "name", it.key() );
    actionPropElement.appendChild( action );

    QMap<QString, QString> attributes = (*it);
    QMap<QString, QString>::ConstIterator attrIt = attributes.begin();
    QMap<QString, QString>::ConstIterator attrEnd = attributes.end();
    for (; attrIt != attrEnd; ++attrIt )
      action.setAttribute( attrIt.key(), attrIt.data() );
  }
}

void KXMLGUIClient::addStateActionEnabled(const QString& state,
                                          const QString& action)
{
  StateChange stateChange = getActionsToChangeForState(state);
  
  stateChange.actionsToEnable.append( action );

  m_actionsStateMap.replace( state, stateChange );
}


void KXMLGUIClient::addStateActionDisabled(const QString& state,
                                           const QString& action)
{
  StateChange stateChange = getActionsToChangeForState(state);
  
  stateChange.actionsToDisable.append( action );

  m_actionsStateMap.replace( state, stateChange );
}


KXMLGUIClient::StateChange KXMLGUIClient::getActionsToChangeForState(const QString& state)
{
  return m_actionsStateMap[state];
}


void KXMLGUIClient::stateChanged(const QString &newstate, KXMLGUIClient::ReverseStateChange reverse)
{
  StateChange stateChange = getActionsToChangeForState(newstate);

  bool setTrue = (reverse == StateNoReverse);
  bool setFalse = !setTrue;
  
  // Enable actions which need to be enabled...
  //
  for ( QStringList::Iterator it = stateChange.actionsToEnable.begin();
        it != stateChange.actionsToEnable.end(); ++it ) {

    KAction *action = actionCollection()->action((*it).latin1());
    if (action) action->setEnabled(setTrue);
  }

  // and disable actions which need to be disabled...
  //
  for ( QStringList::Iterator it = stateChange.actionsToDisable.begin();
        it != stateChange.actionsToDisable.end(); ++it ) {

    KAction *action = actionCollection()->action((*it).latin1());
    if (action) action->setEnabled(setFalse);
  }

}

void KXMLGUIClient::beginXMLPlug( QWidget *w )
{
  actionCollection()->beginXMLPlug( w );
  QPtrListIterator<KXMLGUIClient> childIt( d->m_children );
  for (; childIt.current(); ++childIt )
    childIt.current()->actionCollection()->beginXMLPlug( w );
}

void KXMLGUIClient::endXMLPlug()
{
  actionCollection()->endXMLPlug();
  QPtrListIterator<KXMLGUIClient> childIt( d->m_children );
  for (; childIt.current(); ++childIt )
    childIt.current()->actionCollection()->endXMLPlug();
}

void KXMLGUIClient::prepareXMLUnplug( QWidget * )
{
  actionCollection()->prepareXMLUnplug();
  QPtrListIterator<KXMLGUIClient> childIt( d->m_children );
  for (; childIt.current(); ++childIt )
    childIt.current()->actionCollection()->prepareXMLUnplug();
}

void KXMLGUIClient::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }
