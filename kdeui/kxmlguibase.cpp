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

#include <kxmlguibase.h>

#include <qdom.h>
#include <qaction.h>

#include <kinstance.h>
#include <kstddirs.h>
#include <kdebug.h>

#include <qtextstream.h>
static void dump_xml(const QDomElement& elem)
{
    QString doc;
    QTextStream ts(&doc, IO_WriteOnly);
    ts << elem;
    qDebug("%s", doc.ascii());
}

class KXMLGUIBasePrivate
{
public:
  KXMLGUIBasePrivate()
  {
    m_instance = KGlobal::instance();
  }
  ~KXMLGUIBasePrivate()
  {
  }

  KInstance *m_instance;

  QDomDocument m_doc;
  QActionCollection m_actionCollection;
};

KXMLGUIBase::KXMLGUIBase()
{
  d = new KXMLGUIBasePrivate; 
}

KXMLGUIBase::~KXMLGUIBase()
{
  delete d; 
}

QAction *KXMLGUIBase::action( const char *name )
{
  return d->m_actionCollection.action( name ); 
}

QActionCollection *KXMLGUIBase::actionCollection() const
{
  return &d->m_actionCollection; 
}

QAction *KXMLGUIBase::action( const QDomElement &element )
{
  static QString attrName = QString::fromLatin1( "name" ); 
  return action( element.attribute( attrName ).ascii() );
}

KInstance *KXMLGUIBase::instance() const
{
  return d->m_instance; 
} 

QDomDocument KXMLGUIBase::document() const
{
  return d->m_doc; 
} 

void KXMLGUIBase::setInstance( KInstance *instance )
{
  d->m_instance = instance;
} 

void KXMLGUIBase::setXMLFile( QString file, bool merge )
{
  if ( file[0] != '/' )
  {
    file = locate( "data", QString(instance()->instanceName())+"/"+file );
    if ( file.isEmpty() )
    {
      kDebugError( 1000, "File not found : %s", file.ascii() );
      return;
    }
  }

  QString xml = KXMLGUIFactory::readConfigFile( file );
  setXML( xml, merge );
}

void KXMLGUIBase::setXML( const QString &document, bool merge )
{
  if ( merge )
  {
    QDomElement base = d->m_doc.documentElement();
    QDomDocument doc;

    if ( !document.isNull() )
      doc.setContent( document );

    mergeXML(base, doc.documentElement(), actionCollection());
    dump_xml(base.toElement());

    // we want some sort of failsafe.. just in case
    if ( base.isNull() )
      d->m_doc.setContent( document );
  }
  else
    d->m_doc.setContent( document );
} 

bool KXMLGUIBase::mergeXML( QDomElement &base, const QDomElement &additive, QActionCollection *actionCollection )
{
  static QString tagAction = QString::fromLatin1( "Action" );
  static QString tagMerge = QString::fromLatin1( "Merge" );
  static QString tagSeparator = QString::fromLatin1( "Separator" );
  static QString attrName = QString::fromLatin1( "name" );
  static QString attrAppend = QString::fromLatin1( "append" );
  static QString attrWeakSeparator = QString::fromLatin1( "weakSeparator" );
  static QString tagMergeLocal = QString::fromLatin1( "MergeLocal" );
  static QString tagText = QString::fromLatin1( "text" );
  static QString attrAlreadyVisited = QString::fromLatin1( "alreadyVisited" );

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
      if ( !actionCollection->action( e.attribute( attrName ).utf8() ) )
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

      // okay, hack time. if the last item was a weak separator, then
      // we nuke the current one
      QDomElement prev = e.previousSibling().toElement();
      if ( prev.tagName() == tagSeparator && !prev.attribute( attrWeakSeparator ).isNull() )
      {
        // the previous element was a weak separator
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

        if ( newChild.attribute( attrAlreadyVisited ) == "1" )
          continue;

        QString itAppend( newChild.attribute( attrAppend ) );
        QString elemName( currElement.attribute( attrName ) );

        if ( ( itAppend.isNull() && elemName.isNull() ) ||
             ( itAppend == elemName ) )
        {
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
          base.removeChild( currElement );
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

QDomElement KXMLGUIBase::findMatchingElement( const QDomElement &base, const QDomElement &additive )
{
  static QString tagAction = QString::fromLatin1( "Action" );
  static QString tagMergeLocal = QString::fromLatin1( "MergeLocal" );
  static QString attrName = QString::fromLatin1( "name" );

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
