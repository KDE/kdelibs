#include "qdom.h"
#include "qxml.h"
#include "qmap.h"
#include "qtextstream.h"
#include "qiodevice.h"
#include "qpixmap.h"

#include <string.h>
#include <stdlib.h>

template class QDict<QDOM_NodePrivate>;

// NOT REVISED

/**
 * TODO:
 * If the document dies, remove all pointers to it from children
 * which can not be deleted at this time.
 *
 * If a node dies and has direct children which can not be deleted,
 * then remove the pointer to the parent.
 *
 * Handle QDomDocumentFragment on insertion correctly.
 *
 * createElement and friends create double reference counts.
 */

/**
 * Reference counting:
 *
 * Some simple rules:
 * 1) If an intern object returns a pointer to another intern object
 *    then the reference count of the returned object is not increased.
 * 2) If an extern object is created and gets a pointer to some intern
 *    object, then the extern object increases the intern objects reference count.
 * 3) If an extern object is deleted, then it decreases the reference count
 *    on its associated intern object and deletes it if nobody else hold references
 *    on the intern object.
 */

/**************************************************************
 *
 * QDOMConsumer
 *
 **************************************************************/

class QDomConsumer : public QXMLConsumer
{
public:
  QDomConsumer( QDOM_DocumentPrivate* d );
  ~QDomConsumer();

  virtual bool tagStart( const QString& name );
  virtual bool tagEnd( const QString& name );
  virtual bool attrib( const QString& name, const QString& value );
  virtual bool text( const QString& text );
  virtual bool cdata( const QString& text );
  virtual bool entityRef( const QString& name );
  virtual bool processingInstruction( const QString& name, const QString& value );
  virtual bool doctype( const QString& name );
  virtual bool doctypeExtern( const QString& publicId, const QString& systemId );
  // virtual bool parameterEntityRef( const QString& name );
  virtual bool element( const QString& data );
  virtual bool attlist( const QString& data );
  virtual bool parameterEntity( const QString& name, const QString& publicId, const QString& systemId );
  virtual bool parameterEntity( const QString& name, const QString& value );
  virtual bool entity( const QString& name, const QString& publicId, const QString& systemId, const QString& ndata );
  virtual bool entity( const QString& name, const QString& value );
  virtual bool notation( const QString& name, const QString& publicId, const QString& systemId );
  virtual void parseError( int pos, int line, int linepos );
  virtual bool finished();

private:
  QDOM_DocumentPrivate* doc;
  QDOM_NodePrivate* node;
  bool firstTag;
};

/*==============================================================*/
/*                Implementation                                */
/*==============================================================*/

/**************************************************************
 *
 * QDOM_ImplementationPrivate
 *
 **************************************************************/

class QDOM_ImplementationPrivate : public QShared
{
public:
  QDOM_ImplementationPrivate();
  ~QDOM_ImplementationPrivate();

  QDOM_ImplementationPrivate* clone();

  bool hasFeature( const QString& feature, const QString& version );
};

QDOM_ImplementationPrivate::QDOM_ImplementationPrivate()
{
}

QDOM_ImplementationPrivate::~QDOM_ImplementationPrivate()
{
}

bool QDOM_ImplementationPrivate::hasFeature( const QString& feature, const QString& version )
{
  if ( feature == "XML" )
    if ( version.isEmpty() || version == "1.0" )
      return true;

  return false;
}

QDOM_ImplementationPrivate* QDOM_ImplementationPrivate::clone()
{
    QDOM_ImplementationPrivate* p = new QDOM_ImplementationPrivate;
    // We are not interested in this node
    p->deref();
    return p;
}

/**************************************************************
 *
 * QDomImplementation
 *
 **************************************************************/

/*!
  \class QDomImplementation qdom.h
  \brief QDomImplementation provides access to implementation features of the DOM implementation.

  \ingroup dom

  Like all QDom classes, QDomImplementation is explicit shared. That means there is an
  internal object which is deleted when no QDomImplementation instance references it any more.
  Creating copies of QDomImplementation creates only new references but it does never copy
  the internal object. Unlike other explicit shared Qt classes like QArray there does not
  exist a method called detach.

  The advatnage of this reference counting is that you dont need to care about memory leaks.
*/

/*!
 This constructor creates a QDomImplementation which references no internal
 object. That means that isNull() will return TRUE.
*/
QDomImplementation::QDomImplementation()
{
  impl = 0;
}

/*!
 This copy constructor does not really create a copy of the QDomImplementation
 since QDomImplementation is explicit shared. Instead it creates a new
 reference to the internal object.
*/
QDomImplementation::QDomImplementation( const QDomImplementation& x )
{
  impl = x.impl;
  if ( impl ) impl->ref();
}

/*!
  Internal function
*/
QDomImplementation::QDomImplementation( QDOM_ImplementationPrivate* p )
{
  // We want to be co-owners, so increase the reference count
  impl = p;
}

/*!
  This assignment operator causes the QDomImplementation instance to reference
  another internal object. This does not really create a copy since QDomImplementation is
  explicit shared.
*/
QDomImplementation& QDomImplementation::operator= ( const QDomImplementation& x )
{
  if ( x.impl ) x.impl->ref();		//avoid x=x
  if ( impl && impl->deref() ) delete impl;
  impl = x.impl;

  return *this;
}

/*!
  Compares wether both QDomImplementation objects reference the same
  internal object.
*/
bool QDomImplementation::operator==( const QDomImplementation& x ) const
{
  return ( impl == x.impl );
}

/*!
  Returns TRUE if both QDomImplementation objects reference different
  internal objects.
*/
bool QDomImplementation::operator!=( const QDomImplementation& x ) const
{
  return ( impl != x.impl );
}

/*!
  The destructor does not by default destroy the referenced internal object.
  The internal representation is deleted when no QDomImplementation instance
  references it any more.
*/
QDomImplementation::~QDomImplementation()
{
  if ( impl && impl->deref() ) delete impl;
}

/*!
  The method returns TRUE if QDom implements the requested \e version of a \e feature.

  Currently only the feature "XML" in Version "1.0" is supported.
*/
bool QDomImplementation::hasFeature( const QString& feature, const QString& version )
{
  if ( !impl )
    return FALSE;

  return impl->hasFeature( feature, version );
}

/*!
  Returns TRUE if no internal object is referenced.
*/
bool QDomImplementation::isNull()
{
    return ( impl == 0 );
}

/*==============================================================*/
/*                       NodeList                               */
/*==============================================================*/

/**************************************************************
 *
 * QDOM_NodeListPrivate
 *
 **************************************************************/

class QDOM_NodeListPrivate : public QShared
{
public:
  QDOM_NodeListPrivate();
  virtual ~QDOM_NodeListPrivate();

  virtual QDOM_NodePrivate* item( int index ) = 0;
  virtual uint length() const = 0;
};

QDOM_NodeListPrivate::QDOM_NodeListPrivate()
{
}

QDOM_NodeListPrivate::~QDOM_NodeListPrivate()
{
}

/**************************************************************
 *
 * QDomNodeList
 *
 **************************************************************/

/*!
  \class QDomNodeList qdom.h
  \brief QDomNodeList is a container for QDomNode objects.

  \ingroup dom

  Like all QDom classes, QDomNodeList is explicit shared. That means there is an
  internal object which is deleted when no QDomNodeList instance references it any more.
  Creating copies of QDomNodeList creates only new references but it does never copy
  the internal object. Unlike other explicit shared Qt classes like QArray there does not
  exist a method called detach.

  The advatnage of this reference counting is that you dont need to care about memory leaks.
*/

/*!
 This constructor creates a QDomNodeList which references no internal
 object. That means that isNull() will return TRUE.
*/
QDomNodeList::QDomNodeList()
{
  impl = 0;
}

/*!
 This copy constructor does not really create a copy of the QDomNodeList
 since QDomNodeList is explicit shared. Instead it creates a new
 reference to the internal object.
*/
QDomNodeList::QDomNodeList( QDOM_NodeListPrivate* p )
{
  impl = p;
  if ( impl ) impl->ref();
}

/*!
  Internal function
*/
QDomNodeList::QDomNodeList( const QDomNodeList& n )
{
  impl = n.impl;
  if ( impl )
    impl->ref();
}

/*!
  This assignment operator causes the QDomNodeList instance to reference
  another internal object. This does not really create a copy since QDomNodeList is
  explicit shared.
*/
QDomNodeList& QDomNodeList::operator= ( const QDomNodeList& n )
{
  if ( n.impl )
    n.impl->ref();
  if ( impl && impl->deref() ) delete impl;
  impl = n.impl;

  return *this;
}

/*!
  Compares wether both QDomNodeList objects reference the same
  internal object.
*/
bool QDomNodeList::operator== ( const QDomNodeList& n ) const
{
  return ( impl == n.impl );
}

/*!
  Returns TRUE if both QDomNodeList objects reference different
  internal objects.
*/
bool QDomNodeList::operator!= ( const QDomNodeList& n ) const
{
  return ( impl != n.impl );
}

/*!
  The destructor does not by default destroy the referenced internal object.
  The internal representation is deleted when no QDomNodeList instance
  references it any more.
*/
QDomNodeList::~QDomNodeList()
{
  if ( impl && impl->deref() ) delete impl;
}

/*!
  Returns the Node at position \e index. If index < 0 or
  index >= length() then a null node is returned. That
  means that the returned nodes QDomNode::isNull() method
  will return TRUE.
*/
QDomNode QDomNodeList::item( int index ) const
{
  if ( !impl )
    return QDomNode();

  return QDomNode( impl->item( index ) );
}

/*!
  Returns the amount of nodes in the list.
*/
uint QDomNodeList::length() const
{
  if ( !impl )
    return 0;
  return impl->length();
}

/*==============================================================*/
/*==============================================================*/

/**************************************************************
 *
 * QDOM_NodePrivate
 *
 **************************************************************/

class QDOM_NodePrivate : public QDOM_NodeListPrivate
{
public:
  QDOM_NodePrivate( QDOM_DocumentPrivate*, QDOM_NodePrivate* parent = 0 );
  QDOM_NodePrivate( QDOM_NodePrivate* n, bool deep );
  ~QDOM_NodePrivate();

  QString nodeName() const { return m_name; }
  QString nodeValue() const { return m_value; }
  void setNodeValue( const QString& v ) { m_value = v; }

  QDOM_NodePrivate*         parentNode();
  QDOM_NodeListPrivate*     childNodes();
  QDOM_NodePrivate*         firstChild();
  QDOM_NodePrivate*         lastChild();
  QDOM_NodePrivate*         previousSibling();
  QDOM_NodePrivate*         nextSibling();
  QDOM_DocumentPrivate*     ownerDocument();

  virtual QDOM_NamedNodeMapPrivate* attributes();
  virtual QDOM_NodePrivate* insertBefore( QDOM_NodePrivate* newChild, QDOM_NodePrivate* refChild );
  virtual QDOM_NodePrivate* insertAfter( QDOM_NodePrivate* newChild, QDOM_NodePrivate* refChild );
  virtual QDOM_NodePrivate* replaceChild( QDOM_NodePrivate* newChild, QDOM_NodePrivate* oldChild );
  virtual QDOM_NodePrivate* removeChild( QDOM_NodePrivate* oldChild );
  virtual QDOM_NodePrivate* appendChild( QDOM_NodePrivate* newChild );
  bool  hasChildNodes() const;

  QDOM_NodePrivate* namedItem( const QString& name );

  virtual QDOM_NodePrivate* cloneNode( bool deep = TRUE );
  virtual void clear();

  QDOM_NodePrivate* item( int index );
  uint length() const;

  void setParent( QDOM_NodePrivate* );

  // Dynamic cast
  virtual bool isAttr() { return false; }
  virtual bool isCDATASection() { return false; }
  virtual bool isDocumentFragment() { return false; }
  virtual bool isDocument() { return false; }
  virtual bool isDocumentType() { return false; }
  virtual bool isElement() { return false; }
  virtual bool isEntityReference() { return false; }
  virtual bool isText() { return false; }
  virtual bool isEntity() { return false; }
  virtual bool isNotation() { return false; }
  virtual bool isProcessingInstruction() { return false; }
  virtual bool isCharacterData() { return false; }

  virtual void save( QTextStream& ) const;

  // Variables
  QDOM_NodePrivate* m_previousSibling;
  QDOM_NodePrivate* m_nextSibling;
  QDOM_NodePrivate* m_parentNode;
  QDOM_NodePrivate* m_firstChild;
  QDOM_NodePrivate* m_lastChild;

  QString m_name;
  QString m_value;
};

QDOM_NodePrivate::QDOM_NodePrivate( QDOM_DocumentPrivate* /* qd */, QDOM_NodePrivate *par )
{
  m_parentNode = par;
  m_previousSibling = 0;
  m_nextSibling = 0;
  m_firstChild = 0;
  m_lastChild = 0;
}

QDOM_NodePrivate::QDOM_NodePrivate( QDOM_NodePrivate* n, bool deep )
{
  m_parentNode = 0;
  m_previousSibling = 0;
  m_nextSibling = 0;
  m_firstChild = 0;
  m_lastChild = 0;

  m_name = n->m_name;
  m_value = n->m_value;
  
  if ( !deep )
    return;

  uint len = n->length();

  for( uint i = 0; i < len; ++i )
  {
    QDOM_NodePrivate* x = n->item( i )->cloneNode( TRUE );
    ASSERT( x->count == 0 );
    // qDebug("...copy1 %i %s", x->count, x->nodeName().latin1() );
    appendChild( x );
    ASSERT( x->count == 1 );
    // qDebug("...copy2 %i %s", x->count, x->nodeName().latin1() );
  }
}

QDOM_NodePrivate::~QDOM_NodePrivate()
{
    QDOM_NodePrivate* p = m_firstChild;
    QDOM_NodePrivate* n;

    while( p )
    {
	// qDebug("...clear %i %s", p->count, p->nodeName().latin1() );
	n = p->nextSibling();
	if ( p->deref() )
	    delete p;
	else
	    p->m_parentNode = 0;
	p = n;
    }

    m_firstChild = 0;
    m_lastChild = 0;
}

void QDOM_NodePrivate::clear()
{
  QDOM_NodePrivate* p = m_firstChild;
  QDOM_NodePrivate* n;

  while( p )
  {
    n = p->nextSibling();
    if ( p->deref() )
      delete p;
    p = n;
  }

  m_firstChild = 0;
  m_lastChild = 0;
}

QDOM_NodePrivate* QDOM_NodePrivate::namedItem( const QString& name )
{
  QDOM_NodePrivate* p = m_firstChild;
  while( p )
  {
    if ( p->nodeName() == name )
      return p;
    p = p->nextSibling();
  }

  return 0;
}

QDOM_NamedNodeMapPrivate* QDOM_NodePrivate::attributes()
{
  return 0;
}

QDOM_NodePrivate* QDOM_NodePrivate::insertBefore( QDOM_NodePrivate* newChild, QDOM_NodePrivate* refChild )
{
  // Error check
  if ( !newChild )
    return 0;

  // Error check
  if ( newChild == refChild )
      return 0;

  // Error check
  if ( refChild && refChild->parentNode() != this )
    return 0;

  // Special handling for inserting a fragment. We just insert
  // all elements of the fragment instead of the fragment itself.
  if ( newChild->isDocumentFragment() )
  {
      // Fragment is empty ?
      if ( newChild->m_firstChild == 0 )
	  return newChild;

      // New parent
      QDOM_NodePrivate* n = newChild->m_firstChild;
      while( n )
      {
	  n->m_parentNode = this;
	  n = n->m_nextSibling;
      }

      // Insert at the beginning ?
      if ( !refChild || refChild->m_previousSibling == 0 )
      {
	  if ( m_firstChild )
	      m_firstChild->m_previousSibling = newChild->m_lastChild;
	  newChild->m_lastChild->m_nextSibling = m_firstChild;
	  if ( !m_lastChild )
	      m_lastChild = newChild->m_lastChild;
	  m_firstChild = newChild->m_firstChild;
      }
      // Insert in the middle
      else
      {
	  newChild->m_lastChild->m_nextSibling = refChild;
	  newChild->m_firstChild->m_previousSibling = refChild->previousSibling();
	  refChild->m_previousSibling->m_nextSibling = newChild->m_firstChild;
	  refChild->m_previousSibling = newChild->m_lastChild;
      }

      // No need to increase the reference since QDomDocumentFragment
      // does not decrease the reference.

      // Remove the nodes from the fragment
      newChild->m_firstChild = 0;
      newChild->m_lastChild = 0;
      return newChild;
  }

  // No more errors can occure now, so we take
  // ownership of the node.
  newChild->ref();

  if ( newChild->parentNode() )
    newChild->parentNode()->removeChild( newChild );

  newChild->m_parentNode = this;

  if ( !refChild )
  {
    if ( m_firstChild )
      m_firstChild->m_previousSibling = newChild;
    newChild->m_nextSibling = m_firstChild;
    if ( !m_lastChild )
      m_lastChild = newChild;
    m_firstChild = newChild;
    return newChild;
  }

  if ( refChild->previousSibling() == 0 )
  {
    if ( m_firstChild )
      m_firstChild->m_previousSibling = newChild;
    newChild->m_nextSibling = m_firstChild;
    if ( !m_lastChild )
      m_lastChild = newChild;
    m_firstChild = newChild;
    return newChild;
  }

  newChild->m_nextSibling = refChild;
  newChild->m_previousSibling = refChild->previousSibling();
  refChild->m_previousSibling->m_nextSibling = newChild;
  refChild->m_previousSibling = newChild;

  return newChild;
}

QDOM_NodePrivate* QDOM_NodePrivate::insertAfter( QDOM_NodePrivate* newChild, QDOM_NodePrivate* refChild )
{
  // Error check
  if ( !newChild )
    return 0;

  // Error check
  if ( newChild == refChild )
      return 0;

  // Error check
  if ( refChild && refChild->parentNode() != this )
    return 0;

  // Special handling for inserting a fragment. We just insert
  // all elements of the fragment instead of the fragment itself.
  if ( newChild->isDocumentFragment() )
  {
      // Fragment is empty ?
      if ( newChild->m_firstChild == 0 )
	  return newChild;

      // New parent
      QDOM_NodePrivate* n = newChild->m_firstChild;
      while( n )
      {
	  n->m_parentNode = this;
	  n = n->m_nextSibling;
      }

      // Insert at the end
      if ( !refChild || refChild->m_nextSibling == 0 )
      {
	  if ( m_lastChild )
	      m_lastChild->m_nextSibling = newChild->m_firstChild;
	  newChild->m_firstChild->m_previousSibling = m_lastChild;
	  if ( !m_firstChild )
	      m_firstChild = newChild->m_firstChild;
	  m_lastChild = newChild->m_lastChild;	
      }
      // Insert in the middle
      else
      {
	  newChild->m_firstChild->m_previousSibling = refChild;
	  newChild->m_lastChild->m_nextSibling = refChild->nextSibling();
	  refChild->m_nextSibling->m_previousSibling = newChild->m_lastChild;
	  refChild->m_nextSibling = newChild->m_firstChild;
      }

      // No need to increase the reference since QDomDocumentFragment
      // does not decrease the reference.

      // Remove the nodes from the fragment
      newChild->m_firstChild = 0;
      newChild->m_lastChild = 0;
      return newChild;
  }

  // Release new node from its current parent
  if ( newChild->parentNode() )
    newChild->parentNode()->removeChild( newChild );

  // No more errors can occure now, so we take
  // ownership of the node
  newChild->ref();

  newChild->m_parentNode = this;

  // Insert at the end
  if ( !refChild )
  {
    if ( m_lastChild )
      m_lastChild->m_nextSibling = newChild;
    newChild->m_previousSibling = m_lastChild;
    if ( !m_firstChild )
      m_firstChild = newChild;
    m_lastChild = newChild;
    return newChild;
  }

  if ( refChild->nextSibling() == 0 )
  {
    if ( m_lastChild )
      m_lastChild->m_nextSibling = newChild;
    newChild->m_previousSibling = m_lastChild;
    if ( !m_firstChild )
      m_firstChild = newChild;
    m_lastChild = newChild;
    return newChild;
  }

  newChild->m_previousSibling = refChild;
  newChild->m_nextSibling = refChild->nextSibling();
  refChild->m_nextSibling->m_previousSibling = newChild;
  refChild->m_nextSibling = newChild;

  return newChild;
}

QDOM_NodePrivate* QDOM_NodePrivate::replaceChild( QDOM_NodePrivate* newChild, QDOM_NodePrivate* oldChild )
{
  // Error check
  if ( oldChild->parentNode() != this )
    return 0;

  // Error check
  if ( !newChild || !oldChild )
      return 0;

  // Error check
  if ( newChild == oldChild )
      return 0;

    // Special handling for inserting a fragment. We just insert
  // all elements of the fragment instead of the fragment itself.
  if ( newChild->isDocumentFragment() )
  {
      // Fragment is empty ?
      if ( newChild->m_firstChild == 0 )
	  return newChild;

      // New parent
      QDOM_NodePrivate* n = newChild->m_firstChild;
      while( n )
      {
	  n->m_parentNode = this;
	  n = n->m_nextSibling;
      }


      if ( oldChild->nextSibling() )
	  oldChild->nextSibling()->m_previousSibling = newChild->m_lastChild;
      if ( oldChild->previousSibling() )
	  oldChild->previousSibling()->m_nextSibling = newChild->m_firstChild;

      newChild->m_lastChild->m_nextSibling = oldChild->nextSibling();
      newChild->m_firstChild->m_previousSibling = oldChild->previousSibling();

      if ( m_firstChild == oldChild )
	  m_firstChild = newChild->m_firstChild;
      if ( m_lastChild == oldChild )
	  m_lastChild = newChild->m_lastChild;

      oldChild->m_parentNode = 0;
      oldChild->m_nextSibling = 0;
      oldChild->m_previousSibling = 0;

      // No need to increase the reference since QDomDocumentFragment
      // does not decrease the reference.

      // Remove the nodes from the fragment
      newChild->m_firstChild = 0;
      newChild->m_lastChild = 0;

      // We are no longer interested in the old node
      if ( oldChild ) oldChild->deref();

      return oldChild;
  }

  // No more errors can occure now, so we take
  // ownership of the node
  newChild->ref();

  // Release new node from its current parent
  if ( newChild->parentNode() )
    newChild->parentNode()->removeChild( newChild );

  newChild->m_parentNode = this;

  if ( oldChild->nextSibling() )
    oldChild->nextSibling()->m_previousSibling = newChild;
  if ( oldChild->previousSibling() )
    oldChild->previousSibling()->m_nextSibling = newChild;

  newChild->m_nextSibling = oldChild->nextSibling();
  newChild->m_previousSibling = oldChild->previousSibling();

  if ( m_firstChild == oldChild )
    m_firstChild = newChild;
  if ( m_lastChild == oldChild )
    m_lastChild = newChild;

  oldChild->m_parentNode = 0;
  oldChild->m_nextSibling = 0;
  oldChild->m_previousSibling = 0;

  // We are no longer interested in the old node
  if ( oldChild ) oldChild->deref();

  return oldChild;
}

QDOM_NodePrivate* QDOM_NodePrivate::removeChild( QDOM_NodePrivate* oldChild )
{
  // Error check
  if ( oldChild->parentNode() != this )
    return 0;

  // Perhaps oldChild was just created with "createElement" or that. In this case
  // its parent is QDomDocument but it is not part of the documents child list.
  if ( oldChild->m_nextSibling == 0 && oldChild->m_previousSibling == 0 && m_firstChild != oldChild )
      return 0;
  
  if ( oldChild->nextSibling() )
    oldChild->nextSibling()->m_previousSibling = oldChild->previousSibling();
  if ( oldChild->previousSibling() )
    oldChild->previousSibling()->m_nextSibling = oldChild->nextSibling();

  if ( m_lastChild == oldChild )
    m_lastChild = oldChild->previousSibling();
  if ( m_firstChild == oldChild )
    m_firstChild = oldChild->nextSibling();

  oldChild->m_parentNode = 0;
  oldChild->m_nextSibling = 0;
  oldChild->m_previousSibling = 0;

  // We are no longer interested in the old node
  if ( oldChild ) oldChild->deref();

  return oldChild;
}

QDOM_NodePrivate* QDOM_NodePrivate::appendChild( QDOM_NodePrivate* newChild )
{
  // No reference manipulation needed. Done in insertAfter.
  return insertAfter( newChild, 0 );
}

QDOM_NodePrivate* QDOM_NodePrivate::item( int index )
{
  QDOM_NodePrivate* p = m_firstChild;
  int i = 0;
  while( i < index && p )
  {
    p = p->nextSibling();
    ++i;
  }

  return p;
}

uint QDOM_NodePrivate::length() const
{
  uint i = 0;
  QDOM_NodePrivate* p = m_firstChild;
  while( p )
  {
    p = p->nextSibling();
    ++i;
  }

  return i;
}

void QDOM_NodePrivate::setParent( QDOM_NodePrivate* n )
{
    // Dont take over ownership of our parent :-)
    m_parentNode = n;
}

QDOM_DocumentPrivate* QDOM_NodePrivate::ownerDocument()
{
    QDOM_NodePrivate* p = this;
    while( p && p->isDocumentType() )
	p = p->m_parentNode;
    
    return (QDOM_DocumentPrivate*)p;
}

QDOM_NodePrivate* QDOM_NodePrivate::parentNode()
{
    return m_parentNode;
}

QDOM_NodeListPrivate* QDOM_NodePrivate::childNodes()
{
    return this;
}

QDOM_NodePrivate* QDOM_NodePrivate::firstChild()
{
    return m_firstChild;
}

QDOM_NodePrivate* QDOM_NodePrivate::lastChild()
{
    return m_lastChild;
}

QDOM_NodePrivate* QDOM_NodePrivate::previousSibling()
{
    return m_previousSibling;
}

QDOM_NodePrivate* QDOM_NodePrivate::nextSibling()
{
    return m_nextSibling;
}

bool QDOM_NodePrivate::hasChildNodes() const
{
    return ( m_lastChild != 0 );
}

QDOM_NodePrivate* QDOM_NodePrivate::cloneNode( bool deep )
{
    QDOM_NodePrivate* p = new QDOM_NodePrivate( this, deep );
    // We are not interested in this node
    p->deref();
    return p;
}

void QDOM_NodePrivate::save( QTextStream& s ) const
{
    const QDOM_NodePrivate* n = m_firstChild;
    while( n )
    {
	n->save( s );
	n = n->m_nextSibling;
    }
}

/**************************************************************
 *
 * QDomNode
 *
 **************************************************************/

#define IMPL ((QDOM_NodePrivate*)impl)

/*!
  \class QDomNodeList qdom.h
  \brief QDomNode is the base class of which the complete document tree is constructed

  \ingroup dom
*/

/*!
 This constructor creates a QDomNode which references no internal
 object. That means that isNull() will return TRUE.
*/
QDomNode::QDomNode()
{
  impl = 0;
}

/*!
 This copy constructor does not really create a copy of the QDomNode
 since QDomNode is explicit shared. Instead it creates a new
 reference to the internal object.
*/
QDomNode::QDomNode( const QDomNode& n )
{
  impl = n.impl;
  if ( impl ) impl->ref();
}

/*!
  Internal function.

  It creates a new reference on an internal node object.
*/
QDomNode::QDomNode( QDOM_NodePrivate* n )
{
  impl = n;
  if ( impl ) impl->ref();
}

/*!
  This assignment operator causes the QDomNode instance to reference
  another internal object. This does not really create a copy since QDomNode is
  explicit shared.
*/
QDomNode& QDomNode::operator= ( const QDomNode& n )
{
  if ( n.impl ) n.impl->ref();
  if ( impl && impl->deref() ) delete impl;
  impl = n.impl;

  return *this;
}

/*!
  Compares wether both QDomImplementation objects reference the same
  internal object.
*/
bool QDomNode::operator== ( const QDomNode& n ) const
{
  return ( impl == n.impl );
}

/*!
  Returns TRUE if both QDomImplementation objects reference different
  internal objects.
*/
bool QDomNode::operator!= ( const QDomNode& n ) const
{
  return ( impl != n.impl );
}

/*!
  The destructor does not by default destroy the referenced internal object.
  The internal representation is deleted when no QDomImplementation instance
  references it any more.
*/
QDomNode::~QDomNode()
{
  if ( impl && impl->deref() ) delete impl;
}

/*!
  Returns the name of the node. This name has different meanings
  depending on the subclass.

  <ul>
  <li> QDomElement: The tag name.
  <li> QDomAttr: The name of the attribute.
  <li> QDomText: The string "#text".
  <li> QDomCDATASection: The string "#cdata-section".
  <li> QDomEntityReference: Name of the referenced entity.
  <li> QDomEntity: Name of the entity.
  <li> QDomProcessingInstruction: The target of the processing instruction.
  <li> QDomDocument: The string "#document".
  <li> QDomComment: The string "#comment".
  <li> QDomDocumentType: The name of the document type.
  <li> QDomDocumentFragment: The string "#document-fragment".
  <li> QDomNotation: The name of the notation.
  </ul>

  \sa nodeValue()
*/
QString QDomNode::nodeName() const
{
  if ( !impl )
    return QString::null;
  return IMPL->m_name;
}

/*!
  Returns the nodes value. The meaning if the value depends on the subclass.

  <ul>
  <li> QDomAttr: The attributes value.
  <li> QDomText: The text.
  <li> QDomCDATASection: The content of the CDATA section.
  <li> QDomProcessingInstruction: The entire content of the tag except for the target.
  <li> QDomComment: The comment.
  </ul>

  All other subclasses not listed above do not have a node value. These classes
  will return a null string.

  \sa setNodeValue() nodeName()
*/
QString QDomNode::nodeValue() const
{
  if ( !impl )
    return QString::null;
  return IMPL->m_value;
}

/*!
  Sets the value of the node to \e v.

  \sa nodeValue()
*/
void QDomNode::setNodeValue( const QString& v )
{
  if ( !impl )
    return;
  IMPL->setNodeValue( v );
}

/*!
  Returns the type of the node. That is used to make safe casts
  to one of the subclasses.

  In many cases it will be more convenient to use one of the casting
  functions like toElement(), toText() and so on.
*/
QDomNode::NodeType QDomNode::nodeType() const
{
  return QDomNode::BaseNode;
}

/*!
  Returns the parent node, If this node has no parent, then
  a null node is returned.
*/
QDomNode QDomNode::parentNode() const
{
  if ( !impl )
    return QDomNode();
  return QDomNode( IMPL->parentNode() );
}

/*!
  Returns a list of all child nodes.

  In the most often used
  case you will call this function on a QDomElement object.
  If the nodes XML looks like this:

  \code
  <body>
   <h1>Heading</h1>
   <p>Hallo <b>you</b></p>
  </body>
  \endcode

  Then the list of child nodes will contain the node created by
  the <h1> tag and the node created by the <p> tag.

  The nodes in the list are not copies. They are just references. That means
  changing the nodes in the list affects the children of this node.

  \sa firstChild() lastChild()
*/
QDomNodeList QDomNode::childNodes() const
{
  if ( !impl )
    return QDomNodeList();
  return QDomNodeList( impl );
}

/*!
  Returns the first child of the node. If there is no
  child node a null node is returned.

  \sa lastChild() childNodes()
*/
QDomNode QDomNode::firstChild() const
{
  if ( !impl )
    return QDomNode();
  return QDomNode( IMPL->firstChild() );
}

/*!
  Returns the last child of the node. If there is no
  child node then a null node is returned.
*/
QDomNode QDomNode::lastChild() const
{
  if ( !impl )
    return QDomNode();
  return QDomNode( IMPL->lastChild() );
}

/*!
  Returns the previous sibling in the document tree.

  If you have XML like this:
  \code
  <h1>Heading</h1>
  <p>The text...</p>
  <h2>Next heading</h2>
  \endcode

  and this QDomNode represents the <p> tag, the previousSibling
  will return a reference to the node representing the <h1> tag.

  \sa nextSibling()
*/
QDomNode QDomNode::previousSibling() const
{
  if ( !impl )
    return QDomNode();
  return QDomNode( IMPL->previousSibling() );
}

/*!
  Returns the next sibling in the document tree.

  If you have XML like this:
  \code
  <h1>Heading</h1>
  <p>The text...</p>
  <h2>Next heading</h2>
  \endcode

  and this QDomNode represents the <p> tag, the nextSibling
  will return a reference to the node representing the <h2> tag.

  \sa previousSibling()
*/
QDomNode QDomNode::nextSibling() const
{
  if ( !impl )
    return QDomNode();
  return QDomNode( IMPL->nextSibling() );
}

/*!
  Returns a map of all attributes. Attributes are only
  provided for the subclass QDomElement.

  The returned attributes are no copies. That means
  altering the attributes in the map changes the
  attributes of this QDomNode.
*/
QDomNamedNodeMap QDomNode::attributes() const
{
  if ( !impl )
    return QDomNamedNodeMap();

  return QDomNamedNodeMap( impl->attributes() );
}

/*!
  Returns the document to which this node belongs.
*/
QDomDocument QDomNode::ownerDocument() const
{
  if ( !impl )
    return QDomDocument();
  return QDomDocument( IMPL->ownerDocument() );
}

/*!
  Creates a real copy of the QDomNode.

  If \e deep is true, then the cloning is done recursive.
  That means all children are copied, too. Otherwise the cloned
  node does not contain child nodes.
*/
QDomNode QDomNode::cloneNode( bool deep ) const
{
  if ( !impl )
    return QDomNode();
  return QDomNode( IMPL->cloneNode( deep ) );
}

/*!
  Inserts the node \e newChild before the child node \e refChild.
  \e refChild has to be a direct child of this node. If \e refChild
  is null then \e newChild is inserted as first child.

  If \e newChild is currently child of another parent, then it is reparented.
  If \e newChild is currently a child of this QDomNode, then its position in
  the list of children is changed.

  If \e newChild is a QDomDocumentFragment, then the children of the fragment
  are removed from the fragment and inserted after \e refChild.

  Returns a new reference to \e newChild on success or an empty node on failure.
*/
QDomNode QDomNode::insertBefore( const QDomNode& newChild, const QDomNode& refChild )
{
  if ( !impl )
    return QDomNode();
  return QDomNode( IMPL->insertBefore( newChild.impl, refChild.impl ) );
}

/*!
  Inserts the node \e newChild after the child node \e refChild.
  \e refChild has to be a direct child of this node. If \e refChild
  is null then \e newChild is appended as last child.

  If \e newChild is currently child of another parent, then it is reparented.
  If \e newChild is currently a child of this QDomNode, then its position in
  the list of children is changed.

  If \e newChild is a QDomDocumentFragment, then the children of the fragment
  are removed from the fragment and inserted after \e refChild.

  Returns a new reference to \e newChild on success or an empty node on failure.
*/
QDomNode QDomNode::insertAfter( const QDomNode& newChild, const QDomNode& refChild )
{
  if ( !impl )
    return QDomNode();
  return QDomNode( IMPL->insertAfter( newChild.impl, refChild.impl ) );
}

/*!
  Replaces \e oldChild with \e newChild.
  \e oldChild has to be a direct child of this node.

  If \e newChild is currently child of another parent, then it is reparented.
  If \e newChild is currently a child of this QDomNode, then its position in
  the list of children is changed.

  If \e newChild is a QDomDocumentFragment, then the children of the fragment
  are removed from the fragment and inserted after \e refChild.

  Returns a new reference to \e oldChild on success or a null node an failure.
*/
QDomNode QDomNode::replaceChild( const QDomNode& newChild, const QDomNode& oldChild )
{
  if ( !impl )
    return QDomNode();
  return QDomNode( IMPL->replaceChild( newChild.impl, oldChild.impl ) );
}

/*!
  Removes \e oldChild from the list of children.
  \e oldChild has to be a direct child of this node.

  Returns a new reference to \e oldChild on success or a null node on failure.
*/
QDomNode QDomNode::removeChild( const QDomNode& oldChild )
{
  if ( !impl )
    return QDomNode();
  return QDomNode( IMPL->removeChild( oldChild.impl ) );
}

/*!
  Appends \e newChild to the end of the children list.

  If \e newChild is currently child of another parent, then it is reparented.
  If \e newChild is currently a child of this QDomNode, then its position in
  the list of children is changed.

  Returns a new reference to \e newChild.
*/
QDomNode QDomNode::appendChild( const QDomNode& newChild )
{
  if ( !impl )
    return QDomNode();
  return QDomNode( IMPL->appendChild( newChild.impl ) );
}

/*!
  Returns TRUE if this node does not reference any internal object.
*/
bool QDomNode::isNull() const
{
  return ( impl == 0 );
}

/*!
  Dereferences the internal object.
  The node is then a null node.

  \sa isNull()
*/
void QDomNode::clear()
{
  if ( impl && impl->deref() ) delete impl;
  impl = 0;
}

/*!
  Returns the first child node which nodeName() method
  equals \e name.

  If no such direct child exists, a null node is returned.
*/
QDomNode QDomNode::namedItem( const QString& name ) const
{
  if ( !impl )
    return QDomNode();
  return QDomNode( impl->namedItem( name ) );
}

/*!
  Writes the XML representation of the node with all its children
  on the stream.
*/
void QDomNode::save( QTextStream& str ) const
{
  if ( impl )
    IMPL->save( str );
}

/*!
  Writes the XML representation of the node with all its children
  on the stream.
*/
QTextStream& operator<<( QTextStream& str, const QDomNode& node )
{
  node.save( str );

  return str;
}

#undef IMPL

/*==============================================================*/
/*                      NamedNodeMap                            */
/*==============================================================*/

/**************************************************************
 *
 * QDOM_NamedNodeMapPrivate
 *
 **************************************************************/

class QDOM_NamedNodeMapPrivate : public QShared
{
public:
  QDOM_NamedNodeMapPrivate( QDOM_NodePrivate* );
  ~QDOM_NamedNodeMapPrivate();

  QDOM_NodePrivate* namedItem( const QString& name ) const;
  QDOM_NodePrivate* setNamedItem( QDOM_NodePrivate* arg );
  QDOM_NodePrivate* removeNamedItem( const QString& name );
  QDOM_NodePrivate* item( int index ) const;
  uint length() const;
  bool contains( const QString& name ) const;

  /**
   * Remove all children from the map.
   */
  void clearMap();
  bool isReadOnly() { return m_readonly; }
  void setReadOnly( bool r ) { m_readonly = r; }
  bool isAppendToParent() { return m_appendToParent; }
  /**
   * If TRUE, then the node will redirect insert/remove calls
   * to its parent by calling QDOM_NodePrivate::appendChild or removeChild.
   * In addition the map wont increase or decrease the reference count
   * of the nodes it contains.
   *
   * By default this value is FALSE and the map will handle reference counting
   * by itself.
   */
  void setAppendToParent( bool b ) { m_appendToParent = b; }

  /**
   * Creates a copy of the map. It is a deep copy
   * that means that all children are cloned.
   */
  QDOM_NamedNodeMapPrivate* clone( QDOM_NodePrivate* parent );

  // Variables
  QDict<QDOM_NodePrivate> m_map;
  QDOM_NodePrivate* m_parent;
  bool m_readonly;
  bool m_appendToParent;
};

QDOM_NamedNodeMapPrivate::QDOM_NamedNodeMapPrivate( QDOM_NodePrivate* n )
{
    m_readonly = FALSE;
    m_parent = n;
    m_appendToParent = FALSE;
}

QDOM_NamedNodeMapPrivate::~QDOM_NamedNodeMapPrivate()
{
    clearMap();
}

QDOM_NamedNodeMapPrivate* QDOM_NamedNodeMapPrivate::clone( QDOM_NodePrivate* p )
{
    QDOM_NamedNodeMapPrivate* m = new QDOM_NamedNodeMapPrivate( p );
    m->m_readonly = m_readonly;
    m->m_appendToParent = m_appendToParent;

    QDictIterator<QDOM_NodePrivate> it ( m_map );
    for( ; it.current(); ++it )
	m->setNamedItem( it.current()->cloneNode() );

    // we are no longer interested in ownership
    m->deref();
    return m;
}

void QDOM_NamedNodeMapPrivate::clearMap()
{
    // Dereference all of our children if we took references
    if ( !m_appendToParent )
    {
	QDictIterator<QDOM_NodePrivate> it( m_map );
	for( ; it.current(); ++it )
	    if ( it.current()->deref() )
		delete it.current();
    }
    
    m_map.clear();
}

QDOM_NodePrivate* QDOM_NamedNodeMapPrivate::namedItem( const QString& name ) const
{
  QDOM_NodePrivate* p = m_map[ name ];
  return p;
}

QDOM_NodePrivate* QDOM_NamedNodeMapPrivate::setNamedItem( QDOM_NodePrivate* arg )
{
  if ( m_readonly || !arg )
    return 0;

  if ( m_appendToParent )
    return m_parent->appendChild( arg );

  // We take a reference
  arg->ref();
  m_map.insert( arg->nodeName(), arg );
  return arg;
}

QDOM_NodePrivate* QDOM_NamedNodeMapPrivate::removeNamedItem( const QString& name )
{
  if ( m_readonly )
    return 0;

  QDOM_NodePrivate* p = namedItem( name );
  if ( p == 0 )
    return 0;
  if ( m_appendToParent )
      return m_parent->removeChild( p );

  m_map.remove( p->nodeName() );
  // We took a reference, so we have to free one here
  p->deref();
  return p;
}

QDOM_NodePrivate* QDOM_NamedNodeMapPrivate::item( int index ) const
{
  if ( (uint)index >= length() )
    return 0;

  QDictIterator<QDOM_NodePrivate> it( m_map );
  for( int i = 0; i < index; ++i, ++it ) { }

  return it.current();
}

uint QDOM_NamedNodeMapPrivate::length() const
{
  return m_map.count();
}

bool QDOM_NamedNodeMapPrivate::contains( const QString& name ) const
{
  return ( m_map[ name ] != 0 );
}

/**************************************************************
 *
 * QDomNamedNodeMap
 *
 **************************************************************/

#define IMPL ((QDOM_NamedNodeMapPrivate*)impl)

/*!
  \class QDomNamedNodeMap qdom.h
  \brief QDomNamedNodeMap is a map for QDomNode objects.

  \ingroup dom

  The QDomNamedNodeMap is used in three places:

  <ul>
  <li> QDomDocumentType::entities() returns a map of all entities
       described in the DTD.
  <li> QDomDocumentType::notation() returns a map of all notations
       described in the DTD.
  <li> QDomElement::attributes() returns a map of all attributes.
  </ul>

  The map names the maps node as returned by QDomNode::nodeName()
  to the nodes.
*/

/*!
 This constructor creates a QDomNode which references no internal
 object. That means that isNull() will return TRUE.
*/
QDomNamedNodeMap::QDomNamedNodeMap()
{
  impl = 0;
}

/*!
 This copy constructor does not really create a copy of the QDomNamedNodeMap
 since QDomNamedNodeMap is explicit shared. Instead it creates a new
 reference to the internal object.
*/
QDomNamedNodeMap::QDomNamedNodeMap( const QDomNamedNodeMap& n )
{
  impl = n.impl;
  if ( impl ) impl->ref();
}

/*!
  Internal function
*/
QDomNamedNodeMap::QDomNamedNodeMap( QDOM_NamedNodeMapPrivate* n )
{
  impl = n;
  if ( impl ) impl->ref();
}

/*!
  This assignment operator causes the QDomNamedNodeMap instance to reference
  another internal object. This does not really create a copy since QDomNamedNodeMap is
  explicit shared.
*/
QDomNamedNodeMap& QDomNamedNodeMap::operator= ( const QDomNamedNodeMap& n )
{
  if ( impl && impl->deref() ) delete impl;
  impl = n.impl;
  if ( impl ) impl->ref();

  return *this;
}

/*!
  Compares wether both QDomNamedNodeMap objects reference the same
  internal object.
*/
bool QDomNamedNodeMap::operator== ( const QDomNamedNodeMap& n ) const
{
  return ( impl == n.impl );
}

/*!
  Returns TRUE if both QDomNamedNodeMap objects reference different
  internal objects.
*/
bool QDomNamedNodeMap::operator!= ( const QDomNamedNodeMap& n ) const
{
  return ( impl != n.impl );
}

/*!
  The destructor does not by default destroy the referenced internal object.
  The internal representation is deleted when no QDomNamedNodeMap instance
  references it any more.
*/
QDomNamedNodeMap::~QDomNamedNodeMap()
{
  if ( impl && impl->deref() ) delete impl;
}

/*!
  Returns the node associated with they key \e name.

  If the map does not contain such a node, then a null node is returned.

  \sa setNamedItem()
*/
QDomNode QDomNamedNodeMap::namedItem( const QString& name ) const
{
  if ( !impl )
    return QDomNode();
  return QDomNode( IMPL->namedItem( name ) );
}

/*!
  Inserts the node \newNode in the map. The name of \e newNode
  as returned by QDomNode::nodeName() is used as key for the map.

  Returns a reference to \e newNode.

  \sa removeNamedItem()
*/
QDomNode QDomNamedNodeMap::setNamedItem( const QDomNode& newNode )
{
  if ( !impl )
    return QDomNode();
  return QDomNode( IMPL->setNamedItem( (QDOM_NodePrivate*)newNode.impl ) );
}

/*!
  Remove the node named \name from the map.

  Returns a reference to the removed node of a null node
  if no node was removed.

  \sa setNamedItem()
*/
QDomNode QDomNamedNodeMap::removeNamedItem( const QString& name )
{
  if ( !impl )
    return QDomNode();
  return QDomNode( IMPL->removeNamedItem( name ) );
}

/*!
  Remove the node at position \e index.

  This is used to iterate over the map.

  \sa length()
*/
QDomNode QDomNamedNodeMap::item( int index ) const
{
  if ( !impl )
    return QDomNode();
  return QDomNode( IMPL->item( index ) );
}

/*!
  Returns the amount of nodes on the map.

  \sa item()
*/
uint QDomNamedNodeMap::length() const
{
  if ( !impl )
    return 0;
  return IMPL->length();
}

/*!
  Returns TRUE if the map contains a node for which QDomNode::nodeName()
  returns \e name.
*/
bool QDomNamedNodeMap::contains( const QString& name ) const
{
  if ( !impl )
    return FALSE;
  return IMPL->contains( name );
}

#undef IMPL

/*==============================================================*/
/*==============================================================*/

/**************************************************************
 *
 * QDOM_DocumentTypePrivate
 *
 **************************************************************/

class QDOM_DocumentTypePrivate : public QDOM_NodePrivate
{
public:
  QDOM_DocumentTypePrivate( QDOM_DocumentPrivate*, QDOM_NodePrivate* parent = 0 );
  QDOM_DocumentTypePrivate( QDOM_DocumentTypePrivate* n, bool deep );
  ~QDOM_DocumentTypePrivate();

  virtual QDOM_NamedNodeMapPrivate* entities();
  virtual QDOM_NamedNodeMapPrivate* notations();

  // Overloaded from QDOM_NodePrivate
  virtual QDOM_NodePrivate* cloneNode( bool deep = TRUE );
  virtual QDOM_NodePrivate* insertBefore( QDOM_NodePrivate* newChild, QDOM_NodePrivate* refChild );
  virtual QDOM_NodePrivate* insertAfter( QDOM_NodePrivate* newChild, QDOM_NodePrivate* refChild );
  virtual QDOM_NodePrivate* replaceChild( QDOM_NodePrivate* newChild, QDOM_NodePrivate* oldChild );
  virtual QDOM_NodePrivate* removeChild( QDOM_NodePrivate* oldChild );
  virtual QDOM_NodePrivate* appendChild( QDOM_NodePrivate* newChild );

  // Overloaded from QDOM_DocumentTypePrivate
  virtual bool isDocumentType() { return true; }
  virtual void save( QTextStream& s ) const;

  // Variables
  QDOM_NamedNodeMapPrivate* m_entities;
  QDOM_NamedNodeMapPrivate* m_notations;
};

QDOM_DocumentTypePrivate::QDOM_DocumentTypePrivate( QDOM_DocumentPrivate* doc, QDOM_NodePrivate* parent )
  : QDOM_NodePrivate( doc, parent )
{
  m_entities = new QDOM_NamedNodeMapPrivate( this );
  m_notations = new QDOM_NamedNodeMapPrivate( this );

  m_entities->setAppendToParent( TRUE );
  m_notations->setAppendToParent( TRUE );
}

QDOM_DocumentTypePrivate::QDOM_DocumentTypePrivate( QDOM_DocumentTypePrivate* n, bool deep )
  : QDOM_NodePrivate( n, deep )
{
  m_entities = new QDOM_NamedNodeMapPrivate( this );
  m_notations = new QDOM_NamedNodeMapPrivate( this );

  m_entities->setAppendToParent( TRUE );
  m_notations->setAppendToParent( TRUE );

  // Refill the maps with our new children
  QDOM_NodePrivate* p = firstChild();
  while( p )
  {
    if ( p->isEntity() )
      // Dont use normal insert method since we would create infinite recursion
      m_entities->m_map.insert( p->nodeName(), p );
    if ( p->isNotation() )
      // Dont use normal insert method since we would create infinite recursion
      m_notations->m_map.insert( p->nodeName(), p );
  }
}

QDOM_DocumentTypePrivate::~QDOM_DocumentTypePrivate()
{
    if ( m_entities->deref() ) delete m_entities;
    if ( m_notations->deref() ) delete m_notations;
}

QDOM_NamedNodeMapPrivate* QDOM_DocumentTypePrivate::entities()
{
    return m_entities;
}

QDOM_NamedNodeMapPrivate* QDOM_DocumentTypePrivate::notations()
{
    return m_notations;
}

QDOM_NodePrivate* QDOM_DocumentTypePrivate::cloneNode( bool deep)
{
    QDOM_NodePrivate* p = new QDOM_DocumentTypePrivate( this, deep );
    // We are not interested in this node
    p->deref();
    return p;
}

QDOM_NodePrivate* QDOM_DocumentTypePrivate::insertBefore( QDOM_NodePrivate* newChild, QDOM_NodePrivate* refChild )
{
  // Call the origianl implementation
  QDOM_NodePrivate* p = QDOM_NodePrivate::insertBefore( newChild, refChild );
  // Update the maps
  if ( p && p->isEntity() )
    m_entities->m_map.insert( p->nodeName(), p );
  else if ( p && p->isNotation() )
    m_notations->m_map.insert( p->nodeName(), p );

  return p;
}

QDOM_NodePrivate* QDOM_DocumentTypePrivate::insertAfter( QDOM_NodePrivate* newChild, QDOM_NodePrivate* refChild )
{
  // Call the origianl implementation
  QDOM_NodePrivate* p = QDOM_NodePrivate::insertAfter( newChild, refChild );
  // Update the maps
  if ( p && p->isEntity() )
    m_entities->m_map.insert( p->nodeName(), p );
  else if ( p && p->isNotation() )
    m_notations->m_map.insert( p->nodeName(), p );

  return p;
}

QDOM_NodePrivate* QDOM_DocumentTypePrivate::replaceChild( QDOM_NodePrivate* newChild, QDOM_NodePrivate* oldChild )
{
  // Call the origianl implementation
  QDOM_NodePrivate* p = QDOM_NodePrivate::replaceChild( newChild, oldChild );
  // Update the maps
  if ( p )
  {
    if ( oldChild && oldChild->isEntity() )
      m_entities->m_map.remove( oldChild->nodeName() );
    else if ( oldChild && oldChild->isNotation() )
      m_notations->m_map.remove( oldChild->nodeName() );

    if ( p->isEntity() )
      m_entities->m_map.insert( p->nodeName(), p );
    else if ( p->isNotation() )
      m_notations->m_map.insert( p->nodeName(), p );
  }

  return p;
}

QDOM_NodePrivate* QDOM_DocumentTypePrivate::removeChild( QDOM_NodePrivate* oldChild )
{
  // Call the origianl implementation
  QDOM_NodePrivate* p = QDOM_NodePrivate::removeChild(  oldChild );
  // Update the maps
  if ( p && p->isEntity() )
    m_entities->m_map.remove( p->nodeName() );
  else if ( p && p->isNotation() )
    m_notations->m_map.remove( p ->nodeName() );

  return p;
}

QDOM_NodePrivate* QDOM_DocumentTypePrivate::appendChild( QDOM_NodePrivate* newChild )
{
    return insertAfter( newChild, 0 );
    /* // Call the origianl implementation
  QDOM_NodePrivate* p = QDOM_NodePrivate::appendChild( newChild );
  // Update the maps
  if ( p && p->isEntity() )
    m_entities->m_map.insert( p->nodeName(), p );
  else if ( p && p->isNotation() )
    m_notations->m_map.insert( p->nodeName(), p );

    return p; */
}

void QDOM_DocumentTypePrivate::save( QTextStream& s ) const
{
  s << "<!DOCTYPE " << m_name << " ";

  // qDebug("--------- 3 DocType %i %i", m_entities->m_map.count(), m_notations->m_map.count() );

  if ( m_entities->length() > 0 || m_notations->length() > 0 )
  {
    s << "[ ";

    QDictIterator<QDOM_NodePrivate> it2( m_notations->m_map );
    for( ; it2.current(); ++it2 )
      it2.current()->save( s );

    QDictIterator<QDOM_NodePrivate> it( m_entities->m_map );
    for( ; it.current(); ++it )
      it.current()->save( s );

    s << " ]";
  }

  s << ">";
}

/**************************************************************
 *
 * QDomDocumentType
 *
 **************************************************************/

#define IMPL ((QDOM_DocumentTypePrivate*)impl)

/*!
  \class QDomDocumentType qdom.h
  \brief QDomDocumentType is the representation of the DTD in the document tree

  \ingroup dom

  Since QDom does currently only implement Level1 of the W3C's DOM specification
  QDomDocumentType allows readonly access to some of the DTDs data structures.

  Currently QDomDocumentType returns a map of all entities() and notation().

  In addition the method name() returns the document types
  name as given by the <!DOCTYPE name> tag.
*/

/*!
 This constructor creates a QDomDocumentType which references no internal
 object. That means that isNull() will return TRUE.
*/
QDomDocumentType::QDomDocumentType() : QDomNode()
{
}

/*!
 This copy constructor does not really create a copy of the QDomDocumentType
 since QDomDocumentType is explicit shared. Instead it creates a new
 reference to the internal object.
*/
QDomDocumentType::QDomDocumentType( const QDomDocumentType& n )
  : QDomNode( n )
{
}

/*!
  Internal function
*/
QDomDocumentType::QDomDocumentType( QDOM_DocumentTypePrivate* n )
  : QDomNode( n )
{
}

/*!
  This assignment operator causes the QDomDocumentType instance to reference
  another internal object. This does not really create a copy since QDomDocumentType is
  explicit shared.
*/
QDomDocumentType& QDomDocumentType::operator= ( const QDomDocumentType& n )
{
  return (QDomDocumentType&) QDomNode::operator=( n );
}

/*!
  The destructor does not by default destroy the referenced internal object.
  The internal representation is deleted when no QDomDocumentType instance
  references it any more.
*/
QDomDocumentType::~QDomDocumentType()
{
}

/*!
  Returns the document types name like specified by
  the <!DOCTYPE name> tag.

  The method nodeName() returns the same value.
*/
QString QDomDocumentType::name() const
{
  if ( !impl )
    return QString::null;

  return IMPL->nodeName();
}

/*!
  Returns a map of all entities described in the DTD.
*/
QDomNamedNodeMap QDomDocumentType::entities() const
{
  if ( !impl )
    return QDomNamedNodeMap();
  return QDomNamedNodeMap( IMPL->entities() );
}

/*!
  Returns a map of all notations described in the DTD.
*/
QDomNamedNodeMap QDomDocumentType::notations() const
{
  if ( !impl )
    return QDomNamedNodeMap();
  return QDomNamedNodeMap( IMPL->notations() );
}

/*!
  Returns QDomNode::DocumentTypeNode.

  \sa isDocumentType() QDomNode::toDocumentType()
*/
QDomNode::NodeType QDomDocumentType::nodeType() const
{
  return DocumentTypeNode;
}

/*!
  This method overloads QDomNode::isDocumentType().

  \sa nodeType() QDomNode::toDocumentType()
*/
bool QDomDocumentType::isDocumentType() const
{
  return TRUE;
}

#undef IMPL

/*==============================================================*/
/*                     DocumentFragment                         */
/*==============================================================*/

/**************************************************************
 *
 * QDOM_DocumentFragmentPrivate
 *
 **************************************************************/

class QDOM_DocumentFragmentPrivate : public QDOM_NodePrivate
{
public:
  QDOM_DocumentFragmentPrivate( QDOM_DocumentPrivate*, QDOM_NodePrivate* parent = 0 );
  QDOM_DocumentFragmentPrivate( QDOM_NodePrivate* n, bool deep );
  ~QDOM_DocumentFragmentPrivate();

  // Overloaded from QDOM_NodePrivate
  virtual QDOM_NodePrivate* cloneNode( bool deep = TRUE );
  virtual bool isDocumentFragment() { return TRUE; }

  static QString* s_dfName;
};

QString* QDOM_DocumentFragmentPrivate::s_dfName = 0;

QDOM_DocumentFragmentPrivate::QDOM_DocumentFragmentPrivate( QDOM_DocumentPrivate* doc, QDOM_NodePrivate* parent )
  : QDOM_NodePrivate( doc, parent )
{
  if ( !s_dfName )
    s_dfName = new QString( "#document-fragment" );
  m_name = *s_dfName;
}

QDOM_DocumentFragmentPrivate::QDOM_DocumentFragmentPrivate( QDOM_NodePrivate* n, bool deep )
  : QDOM_NodePrivate( n, deep )
{
}

QDOM_DocumentFragmentPrivate::~QDOM_DocumentFragmentPrivate()
{
}

QDOM_NodePrivate* QDOM_DocumentFragmentPrivate::cloneNode( bool deep)
{
    QDOM_NodePrivate* p = new QDOM_DocumentFragmentPrivate( this, deep );
    // We are not interested in this node
    p->deref();
    return p;
}

/**************************************************************
 *
 * QDomDocumentFragment
 *
 **************************************************************/

#define IMPL ((QDOM_DocumentFragmentPrivate*)impl)

/*!
  \class QDomDocumentFragment qdom.h
  \brief QDomDocumentFragment is used to hold a list of children during complex tree operations

  \ingroup dom

  In some situations you may want to group several nodes in a list and insert
  them all together as children of some node. In these cases QDomDocumentFragment can
  be used as a temporary container for this list of children.

  The most important feature of QDomDocumentFragment is, that it is treated in a special
  way by QDomNode::insertAfter(), QDomNode::insertBefore() and QDomNode::replaceChild().
  Instead of inserting the fragment itself, all children of the fragment are inserted.
*/

/*!
 This constructor creates a QDomDocumentFragment which references no internal
 object. That means that isNull() will return TRUE.
*/
QDomDocumentFragment::QDomDocumentFragment()
{
}

/*!
  Internal function
*/
QDomDocumentFragment::QDomDocumentFragment( QDOM_DocumentFragmentPrivate* n )
  : QDomNode( n )
{
}

/*!
 This copy constructor does not really create a copy of the QDomDocumentType
 since QDomDocumentType is explicit shared. Instead it creates a new
 reference to the internal object.
*/
QDomDocumentFragment::QDomDocumentFragment( const QDomDocumentFragment& x )
  : QDomNode( x )
{
}

/*!
  This assignment operator causes the QDomDocumentType instance to reference
  another internal object. This does not really create a copy since QDomDocumentType is
  explicit shared.
*/
QDomDocumentFragment& QDomDocumentFragment::operator= ( const QDomDocumentFragment& x )
{
  return (QDomDocumentFragment&) QDomNode::operator=( x );
}

/*!
  The destructor does not by default destroy the referenced internal object.
  The internal representation is deleted when no QDomDocumentType instance
  references it any more.
*/
QDomDocumentFragment::~QDomDocumentFragment()
{
}

/*!
  Returns QDomNode::DocumentFragment

  \sa isDocumentFragment() QDomNode::toDocumentFragment()
*/
QDomNode::NodeType QDomDocumentFragment::nodeType() const
{
  return QDomNode::DocumentFragmentNode;
}

/*!
  This method overloads QDomNode::isDocumentFragment().

  \sa nodeType() QDomNode::toDocumentFragment()
*/
bool QDomDocumentFragment::isDocumentFragment() const
{
  return TRUE;
}

#undef IMPL

/*==============================================================*/
/*                     CharacterData                            */
/*==============================================================*/

/**************************************************************
 *
 * QDOM_CharacterDataPrivate
 *
 **************************************************************/

class QDOM_CharacterDataPrivate : public QDOM_NodePrivate
{
public:
  QDOM_CharacterDataPrivate( QDOM_DocumentPrivate*, QDOM_NodePrivate* parent, const QString& data );
  QDOM_CharacterDataPrivate( QDOM_CharacterDataPrivate* n, bool deep );
  ~QDOM_CharacterDataPrivate();

  uint dataLength() const;
  QString substringData( unsigned long offset, unsigned long count ) const;
  void    appendData( const QString& arg );
  void    insertData( unsigned long offset, const QString& arg );
  void    deleteData( unsigned long offset, unsigned long count );
  void    replaceData( unsigned long offset, unsigned long count, const QString& arg );

  // Overloaded from QDOM_NodePrivate
  virtual bool isCharacterData() { return TRUE; }
  virtual QDOM_NodePrivate* cloneNode( bool deep = TRUE );

  static QString* s_cdName;
};

QString* QDOM_CharacterDataPrivate::s_cdName = 0;

QDOM_CharacterDataPrivate::QDOM_CharacterDataPrivate( QDOM_DocumentPrivate* d, QDOM_NodePrivate* p,
						      const QString& data )
  : QDOM_NodePrivate( d, p )
{
  m_value = data;

  if ( !s_cdName )
    s_cdName = new QString( "#character-data" );
  m_name = *s_cdName;
}

QDOM_CharacterDataPrivate::QDOM_CharacterDataPrivate( QDOM_CharacterDataPrivate* n, bool deep )
  : QDOM_NodePrivate( n, deep )
{
}

QDOM_CharacterDataPrivate::~QDOM_CharacterDataPrivate()
{
}

QDOM_NodePrivate* QDOM_CharacterDataPrivate::cloneNode( bool deep )
{
    QDOM_NodePrivate* p = new QDOM_CharacterDataPrivate( this, deep );
    // We are not interested in this node
    p->deref();
    return p;
}

uint QDOM_CharacterDataPrivate::dataLength() const
{
  return m_value.length();
}

QString QDOM_CharacterDataPrivate::substringData( unsigned long offset, unsigned long count ) const
{
  return m_value.mid( offset, count );
}

void QDOM_CharacterDataPrivate::insertData( unsigned long offset, const QString& arg )
{
  m_value.insert( offset, arg );
}

void QDOM_CharacterDataPrivate::deleteData( unsigned long offset, unsigned long count )
{
  m_value.remove( offset, count );
}

void QDOM_CharacterDataPrivate::replaceData( unsigned long offset, unsigned long count, const QString& arg )
{
  m_value.replace( offset, count, arg );
}

void QDOM_CharacterDataPrivate::appendData( const QString& arg )
{
  m_value += arg;
}

/**************************************************************
 *
 * QDomCharacterData
 *
 **************************************************************/

#define IMPL ((QDOM_CharacterDataPrivate*)impl)

QDomCharacterData::QDomCharacterData()
{
}

QDomCharacterData::QDomCharacterData( const QDomCharacterData& x )
  : QDomNode( x )
{
}

QDomCharacterData::QDomCharacterData( QDOM_CharacterDataPrivate* n )
  : QDomNode( n )
{
}

QDomCharacterData& QDomCharacterData::operator= ( const QDomCharacterData& x )
{
  return (QDomCharacterData&) QDomNode::operator=( x );
}

QDomCharacterData::~QDomCharacterData()
{
}

QString QDomCharacterData::data() const
{
  if ( !impl )
    return QString::null;
  return impl->nodeValue();
}

void QDomCharacterData::setData( const QString& v )
{
  if ( impl )
    impl->setNodeValue( v );
}

uint QDomCharacterData::length() const
{
  if ( impl )
    return IMPL->dataLength();
  return 0;
}

QString QDomCharacterData::substringData( unsigned long offset, unsigned long count )
{
  if ( !impl )
    return QString::null;
  return IMPL->substringData( offset, count );
}

void QDomCharacterData::appendData( const QString& arg )
{
  if ( impl )
    IMPL->appendData( arg );
}

void QDomCharacterData::insertData( unsigned long offset, const QString& arg )
{
  if ( impl )
    IMPL->insertData( offset, arg );
}

void QDomCharacterData::deleteData( unsigned long offset, unsigned long count )
{
  if ( impl )
    IMPL->deleteData( offset, count );
}

void QDomCharacterData::replaceData( unsigned long offset, unsigned long count, const QString& arg )
{
  if ( impl )
    IMPL->replaceData( offset, count, arg );
}

QDomNode::NodeType QDomCharacterData::nodeType() const
{
  return CharacterDataNode;
}

bool QDomCharacterData::isCharacterData() const
{
  return TRUE;
}

#undef IMPL

/**************************************************************
 *
 * QDOM_TextPrivate
 *
 **************************************************************/

class QDOM_TextPrivate : public QDOM_CharacterDataPrivate
{
public:
  QDOM_TextPrivate( QDOM_DocumentPrivate*, QDOM_NodePrivate* parent, const QString& value );
  QDOM_TextPrivate( QDOM_TextPrivate* n, bool deep );
  ~QDOM_TextPrivate();

  QDOM_TextPrivate* splitText( int offset );

  // Overloaded from QDOM_NodePrivate
  virtual QDOM_NodePrivate* cloneNode( bool deep = TRUE );
  virtual bool isText() { return TRUE; }
  virtual void save( QTextStream& s ) const;

  static QString* s_textName;
};

/*==============================================================*/
/*                        Attr                                  */
/*==============================================================*/

/**************************************************************
 *
 * QDOM_AttrPrivate
 *
 **************************************************************/

class QDOM_AttrPrivate : public QDOM_NodePrivate
{
public:
  QDOM_AttrPrivate( QDOM_DocumentPrivate*, QDOM_NodePrivate* parent, const QString& name );
  QDOM_AttrPrivate( QDOM_AttrPrivate* n, bool deep );
  ~QDOM_AttrPrivate();

  bool specified() const;

  // Overloaded from QDOM_NodePrivate
  virtual QDOM_NodePrivate* cloneNode( bool deep = TRUE );
  virtual bool isAttr() { return TRUE; }
  virtual void save( QTextStream& s ) const;

  // Variables
  bool m_specified;
};

QDOM_AttrPrivate::QDOM_AttrPrivate( QDOM_DocumentPrivate* d, QDOM_NodePrivate* parent, const QString& name )
  : QDOM_NodePrivate( d, parent )
{
    m_name = name;
    // qDebug("ATTR");
}

QDOM_AttrPrivate::QDOM_AttrPrivate( QDOM_AttrPrivate* n, bool deep )
  : QDOM_NodePrivate( n, deep )
{
    m_specified = n->specified();
    // qDebug("ATTR");
}

QDOM_AttrPrivate::~QDOM_AttrPrivate()
{
    // qDebug("~ATTR %s=%s", nodeName().latin1(), nodeValue().latin1() );
}

QDOM_NodePrivate* QDOM_AttrPrivate::cloneNode( bool deep )
{
    QDOM_NodePrivate* p = new QDOM_AttrPrivate( this, deep );
    // We are not interested in this node
    p->deref();
    return p;
}

bool QDOM_AttrPrivate::specified() const
{
  return m_specified;
}

/*
  Encode an attribute value upon saving.
*/
static QCString encodeAttr( const QCString& str )
{
    QCString tmp( str );
    uint len = tmp.length();
    uint i = 0;
    const char* d = tmp.data();
    while( i < len )
    {
	if ( d[i] == '<' )
        {
	    tmp.replace( i, 1, "&lt;" );
	    d = tmp.data();
	    len += 3;
	    i += 4;
	}
	else if ( d[i] == '"' )
        {
	    tmp.replace( i, 1, "&quot;" );
	    d = tmp.data();
	    len += 5;
	    i += 6;
	}
	else if ( d[i] == '&' )
        {
	    tmp.replace( i, 1, "&amp;" );
	    d = tmp.data();
	    len += 4;
	    i += 5;
	}
	else
	    ++i;
    }

    return tmp;
}

void QDOM_AttrPrivate::save( QTextStream& s ) const
{
  s << m_name << "=\"" << encodeAttr( m_value.utf8() ) << "\"";
}

/**************************************************************
 *
 * QDomAttr
 *
 **************************************************************/

#define IMPL ((QDOM_AttrPrivate*)impl)

QDomAttr::QDomAttr()
{
}

QDomAttr::QDomAttr( const QDomAttr& x )
  : QDomNode( x )
{
}

QDomAttr::QDomAttr( QDOM_AttrPrivate* n )
  : QDomNode( n )
{
}

QDomAttr& QDomAttr::operator= ( const QDomAttr& x )
{
  return (QDomAttr&) QDomNode::operator=( x );
}

QDomAttr::~QDomAttr()
{
}

QString QDomAttr::name() const
{
  if ( !impl )
    return QString::null;
  return impl->nodeName();
}

bool QDomAttr::specified() const
{
  if ( !impl )
    return false;
  return IMPL->specified();
}

QString QDomAttr::value() const
{
  if ( !impl )
    return QString::null;
  return impl->nodeValue();
}

void QDomAttr::setValue( const QString& v )
{
  if ( !impl )
    return;
  impl->setNodeValue( v );
}

QDomNode::NodeType QDomAttr::nodeType() const
{
  return AttributeNode;
}

bool QDomAttr::isAttr() const
{
  return TRUE;
}

#undef IMPL

/*==============================================================*/
/*                        Element                               */
/*==============================================================*/

/**************************************************************
 *
 * QDOM_ElementPrivate
 *
 **************************************************************/

static void qNormalizeElement( QDOM_NodePrivate* n )
{
  QDOM_NodePrivate* p = n->firstChild();
  QDOM_TextPrivate* t = 0;

  while( p )
  {
    if ( p->isText() )
    {
      if ( t )
      {
	QDOM_NodePrivate* tmp = p->nextSibling();
	t->appendData( p->nodeValue() );
	n->removeChild( p );
	p = tmp;
      }
      else
      {
	t = (QDOM_TextPrivate*)p;
	p = p->nextSibling();
      }
    }
    else
    {
      p = p->nextSibling();
      t = 0;
    }
  }
}

class QDOM_ElementPrivate : public QDOM_NodePrivate
{
public:
  QDOM_ElementPrivate( QDOM_DocumentPrivate*, QDOM_NodePrivate* parent, const QString& name );
  QDOM_ElementPrivate( QDOM_ElementPrivate* n, bool deep );
  ~QDOM_ElementPrivate();

  virtual QString   attribute( const QString& name ) const;
  virtual void      setAttribute( const QString& name, const QString& value );
  virtual void      removeAttribute( const QString& name );
  virtual QDOM_AttrPrivate* attributeNode( const QString& name);
  virtual QDOM_AttrPrivate* setAttributeNode( QDOM_AttrPrivate* newAttr );
  virtual QDOM_AttrPrivate* removeAttributeNode( QDOM_AttrPrivate* oldAttr );
  // TODO
  // virtual NodeList* elementsByTagName( const QString& name );
  virtual bool hasAttribute( const QString& name );
  virtual void normalize();

  QString text();

  // Overloaded from QDOM_NodePrivate
  virtual QDOM_NamedNodeMapPrivate* attributes() { return m_attr; }
  virtual bool isElement() { return TRUE; }
  virtual QDOM_NodePrivate* cloneNode( bool deep = TRUE );
  virtual void save( QTextStream& s ) const;

  // Variables
  QDOM_NamedNodeMapPrivate* m_attr;
};

QDOM_ElementPrivate::QDOM_ElementPrivate( QDOM_DocumentPrivate* d, QDOM_NodePrivate* p,
					  const QString& tagname )
  : QDOM_NodePrivate( d, p )
{
  m_name = tagname;
  m_attr = new QDOM_NamedNodeMapPrivate( this );
}

QDOM_ElementPrivate::QDOM_ElementPrivate( QDOM_ElementPrivate* n, bool deep ) :
  QDOM_NodePrivate( n, deep )
{
    m_attr = n->m_attr->clone( this );
    // Reference is down to 0, so we set it to 1 here.
    m_attr->ref();
}

QDOM_ElementPrivate::~QDOM_ElementPrivate()
{
    // qDebug("~Element=%s", nodeName().latin1() );
    if ( m_attr->deref() )
	delete m_attr;
}

QDOM_NodePrivate* QDOM_ElementPrivate::cloneNode( bool deep)
{
    QDOM_NodePrivate* p = new QDOM_ElementPrivate( this, deep );
    // We are not interested in this node
    p->deref();
    return p;
}

QString QDOM_ElementPrivate::attribute( const QString& name ) const
{
  QDOM_NodePrivate* n = m_attr->namedItem( name );
  if ( !n )
    return QString::null;

  return n->nodeValue();
}

void QDOM_ElementPrivate::setAttribute( const QString& name, const QString& value )
{
    removeAttribute( name );

    QDOM_NodePrivate* n = new QDOM_AttrPrivate( ownerDocument(), this, name );
    n->setNodeValue( value );

    // Referencing is done by the map, so we set the reference
    // counter back to 0 here. This is ok since we created the QDOM_AttrPrivate.
    n->deref();
    m_attr->setNamedItem( n );
}

void QDOM_ElementPrivate::removeAttribute( const QString& name )
{
    QDOM_NodePrivate* p = m_attr->removeNamedItem( name );
    if ( p && p->count == 0 )
	delete p;
}

QDOM_AttrPrivate* QDOM_ElementPrivate::attributeNode( const QString& name )
{
    return (QDOM_AttrPrivate*)m_attr->namedItem( name );
}

QDOM_AttrPrivate* QDOM_ElementPrivate::setAttributeNode( QDOM_AttrPrivate* newAttr )
{
    QDOM_NodePrivate* n = m_attr->namedItem( newAttr->nodeName() );
  
    // Referencing is done by the maps
    m_attr->setNamedItem( newAttr );

    return (QDOM_AttrPrivate*)n;
}

QDOM_AttrPrivate* QDOM_ElementPrivate::removeAttributeNode( QDOM_AttrPrivate* oldAttr )
{
  return (QDOM_AttrPrivate*)m_attr->removeNamedItem( oldAttr->nodeName() );
}

bool QDOM_ElementPrivate::hasAttribute( const QString& name )
{
  return m_attr->contains( name );
}

// TODO
/*
QDomNodeList* QDOM_ElementPrivate::elementsByTagName( const QString& name )
{
  NodeList* l = new NodeList( FALSE );

  qElementsByTagName( this, name, l );

  return l;
}
*/
/* const QDomNode* QDOM_ElementPrivate::childByTagName( const QString& name ) const
{
  const QDomNodeList* l = childNodes();
  uint len = l->length();

  for( uint i = 0; i < len; ++i )
    if ( l->item(i)->nodeName() == name )
      return l->item(i);

  return 0;
  } */


void QDOM_ElementPrivate::normalize()
{
  qNormalizeElement( this );
}

QString QDOM_ElementPrivate::text()
{
  QString t( "" );

  QDOM_NodePrivate* p = m_firstChild;
  while( p )
  {
    if ( p->isText() || p->isCDATASection() )
	t += p->nodeValue();
    else if ( p->isElement() )
	t += ((QDOM_ElementPrivate*)p)->text();
    p = p->nextSibling();
  }

  return t;
}

void QDOM_ElementPrivate::save( QTextStream& s ) const
{
  s << "<" << m_name;

  if ( !m_attr->m_map.isEmpty() )
  {
    s << " ";
    QDictIterator<QDOM_NodePrivate> it( m_attr->m_map );
    for( ; it.current(); ++it )
    {
      it.current()->save( s );
      s << " ";
    }
  }

  if ( hasChildNodes() )
  {
    s << ">" << endl;
    QDOM_NodePrivate::save( s );
    s << "</" << m_name << ">" << endl;
  }
  else
    s << "/>" << endl;
}

/**************************************************************
 *
 * QDomElement
 *
 **************************************************************/

#define IMPL ((QDOM_ElementPrivate*)impl)

QDomElement::QDomElement()
  : QDomNode()
{
}

QDomElement::QDomElement( const QDomElement& x )
  : QDomNode( x )
{
}

QDomElement::QDomElement( QDOM_ElementPrivate* n )
  : QDomNode( n )
{
}

QDomElement& QDomElement::operator= ( const QDomElement& x )
{
  return (QDomElement&) QDomNode::operator=( x );
}

QDomElement::~QDomElement()
{
}

QDomNode::NodeType QDomElement::nodeType() const
{
  return ElementNode;
}

void QDomElement::setTagName( const QString& name )
{
  if ( impl )
    impl->m_name = name;
}

QString QDomElement::tagName() const
{
  if ( !impl )
    return QString::null;
  return impl->nodeName();
}

QString QDomElement::attribute( const QString& name ) const
{
  if ( !impl )
    return QString::null;
  return IMPL->attribute( name );
}

void QDomElement::setAttribute( const QString& name, const QString& value )
{
  if ( !impl )
    return;
  IMPL->setAttribute( name, value );
}

void QDomElement::setAttribute( const QString& name, int value )
{
  if ( !impl )
    return;
  QString x;
  x.setNum( value );
  IMPL->setAttribute( name, x );
}

void QDomElement::setAttribute( const QString& name, uint value )
{
  if ( !impl )
    return;
  QString x;
  x.setNum( value );
  IMPL->setAttribute( name, x );
}

void QDomElement::setAttribute( const QString& name, double value )
{
  if ( !impl )
    return;
  QString x;
  x.setNum( value );
  IMPL->setAttribute( name, x );
}

void QDomElement::removeAttribute( const QString& name )
{
  if ( !impl )
    return;
  IMPL->removeAttribute( name );
}

QDomAttr QDomElement::attributeNode( const QString& name)
{
  if ( !impl )
    return QDomAttr();
  return QDomAttr( IMPL->attributeNode( name ) );
}

QDomAttr QDomElement::setAttributeNode( const QDomAttr& newAttr )
{
  if ( !impl )
    return QDomAttr();
  return QDomAttr( IMPL->setAttributeNode( ((QDOM_AttrPrivate*)newAttr.impl) ) );
}

QDomAttr QDomElement::removeAttributeNode( const QDomAttr& oldAttr )
{
  if ( !impl )
    return QDomAttr();
  return QDomAttr( IMPL->removeAttributeNode( ((QDOM_AttrPrivate*)oldAttr.impl) ) );
}

// TODO
//  NodeList QDomElement::elementsByTagName( const QString& name );

void QDomElement::normalize()
{
  if ( !impl )
    return;
  IMPL->normalize();
}

bool QDomElement::isElement() const
{
  return TRUE;
}

QDomNamedNodeMap QDomElement::attributes() const
{
  if ( !impl )
    return QDomNamedNodeMap();
  return QDomNamedNodeMap( IMPL->attributes() );
}

bool QDomElement::hasAttribute( const QString& name ) const
{
  if ( !impl )
    return FALSE;
  return IMPL->hasAttribute( name );
}

/*!
  Returns the text contained in children of the tag.
  If the tag has child tags, then the text() method is called
  for this children, too.

  Example:
  \code
  <h1>Hello <b>Qt</b> <![CDATA[<xml is cool>]]></h1>
  \endcode

  If this QDomElement represents the <h1> tag, then text()
  will return "Hello Qt <xml is cool>".

  Comments are ignored by this method. It evaluates only
  QDomText and QDomCDATASection and does a recursion on
  QDomElement.
*/
QString QDomElement::text() const
{
  if ( !impl )
    return QString::null;
  return IMPL->text();
}

QRect QDomElement::toRect() const
{
  if ( !impl )
    return QRect();
  return QRect( attribute( "x" ).toInt(), attribute( "y" ).toInt(),
		attribute( "width" ).toInt(), attribute( "height" ).toInt() );
}

QSize QDomElement::toSize() const
{
  if ( !impl )
    return QSize();
  return QSize( attribute( "width" ).toInt(), attribute( "height" ).toInt() );
}

QPoint QDomElement::toPoint() const
{
  if ( !impl )
    return QPoint();
  return QPoint( attribute( "x" ).toInt(), attribute( "y" ).toInt() );
}

QPen QDomElement::toPen() const
{
  if ( !impl )
    return QPen();

  bool ok;
  QPen p;
  p.setStyle( (Qt::PenStyle)attribute("style").toInt( &ok ) );
  if ( !ok ) return QPen();

  p.setWidth( attribute("width").toInt( &ok ) );
  if ( !ok ) return QPen();

  p.setColor( QColor( attribute("color") ) );

  return p;
}

QFont QDomElement::toFont() const
{
  if ( !impl )
    return QFont();

  QFont f;
  f.setFamily( attribute( "family" ) );

  bool ok;
  f.setPointSize( attribute("size").toInt( &ok ) );
  if ( !ok ) return QFont();

  f.setWeight( attribute("weight").toInt( &ok ) );
  if ( !ok ) return QFont();

  if ( hasAttribute( "italic" ) )
    f.setItalic( TRUE );

  if ( hasAttribute( "bold" ) )
    f.setBold( TRUE );

  return f;
}

#undef IMPL

/*==============================================================*/
/*                          Text                                */
/*==============================================================*/

/**************************************************************
 *
 * QDOM_TextPrivate
 *
 **************************************************************/

QString* QDOM_TextPrivate::s_textName = 0;

QDOM_TextPrivate::QDOM_TextPrivate( QDOM_DocumentPrivate* d, QDOM_NodePrivate* parent, const QString& value )
  : QDOM_CharacterDataPrivate( d, parent, value )
{
  if ( !s_textName )
    s_textName = new QString( "#text" );
  m_name = *s_textName;
}

QDOM_TextPrivate::QDOM_TextPrivate( QDOM_TextPrivate* n, bool deep )
  : QDOM_CharacterDataPrivate( n, deep )
{
}

QDOM_TextPrivate::~QDOM_TextPrivate()
{
}

QDOM_NodePrivate* QDOM_TextPrivate::cloneNode( bool deep)
{
    QDOM_NodePrivate* p = new QDOM_TextPrivate( this, deep );
    // We are not interested in this node
    p->deref();
    return p;
}

QDOM_TextPrivate* QDOM_TextPrivate::splitText( int offset )
{
  if ( !parentNode() )
  {
    qWarning( "QDomText::splitText  The node has no parent. So I can not split" );
    return 0;
  }

  QDOM_TextPrivate* t = new QDOM_TextPrivate( ownerDocument(), 0, m_value.mid( offset ) );
  m_value.truncate( offset );

  parentNode()->insertAfter( t, this );

  return t;
}

void QDOM_TextPrivate::save( QTextStream& s ) const
{
  s << encodeAttr( m_value.utf8() );
}

/**************************************************************
 *
 * QDomText
 *
 **************************************************************/

#define IMPL ((QDOM_TextPrivate*)impl)

QDomText::QDomText()
  : QDomCharacterData()
{
}

QDomText::QDomText( const QDomText& x )
  : QDomCharacterData( x )
{
}

QDomText::QDomText( QDOM_TextPrivate* n )
  : QDomCharacterData( n )
{
}

QDomText& QDomText::operator= ( const QDomText& x )
{
  return (QDomText&) QDomNode::operator=( x );
}

QDomText::~QDomText()
{
}

QDomNode::NodeType QDomText::nodeType() const
{
  return TextNode;
}

QDomText QDomText::splitText( int offset )
{
  if ( !impl )
    return QDomText();
  return QDomText( IMPL->splitText( offset ) );
}

bool QDomText::isText() const
{
  return TRUE;
}

#undef IMPL

/*==============================================================*/
/*                          Comment                             */
/*==============================================================*/

/**************************************************************
 *
 * QDOM_CommentPrivate
 *
 **************************************************************/

class QDOM_CommentPrivate : public QDOM_CharacterDataPrivate
{
public:
  QDOM_CommentPrivate( QDOM_DocumentPrivate*, QDOM_NodePrivate* parent, const QString& value );
  QDOM_CommentPrivate( QDOM_CommentPrivate* n, bool deep );
  ~QDOM_CommentPrivate();

  // Overloaded from QDOM_NodePrivate
  virtual QDOM_NodePrivate* cloneNode( bool deep = TRUE );
  virtual bool isComment() { return TRUE; }
  virtual void save( QTextStream& s ) const;

  static QString* s_commentName;
};

QString* QDOM_CommentPrivate::s_commentName = 0;

QDOM_CommentPrivate::QDOM_CommentPrivate( QDOM_DocumentPrivate* d, QDOM_NodePrivate* parent, const QString& value )
  : QDOM_CharacterDataPrivate( d, parent, value )
{
  if ( !s_commentName )
    s_commentName = new QString( "#comment" );
  m_name = *s_commentName;
}

QDOM_CommentPrivate::QDOM_CommentPrivate( QDOM_CommentPrivate* n, bool deep )
  : QDOM_CharacterDataPrivate( n, deep )
{
}

QDOM_CommentPrivate::~QDOM_CommentPrivate()
{
}

QDOM_NodePrivate* QDOM_CommentPrivate::cloneNode( bool deep)
{
    QDOM_NodePrivate* p = new QDOM_CommentPrivate( this, deep );
    // We are not interested in this node
    p->deref();
    return p;
}

void QDOM_CommentPrivate::save( QTextStream& s ) const
{
  s << "<!--" << m_value << "-->";
}

/**************************************************************
 *
 * QDomComment
 *
 **************************************************************/

#define IMPL ((QDOM_CommentPrivate*)impl)

QDomComment::QDomComment()
  : QDomCharacterData()
{
}

QDomComment::QDomComment( const QDomComment& x )
  : QDomCharacterData( x )
{
}

QDomComment::QDomComment( QDOM_CommentPrivate* n )
  : QDomCharacterData( n )
{
}

QDomComment& QDomComment::operator= ( const QDomComment& x )
{
  return (QDomComment&) QDomNode::operator=( x );
}

QDomComment::~QDomComment()
{
}

QDomNode::NodeType QDomComment::nodeType() const
{
  return CommentNode;
}

bool QDomComment::isComment() const
{
  return TRUE;
}

#undef IMPL

/*==============================================================*/
/*                        CDATASection                          */
/*==============================================================*/

/**************************************************************
 *
 * QDOM_CDATASectionPrivate
 *
 **************************************************************/

class QDOM_CDATASectionPrivate : public QDOM_TextPrivate
{
public:
  QDOM_CDATASectionPrivate( QDOM_DocumentPrivate*, QDOM_NodePrivate* parent, const QString& value );
  QDOM_CDATASectionPrivate( QDOM_CDATASectionPrivate* n, bool deep );
  ~QDOM_CDATASectionPrivate();

  // Overloaded from QDOM_NodePrivate
  virtual QDOM_NodePrivate* cloneNode( bool deep = TRUE );
  virtual bool isCDATASection() { return TRUE; }
  virtual void save( QTextStream& s ) const;

  static QString* s_cdataName;
};

QString* QDOM_CDATASectionPrivate::s_cdataName = 0;

QDOM_CDATASectionPrivate::QDOM_CDATASectionPrivate( QDOM_DocumentPrivate* d, QDOM_NodePrivate* parent,
						    const QString& value )
  : QDOM_TextPrivate( d, parent, value )
{
  if ( !s_cdataName )
    s_cdataName = new QString( "#cdata-section" );
  m_name = *s_cdataName;
}

QDOM_CDATASectionPrivate::QDOM_CDATASectionPrivate( QDOM_CDATASectionPrivate* n, bool deep )
  : QDOM_TextPrivate( n, deep )
{
}

QDOM_CDATASectionPrivate::~QDOM_CDATASectionPrivate()
{
}

QDOM_NodePrivate* QDOM_CDATASectionPrivate::cloneNode( bool deep)
{
    QDOM_NodePrivate* p = new QDOM_CDATASectionPrivate( this, deep );
    // We are not interested in this node
    p->deref();
    return p;
}

void QDOM_CDATASectionPrivate::save( QTextStream& s ) const
{
    // #### How do we escape "]]>" ?
    s << "<![CDATA[" << m_value << "]]>";
}

/**************************************************************
 *
 * QDomCDATASection
 *
 **************************************************************/

#define IMPL ((QDOM_CDATASectionPrivate*)impl)

QDomCDATASection::QDomCDATASection()
  : QDomText()
{
}

QDomCDATASection::QDomCDATASection( const QDomCDATASection& x )
  : QDomText( x )
{
}

QDomCDATASection::QDomCDATASection( QDOM_CDATASectionPrivate* n )
  : QDomText( n )
{
}

QDomCDATASection& QDomCDATASection::operator= ( const QDomCDATASection& x )
{
  return (QDomCDATASection&) QDomNode::operator=( x );
}

QDomCDATASection::~QDomCDATASection()
{
}

QDomNode::NodeType QDomCDATASection::nodeType() const
{
  return CDATASectionNode;
}

bool QDomCDATASection::isCDATASection() const
{
  return TRUE;
}

#undef IMPL

/*==============================================================*/
/*                          Notation                            */
/*==============================================================*/

/**************************************************************
 *
 * QDOM_NotationPrivate
 *
 **************************************************************/

class QDOM_NotationPrivate : public QDOM_NodePrivate
{
public:
  QDOM_NotationPrivate( QDOM_DocumentPrivate*, QDOM_NodePrivate* parent, const QString& name,
			const QString& pub, const QString& sys );
  QDOM_NotationPrivate( QDOM_NotationPrivate* n, bool deep );
  ~QDOM_NotationPrivate();

  // Overloaded from QDOM_NodePrivate
  virtual QDOM_NodePrivate* cloneNode( bool deep = TRUE );
  virtual bool isNotation() { return TRUE; }
  virtual void save( QTextStream& s ) const;

  // Variables
  QString m_sys;
  QString m_pub;
};

QDOM_NotationPrivate::QDOM_NotationPrivate( QDOM_DocumentPrivate* d, QDOM_NodePrivate* parent, const QString& name,
					    const QString& pub, const QString& sys )
  : QDOM_NodePrivate( d, parent )
{
  m_name = name;
  m_pub = pub;
  m_sys = sys;
}

QDOM_NotationPrivate::QDOM_NotationPrivate( QDOM_NotationPrivate* n, bool deep )
  : QDOM_NodePrivate( n, deep )
{
  m_sys = n->m_sys;
  m_pub = n->m_pub;
}

QDOM_NotationPrivate::~QDOM_NotationPrivate()
{
}

QDOM_NodePrivate* QDOM_NotationPrivate::cloneNode( bool deep)
{
    QDOM_NodePrivate* p = new QDOM_NotationPrivate( this, deep );
    // We are not interested in this node
    p->deref();
    return p;
}

void QDOM_NotationPrivate::save( QTextStream& s ) const
{
  s << "<!NOTATION " << m_name << " ";
  if ( !m_pub.isEmpty() )
  {
    s << "PUBLIC " << m_pub;
    if ( !m_sys.isEmpty() )
      s << " " << m_sys;
  }
  else
    s << "SYSTEM " << m_sys;

  s << ">";
}

/**************************************************************
 *
 * QDomNotation
 *
 **************************************************************/

#define IMPL ((QDOM_NotationPrivate*)impl)

QDomNotation::QDomNotation()
  : QDomNode()
{
}

QDomNotation::QDomNotation( const QDomNotation& x )
  : QDomNode( x )
{
}

QDomNotation::QDomNotation( QDOM_NotationPrivate* n )
  : QDomNode( n )
{
}

QDomNotation& QDomNotation::operator= ( const QDomNotation& x )
{
  return (QDomNotation&) QDomNode::operator=( x );
}

QDomNotation::~QDomNotation()
{
}

QDomNode::NodeType QDomNotation::nodeType() const
{
  return NotationNode;
}

QString QDomNotation::publicId() const
{
  if ( !impl )
    return QString::null;
  return IMPL->m_pub;
}

QString QDomNotation::systemId() const
{
  if ( !impl )
    return QString::null;
  return IMPL->m_sys;
}

bool QDomNotation::isNotation() const
{
  return TRUE;
}

#undef IMPL


/*==============================================================*/
/*                          Entity                            */
/*==============================================================*/

/**************************************************************
 *
 * QDOM_EntityPrivate
 *
 **************************************************************/

class QDOM_EntityPrivate : public QDOM_NodePrivate
{
public:
  QDOM_EntityPrivate( QDOM_DocumentPrivate*, QDOM_NodePrivate* parent, const QString& name,
		      const QString& pub, const QString& sys, const QString& notation );
  QDOM_EntityPrivate( QDOM_EntityPrivate* n, bool deep );
  ~QDOM_EntityPrivate();

  // Overloaded from QDOM_NodePrivate
  virtual QDOM_NodePrivate* cloneNode( bool deep = TRUE );
  virtual bool isEntity() { return TRUE; }
  virtual void save( QTextStream& s ) const;

  // Variables
  QString m_sys;
  QString m_pub;
  QString m_notationName;
};

QDOM_EntityPrivate::QDOM_EntityPrivate( QDOM_DocumentPrivate* d, QDOM_NodePrivate* parent, const QString& name,
					const QString& pub, const QString& sys, const QString& notation )
  : QDOM_NodePrivate( d, parent )
{
  m_name = name;
  m_pub = pub;
  m_sys = sys;
  m_notationName = notation;
}

QDOM_EntityPrivate::QDOM_EntityPrivate( QDOM_EntityPrivate* n, bool deep )
  : QDOM_NodePrivate( n, deep )
{
  m_sys = n->m_sys;
  m_pub = n->m_pub;
  m_notationName = n->m_notationName;
}

QDOM_EntityPrivate::~QDOM_EntityPrivate()
{
}

QDOM_NodePrivate* QDOM_EntityPrivate::cloneNode( bool deep)
{
    QDOM_NodePrivate* p = new QDOM_EntityPrivate( this, deep );
    // We are not interested in this node
    p->deref();
    return p;
}

/*
  Encode an entity value upon saving.
*/
static QCString encodeEntity( const QCString& str )
{
    QCString tmp( str );
    uint len = tmp.length();
    uint i = 0;
    const char* d = tmp.data();
    while( i < len )
    {
	if ( d[i] == '%' )
        {
	    tmp.replace( i, 1, "&#60;" );
	    d = tmp.data();
	    len += 4;
	    i += 5;
	}
	else if ( d[i] == '"' )
        {
	    tmp.replace( i, 1, "&#34;" );
	    d = tmp.data();
	    len += 4;
	    i += 5;
	}
	// Dont encode &lt; or &quot; or &custom;.
	// Only encode character references
	else if ( d[i] == '&' && i + 1 < len && d[i+1] == '#' )
        {
	    tmp.replace( i, 1, "&#38;" );
	    d = tmp.data();
	    len += 4;
	    i += 5;
	}
	else
	    ++i;
    }

    return tmp;
}

void QDOM_EntityPrivate::save( QTextStream& s ) const
{
  if ( m_sys.isEmpty() && m_pub.isEmpty() )
  {
    s << "<!ENTITY " << m_name << " \"" << encodeEntity( m_value.utf8() ) << "\">";
  }
  else
  {
    s << "<!ENTITY " << m_name << " ";
    if ( m_pub.isEmpty() )
      s << "SYSTEM " << m_sys;
    else
      s << "PUBLIC " << m_pub << " " << m_sys;
    s << ">";
  }
}

/**************************************************************
 *
 * QDomEntity
 *
 **************************************************************/

#define IMPL ((QDOM_EntityPrivate*)impl)

QDomEntity::QDomEntity()
  : QDomNode()
{
}

QDomEntity::QDomEntity( const QDomEntity& x )
  : QDomNode( x )
{
}

QDomEntity::QDomEntity( QDOM_EntityPrivate* n )
  : QDomNode( n )
{
}

QDomEntity& QDomEntity::operator= ( const QDomEntity& x )
{
  return (QDomEntity&) QDomNode::operator=( x );
}

QDomEntity::~QDomEntity()
{
}

QDomNode::NodeType QDomEntity::nodeType() const
{
  return EntityNode;
}

QString QDomEntity::publicId() const
{
  if ( !impl )
    return QString::null;
  return IMPL->m_pub;
}

QString QDomEntity::systemId() const
{
  if ( !impl )
    return QString::null;
  return IMPL->m_sys;
}

QString QDomEntity::notationName() const
{
  if ( !impl )
    return QString::null;
  return IMPL->m_notationName;
}

bool QDomEntity::isEntity() const
{
  return TRUE;
}

#undef IMPL


/*==============================================================*/
/*                      EntityReference                         */
/*==============================================================*/

/**************************************************************
 *
 * QDOM_EntityReferencePrivate
 *
 **************************************************************/

class QDOM_EntityReferencePrivate : public QDOM_NodePrivate
{
public:
  QDOM_EntityReferencePrivate( QDOM_DocumentPrivate*, QDOM_NodePrivate* parent, const QString& name );
  QDOM_EntityReferencePrivate( QDOM_NodePrivate* n, bool deep );
  ~QDOM_EntityReferencePrivate();

  // Overloaded from QDOM_NodePrivate
  virtual QDOM_NodePrivate* cloneNode( bool deep = TRUE );
  virtual bool isEntityReference() { return TRUE; }
  virtual void save( QTextStream& s ) const;
};

QDOM_EntityReferencePrivate::QDOM_EntityReferencePrivate( QDOM_DocumentPrivate* d, QDOM_NodePrivate* parent, const QString& name )
  : QDOM_NodePrivate( d, parent )
{
  m_name = name;
}

QDOM_EntityReferencePrivate::QDOM_EntityReferencePrivate( QDOM_NodePrivate* n, bool deep )
  : QDOM_NodePrivate( n, deep )
{
}

QDOM_EntityReferencePrivate::~QDOM_EntityReferencePrivate()
{
}

QDOM_NodePrivate* QDOM_EntityReferencePrivate::cloneNode( bool deep)
{
    QDOM_NodePrivate* p = new QDOM_EntityReferencePrivate( this, deep );
    // We are not interested in this node
    p->deref();
    return p;
}

void QDOM_EntityReferencePrivate::save( QTextStream& s ) const
{
  s << "&" << m_name << ";";
}

/**************************************************************
 *
 * QDomEntityReference
 *
 **************************************************************/

#define IMPL ((QDOM_EntityReferencePrivate*)impl)

QDomEntityReference::QDomEntityReference()
  : QDomNode()
{
}

QDomEntityReference::QDomEntityReference( const QDomEntityReference& x )
  : QDomNode( x )
{
}

QDomEntityReference::QDomEntityReference( QDOM_EntityReferencePrivate* n )
  : QDomNode( n )
{
}

QDomEntityReference& QDomEntityReference::operator= ( const QDomEntityReference& x )
{
  return (QDomEntityReference&) QDomNode::operator=( x );
}

QDomEntityReference::~QDomEntityReference()
{
}

QDomNode::NodeType QDomEntityReference::nodeType() const
{
  return EntityReferenceNode;
}

bool QDomEntityReference::isEntityReference() const
{
  return TRUE;
}

#undef IMPL


/*==============================================================*/
/*                      ProcessingInstruction                   */
/*==============================================================*/

/**************************************************************
 *
 * QDOM_ProcessingInstructionPrivate
 *
 **************************************************************/

class QDOM_ProcessingInstructionPrivate : public QDOM_NodePrivate
{
public:
  QDOM_ProcessingInstructionPrivate( QDOM_DocumentPrivate*, QDOM_NodePrivate* parent, const QString& target,
				     const QString& data);
  QDOM_ProcessingInstructionPrivate( QDOM_ProcessingInstructionPrivate* n, bool deep );
  ~QDOM_ProcessingInstructionPrivate();

  // Overloaded from QDOM_NodePrivate
  virtual QDOM_NodePrivate* cloneNode( bool deep = TRUE );
  virtual bool isProcessingInstruction() { return TRUE; }
  virtual void save( QTextStream& s ) const;
};

QDOM_ProcessingInstructionPrivate::QDOM_ProcessingInstructionPrivate( QDOM_DocumentPrivate* d,
								      QDOM_NodePrivate* parent,
								      const QString& target,
								      const QString& data )
  : QDOM_NodePrivate( d, parent )
{
  m_name = target;
  m_value = data;
}

QDOM_ProcessingInstructionPrivate::QDOM_ProcessingInstructionPrivate( QDOM_ProcessingInstructionPrivate* n, bool deep )
  : QDOM_NodePrivate( n, deep )
{
}

QDOM_ProcessingInstructionPrivate::~QDOM_ProcessingInstructionPrivate()
{
}

QDOM_NodePrivate* QDOM_ProcessingInstructionPrivate::cloneNode( bool deep)
{
    QDOM_NodePrivate* p = new QDOM_ProcessingInstructionPrivate( this, deep );
    // We are not interested in this node
    p->deref();
    return p;
}

void QDOM_ProcessingInstructionPrivate::save( QTextStream& s ) const
{
  s << "<?" << m_name << " " << m_value << "?>";
}

/**************************************************************
 *
 * QDomProcessingInstruction
 *
 **************************************************************/

#define IMPL ((QDOM_ProcessingInstructionPrivate*)impl)

QDomProcessingInstruction::QDomProcessingInstruction()
  : QDomNode()
{
}

QDomProcessingInstruction::QDomProcessingInstruction( const QDomProcessingInstruction& x )
  : QDomNode( x )
{
}

QDomProcessingInstruction::QDomProcessingInstruction( QDOM_ProcessingInstructionPrivate* n )
  : QDomNode( n )
{
}

QDomProcessingInstruction& QDomProcessingInstruction::operator= ( const QDomProcessingInstruction& x )
{
  return (QDomProcessingInstruction&) QDomNode::operator=( x );
}

QDomProcessingInstruction::~QDomProcessingInstruction()
{
}

QDomNode::NodeType QDomProcessingInstruction::nodeType() const
{
  return ProcessingInstructionNode;
}

QString QDomProcessingInstruction::target() const
{
  if ( !impl )
    return QString::null;
  return impl->nodeName();
}

QString QDomProcessingInstruction::data() const
{
  if ( !impl )
    return QString::null;
  return impl->nodeValue();
}

void QDomProcessingInstruction::setData( const QString& d )
{
  if ( !impl )
    return;
  impl->setNodeValue( d );
}

bool QDomProcessingInstruction::isProcessingInstruction() const
{
  return TRUE;
}

#undef IMPL

/*==============================================================*/
/*                 Document                                     */
/*==============================================================*/

/**************************************************************
 *
 * QDOM_DocumentPrivate
 *
 **************************************************************/

class QDOM_DocumentPrivate : public QDOM_NodePrivate
{
public:
  QDOM_DocumentPrivate();
  QDOM_DocumentPrivate( const QString& name );
  QDOM_DocumentPrivate( QDOM_DocumentPrivate* n, bool deep );
  ~QDOM_DocumentPrivate();

  bool setContent( const QString& text );
  QDomMimeSourceFactory* mimeSourceFactory();
  void setMimeSourceFactory( QDomMimeSourceFactory* );

  // Attributes
  QDOM_DocumentTypePrivate* doctype() { return type; };
  QDOM_ImplementationPrivate* implementation() { return impl; };
  QDOM_ElementPrivate* documentElement();

  // Factories
  QDOM_ElementPrivate*               createElement( const QString& tagName );
  QDOM_DocumentFragmentPrivate*      createDocumentFragment();
  QDOM_TextPrivate*                  createTextNode( const QString& data );
  QDOM_CommentPrivate*               createComment( const QString& data );
  QDOM_CDATASectionPrivate*          createCDATASection( const QString& data );
  QDOM_ProcessingInstructionPrivate* createProcessingInstruction( const QString& target, const QString& data );
  QDOM_AttrPrivate*                  createAttribute( const QString& name );
  QDOM_EntityReferencePrivate*       createEntityReference( const QString& name );
  QDOM_NodeListPrivate*              elementsByTagName( const QString& tagname );

  // Overloaded from QDOM_NodePrivate
  virtual QDOM_NodePrivate* cloneNode( bool deep = TRUE );
  virtual bool isDocument() { return TRUE; }
  virtual void clear();
  virtual void save( QTextStream& ) const;

  // Variables
  QDOM_ImplementationPrivate* impl;
  QDOM_DocumentTypePrivate* type;
  QDomMimeSourceFactory* m_mimeSourceFactory;

  static QString* s_docName;
};

QString* QDOM_DocumentPrivate::s_docName = 0;

QDOM_DocumentPrivate::QDOM_DocumentPrivate()
  : QDOM_NodePrivate( 0 )
{
  m_mimeSourceFactory = 0;
  impl = new QDOM_ImplementationPrivate();
  type = new QDOM_DocumentTypePrivate( this, this );

  if ( !s_docName )
    s_docName = new QString( "#document" );
  m_name = *s_docName;
}

QDOM_DocumentPrivate::QDOM_DocumentPrivate( const QString& name )
  : QDOM_NodePrivate( 0 )
{
  m_mimeSourceFactory = 0;
  impl = new QDOM_ImplementationPrivate();
  type = new QDOM_DocumentTypePrivate( this, this );
  type->m_name = name;

  if ( !s_docName )
    s_docName = new QString( "#document" );
  m_name = *s_docName;
}

QDOM_DocumentPrivate::QDOM_DocumentPrivate( QDOM_DocumentPrivate* n, bool deep )
  : QDOM_NodePrivate( n, deep )
{
    m_mimeSourceFactory = n->m_mimeSourceFactory;
    impl = n->impl->clone();
    // Reference count is down to 0, so we set it to 1 here.
    impl->ref();
    type = (QDOM_DocumentTypePrivate*)n->type->cloneNode();
    type->setParent( this );
    // Reference count is down to 0, so we set it to 1 here.
    type->ref();
}

QDOM_DocumentPrivate::~QDOM_DocumentPrivate()
{
    // qDebug("~Document %x", this);
    if ( impl->deref() ) delete impl;
    if ( type->deref() ) delete type;
}

void QDOM_DocumentPrivate::clear()
{
    if ( impl->deref() ) delete impl;
    if ( type->deref() ) delete type;
    impl = 0;
    type = 0;
    QDOM_NodePrivate::clear();
}

QDomMimeSourceFactory* QDOM_DocumentPrivate::mimeSourceFactory()
{
  if ( m_mimeSourceFactory )
    return m_mimeSourceFactory;

  return QDomMimeSourceFactory::defaultDomFactory();
}

void QDOM_DocumentPrivate::setMimeSourceFactory( QDomMimeSourceFactory* f )
{
  m_mimeSourceFactory = f;
}

bool QDOM_DocumentPrivate::setContent( const QString& text )
{
  clear();
  impl = new QDOM_ImplementationPrivate;
  type = new QDOM_DocumentTypePrivate( this, this );

  QXMLSimpleParser p;
  QDomConsumer c( this );
  int pos = p.parse( text, &c );
  if ( pos != -1 )
  {
    qWarning("Parsing error at %i\n",pos);
    return FALSE;
  }
  return TRUE;
}

QDOM_NodePrivate* QDOM_DocumentPrivate::cloneNode( bool deep)
{
    QDOM_NodePrivate* p = new QDOM_DocumentPrivate( this, deep );
    // We are not interested in this node
    p->deref();
    return p;
}

QDOM_ElementPrivate* QDOM_DocumentPrivate::documentElement()
{
  QDOM_NodePrivate* p = m_firstChild;
  while( p && !p->isElement() )
    p = p->nextSibling();

  return (QDOM_ElementPrivate*)p;
}

QDOM_ElementPrivate* QDOM_DocumentPrivate::createElement( const QString& tagName )
{
    QDOM_ElementPrivate* e = new QDOM_ElementPrivate( this, this, tagName );
    e->deref();
    return e;
}

QDOM_DocumentFragmentPrivate* QDOM_DocumentPrivate::createDocumentFragment()
{
    QDOM_DocumentFragmentPrivate* f = new QDOM_DocumentFragmentPrivate( this, this );
    f->deref();
    return f;
}

QDOM_TextPrivate* QDOM_DocumentPrivate::createTextNode( const QString& data )
{
    QDOM_TextPrivate* t = new QDOM_TextPrivate( this, this, data );
    t->deref();
    return t;
}

QDOM_CommentPrivate* QDOM_DocumentPrivate::createComment( const QString& data )
{
    QDOM_CommentPrivate* c = new QDOM_CommentPrivate( this, this, data );
    c->deref();
    return c;
}

QDOM_CDATASectionPrivate* QDOM_DocumentPrivate::createCDATASection( const QString& data )
{
    QDOM_CDATASectionPrivate* c = new QDOM_CDATASectionPrivate( this, this, data );
    c->deref();
    return c;
}

QDOM_ProcessingInstructionPrivate* QDOM_DocumentPrivate::createProcessingInstruction( const QString& target, const QString& data )
{
    QDOM_ProcessingInstructionPrivate* p = new QDOM_ProcessingInstructionPrivate( this, this, target, data );
    p->deref();
    return p;
}

QDOM_AttrPrivate* QDOM_DocumentPrivate::createAttribute( const QString& name )
{
    QDOM_AttrPrivate* a = new QDOM_AttrPrivate( this, this, name );
    a->deref();
    return a;
}

QDOM_EntityReferencePrivate* QDOM_DocumentPrivate::createEntityReference( const QString& name )
{
    QDOM_EntityReferencePrivate* e = new QDOM_EntityReferencePrivate( this, this, name );
    e->deref();
    return e;
}

void QDOM_DocumentPrivate::save( QTextStream& s ) const
{
  bool doc = FALSE;

  QDOM_NodePrivate* n = m_firstChild;
  while( n )
  {
    if ( !doc && !n->isProcessingInstruction() )
    {
      type->save( s );
      doc = TRUE;
    }
    n->save( s );
    n = n->m_nextSibling;
  }
}

/*
static void qElementsByTagName( const QDomNode* node, const QString& tagname, QDomNodeList* l )
{
  const QDomNodeList* m = node->childNodes();
  uint len = m->length();
  for( uint i = 0; i < len; ++i )
  {
    const QDomNode* n = m->item( i );
    if ( n->nodeName() == tagname )
      l->appendChild( (QDomNode*)n );

    qElementsByTagName( n, tagname, l );
  }
}

QDomNodeList* QDOM_DocumentPrivate::elementsByTagName( const QString& tagname )
{
  NodeList* l = new NodeList( FALSE );

  qElementsByTagName( this, tagname, l );

  return l;
}
*/

/**************************************************************
 *
 * QDomDocument
 *
 **************************************************************/

#define IMPL ((QDOM_DocumentPrivate*)impl)

QDomDocument::QDomDocument()
{
}

QDomDocument::QDomDocument( const QString& name )
{
  // We take over ownership
  impl = new QDOM_DocumentPrivate( name );
}

QDomDocument::QDomDocument( const QDomDocument& x )
  : QDomNode( x )
{
}

QDomDocument::QDomDocument( QDOM_DocumentPrivate* x )
  : QDomNode( x )
{
}

QDomDocument::QDomDocument( QIODevice* dev )
{
  uint size = dev->size();
  char* buffer = new char[ size + 1 ];
  dev->readBlock( buffer, size );
  buffer[ size ] = 0;

  QString text = QString::fromUtf8( buffer, size );
  delete[] buffer;

  setContent( text );
}

QDomDocument& QDomDocument::operator= ( const QDomDocument& x )
{
  return (QDomDocument&) QDomNode::operator=( x );
}

QDomDocument::~QDomDocument()
{
}

bool QDomDocument::setContent( const QString& text )
{
  if ( !impl )
    impl = new QDOM_DocumentPrivate;
  return IMPL->setContent( text );
}

QDomDocumentType QDomDocument::doctype() const
{
  if ( !impl )
    return QDomDocumentType();
  return QDomDocumentType( IMPL->doctype() );
}

QDomImplementation QDomDocument::implementation() const
{
  if ( !impl )
    return QDomImplementation();
  return QDomImplementation( IMPL->implementation() );
}

QDomElement QDomDocument::documentElement() const
{
  if ( !impl )
    return QDomElement();
  return QDomElement( IMPL->documentElement() );
}

QDomElement QDomDocument::createElement( const QString& tagName )
{
  if ( !impl )
    return QDomElement();
  return QDomElement( IMPL->createElement( tagName ) );
}

QDomDocumentFragment QDomDocument::createDocumentFragment()
{
  if ( !impl )
    return QDomDocumentFragment();
  return QDomDocumentFragment( IMPL->createDocumentFragment() );
}

QDomText QDomDocument::createTextNode( const QString& value )
{
  if ( !impl )
    return QDomText();
  return QDomText( IMPL->createTextNode( value ) );
}

QDomComment QDomDocument::createComment( const QString& value )
{
  if ( !impl )
    return QDomComment();
  return QDomComment( IMPL->createComment( value ) );
}

QDomCDATASection QDomDocument::createCDATASection( const QString& value )
{
  if ( !impl )
    return QDomCDATASection();
  return QDomCDATASection( IMPL->createCDATASection( value ) );
}

QDomProcessingInstruction QDomDocument::createProcessingInstruction( const QString& target,
									 const QString& data )
{
  if ( !impl )
    return QDomProcessingInstruction();
  return QDomProcessingInstruction( IMPL->createProcessingInstruction( target, data ) );
}

QDomAttr QDomDocument::createAttribute( const QString& name )
{
  if ( !impl )
    return QDomAttr();
  return QDomAttr( IMPL->createAttribute( name ) );
}

QDomEntityReference QDomDocument::createEntityReference( const QString& name )
{
  if ( !impl )
    return QDomEntityReference();
  return QDomEntityReference( IMPL->createEntityReference( name ) );
}

QDomElement QDomDocument::createElement( const QString& tagname, const QRect& rect )
{
  if ( !impl )
    return QDomElement();
  QDomElement e( createElement( tagname ) );

  e.setAttribute( "x", rect.x() );
  e.setAttribute( "y", rect.y() );
  e.setAttribute( "width", rect.width() );
  e.setAttribute( "height", rect.height() );

  return e;
}

QDomElement QDomDocument::createElement( const QString& tagname, const QPoint& p )
{
  if ( !impl )
    return QDomElement();
  QDomElement e( createElement( tagname ) );

  e.setAttribute( "x", p.x() );
  e.setAttribute( "y", p.y() );

  return e;
}

QDomElement QDomDocument::createElement( const QString& tagname, const QSize& s )
{
  if ( !impl )
    return QDomElement();
  QDomElement e( createElement( tagname ) );

  e.setAttribute( "width", s.width() );
  e.setAttribute( "height", s.height() );

  return e;
}

QDomElement QDomDocument::createElement( const QString& tagname, const QPen& pen )
{
  if ( !impl )
    return QDomElement();
  QDomElement e( createElement( tagname ) );

  e.setAttribute( "color", pen.color().name() );
  e.setAttribute( "style", (int)pen.style() );
  e.setAttribute( "width", (int)pen.width() );

  return e;
}

QDomElement QDomDocument::createElement( const QString& tagname, const QFont& font )
{
  if ( !impl )
    return QDomElement();
  QDomElement e( createElement( tagname ) );

  e.setAttribute( "family", font.family() );
  e.setAttribute( "size", font.pointSize() );
  e.setAttribute( "weight", font.weight() );
  if ( font.bold() )
    e.setAttribute( "bold", "yes" );
  if ( font.italic() )
    e.setAttribute( "italic", "yes" );

  return e;
}

// TODO
// QDomNodeList QDomDocument::elementsByTagName( const QString& tagname )

QDomNode::NodeType QDomDocument::nodeType() const
{
  return DocumentNode;
}

bool QDomDocument::isDocument() const
{
  return TRUE;
}

/*!
  Returns the factory attached to this document.

  \sa setMimeSourceFactory()
*/
QDomMimeSourceFactory* QDomDocument::mimeSourceFactory()
{
  if ( !impl )
    return 0;
  return IMPL->mimeSourceFactory();
}

/*!
  Returns the factory attached to this document.

  \sa setMimeSourceFactory()
*/
const QDomMimeSourceFactory* QDomDocument::mimeSourceFactory() const
{
  if ( !impl )
    return 0;
  return IMPL->mimeSourceFactory();
}

/*!
  Changes the factory for this document. If it is set to zero, then
  the default factory as delivered by QDomMimeSourceFactory::defaultFactory is
  used.

  The factory is used to access external data such as pictures which reside
  on some external storage but are referenced in the document.

  \sa mimeSourceFactory()
*/
void QDomDocument::setMimeSourceFactory( QDomMimeSourceFactory* f )
{
  if ( !impl )
    return;
  IMPL->setMimeSourceFactory( f );
}

#undef IMPL

/*==============================================================*/
/*               Node casting functions                         */
/*==============================================================*/

QDomAttr QDomNode::toAttr()
{
  if ( impl && impl->isAttr() )
  {
      // impl->ref();
    return QDomAttr( ((QDOM_AttrPrivate*)impl) );
  }
  return QDomAttr();
}

QDomCDATASection QDomNode::toCDATASection()
{
  if ( impl && impl->isCDATASection() )
  {
      // impl->ref();
    return QDomCDATASection( ((QDOM_CDATASectionPrivate*)impl) );
  }
  return QDomCDATASection();
}

QDomDocumentFragment QDomNode::toDocumentFragment()
{
  if ( impl && impl->isDocumentFragment() )
  {
      // impl->ref();
    return QDomDocumentFragment( ((QDOM_DocumentFragmentPrivate*)impl) );
  }
  return QDomDocumentFragment();
}

QDomDocument QDomNode::toDocument()
{
  if ( impl && impl->isDocument() )
  {
      // impl->ref();
    return QDomDocument( ((QDOM_DocumentPrivate*)impl) );
  }
  return QDomDocument();
}

QDomDocumentType QDomNode::toDocumentType()
{
  if ( impl && impl->isDocumentType() )
  {
      // impl->ref();
    return QDomDocumentType( ((QDOM_DocumentTypePrivate*)impl) );
  }
  return QDomDocumentType();
}

QDomElement QDomNode::toElement()
{
  if ( impl && impl->isElement() )
  {
      // impl->ref();
    return QDomElement( ((QDOM_ElementPrivate*)impl) );
  }
  return QDomElement();
}

QDomEntityReference QDomNode::toEntityReference()
{
  if ( impl && impl->isEntityReference() )
  {
      // impl->ref();
    return QDomEntityReference( ((QDOM_EntityReferencePrivate*)impl) );
  }
  return QDomEntityReference();
}

QDomText QDomNode::toText()
{
  if ( impl && impl->isText() )
  {
      // impl->ref();
    return QDomText( ((QDOM_TextPrivate*)impl) );
  }
  return QDomText();
}

QDomEntity QDomNode::toEntity()
{
  if ( impl && impl->isEntity() )
  {
      // impl->ref();
    return QDomEntity( ((QDOM_EntityPrivate*)impl) );
  }
  return QDomEntity();
}

QDomNotation QDomNode::toNotation()
{
  if ( impl && impl->isNotation() )
  {
      // impl->ref();
    return QDomNotation( ((QDOM_NotationPrivate*)impl) );
  }
  return QDomNotation();
}

QDomProcessingInstruction QDomNode::toProcessingInstruction()
{
  if ( impl && impl->isProcessingInstruction() )
  {
      // impl->ref();
    return QDomProcessingInstruction( ((QDOM_ProcessingInstructionPrivate*)impl) );
  }
  return QDomProcessingInstruction();
}

QDomCharacterData QDomNode::toCharacterData()
{
  if ( impl && impl->isCharacterData() )
  {
      // impl->ref();
    return QDomCharacterData( ((QDOM_CharacterDataPrivate*)impl) );
  }
  return QDomCharacterData();
}

/*==============================================================*/
/*                      QDomConsumer                            */
/*==============================================================*/

/**************************************************
 *
 * QDomConsumer
 *
 **************************************************/

QDomConsumer::QDomConsumer( QDOM_DocumentPrivate* d )
{
  doc = d;
  node = doc;
  firstTag = FALSE;
}

QDomConsumer::~QDomConsumer()
{
}

bool QDomConsumer::tagStart( const QString& name )
{
  if ( node == doc )
  {
    // Document may only have one child
    if ( firstTag )
      return FALSE;
    firstTag = TRUE;
    // Has to be a special tag
    if ( name != doc->doctype()->nodeName() )
      return FALSE;
  }

  QDOM_NodePrivate* n = doc->createElement( name );
  node->appendChild( n );
  node = n;

  return TRUE;
}

bool QDomConsumer::tagEnd( const QString& name )
{
  if ( node == doc )
    return FALSE;

  if ( node->nodeName() != name )
  {
    qDebug("Tag %s does not close %s\n",name.ascii(),node->nodeName().ascii() );
    return FALSE;
  }

  node = node->parentNode();

  return TRUE;
}

bool QDomConsumer::attrib( const QString& name, const QString& value )
{
  if ( !node->isElement() )
    return FALSE;

  ((QDOM_ElementPrivate*)node)->setAttribute( name, value );

  return TRUE;
}

bool QDomConsumer::text( const QString& text )
{
  // No text as child of some document
  if ( node == doc )
    return FALSE;

  node->appendChild( doc->createTextNode( text ) );

  return TRUE;
}

bool QDomConsumer::cdata( const QString& text )
{
  // No text as child of some document
  if ( node == doc )
    return FALSE;

  node->appendChild( doc->createCDATASection( text ) );

  return TRUE;
}

bool QDomConsumer::entityRef( const QString& name )
{
  qDebug("ENTITYREF=%s\n",name.ascii());

  if ( node == doc )
    return FALSE;

  // TODO: Find corresponding entity
  QDOM_NamedNodeMapPrivate* m = doc->doctype()->entities();
  if ( !m )
    return FALSE;
  QDOM_NodePrivate* n = m->namedItem( name );
  if ( !n || !n->isEntity() )
  {
    qWarning( "Entity of name %s unsupported", name.latin1() );
    return FALSE;
  }

  node->appendChild( doc->createEntityReference( name ) );

  return TRUE;
}

bool QDomConsumer::processingInstruction( const QString& name, const QString& value )
{
  node->appendChild( doc->createProcessingInstruction( name, value ) );

  return TRUE;
}

bool QDomConsumer::doctype( const QString& name )
{
  qDebug("DOCTYPE %s", name.latin1());
  doc->doctype()->m_name = name;

  return TRUE;
}

bool QDomConsumer::doctypeExtern( const QString& publicId, const QString& systemId )
{
  printf("DOCTYPE EXTERN %s %s\n", publicId.ascii(),systemId.ascii());
  return TRUE;
}

bool QDomConsumer::element( const QString& )
{
    // printf("ELEMENT %s\n", data.ascii());
  return TRUE;
}

bool QDomConsumer::attlist( const QString& data )
{
  printf("ATTLIST %s\n", data.ascii());
  return TRUE;
}

bool QDomConsumer::parameterEntity( const QString& name, const QString& publicId, const QString& systemId )
{
  printf("ENTITY PARAM %s %s %s\n", name.ascii(),publicId.ascii(),systemId.ascii());
  return TRUE;
}

bool QDomConsumer::parameterEntity( const QString& name, const QString& value )
{
  printf("ENTITY PARAM %s %s\n", name.ascii(),value.ascii());
  return TRUE;
}

bool QDomConsumer::entity( const QString& name, const QString& publicId, const QString& systemId, const QString& ndata )
{
  printf("ENTITY %s %s %s %s\n", name.ascii(),publicId.ascii(),systemId.ascii(),ndata.ascii());
  QDOM_EntityPrivate* e = new QDOM_EntityPrivate( doc, 0, name, publicId, systemId, ndata );
  doc->doctype()->appendChild( e );
  return TRUE;
}

bool QDomConsumer::entity( const QString& name, const QString& value )
{
  printf("ENTITY %s %s\n", name.ascii(),value.ascii());

  QDOM_EntityPrivate* e = new QDOM_EntityPrivate( doc, 0, name, QString::null, QString::null, QString::null );
  e->m_value = value;
  doc->doctype()->appendChild( e );

  return TRUE;
}

bool QDomConsumer::notation( const QString& name, const QString& publicId, const QString& systemId )
{
  printf("NOTATION %s %s %s\n", name.ascii(),publicId.ascii(),systemId.ascii());
  QDOM_NotationPrivate* n = new QDOM_NotationPrivate( doc, 0, name, publicId, systemId );
  doc->doctype()->appendChild( n );
  return TRUE;
}

void QDomConsumer::parseError( int pos, int line, int linepos )
{
    qDebug("QDom: Parsing error at line %i:%i, total position %i.", line, linepos, pos );
}

bool QDomConsumer::finished()
{
  if ( node != doc )
    return FALSE;
  return TRUE;
}

/**************************************************
 *
 * QDomMimeSourceFactory
 *
 **************************************************/

class QDOM_MimeSourceFactoryPrivate
{
public:
    QMap<int,QString> m_map;

    static QDomMimeSourceFactory* s_factory;
};

QDomMimeSourceFactory* QDOM_MimeSourceFactoryPrivate::s_factory = 0;

QDomMimeSourceFactory::QDomMimeSourceFactory()
{
    d = new QDOM_MimeSourceFactoryPrivate;
}

QDomMimeSourceFactory::~QDomMimeSourceFactory()
{
    delete d;
}

QPixmap QDomMimeSourceFactory::pixmap( const QString& name )
{
    QPixmap pix;
    const QMimeSource* m = data( name );

    if ( m && m->provides( "image/x-xpm" ) )
	pix = m->encodedData( "image/x-xpm" );
    if ( m && m->provides( "image/png" ) )
	pix = QPixmap( m->encodedData( "image/png" ) );
    if ( m && m->provides( "image/bmp" ) )
	pix = QPixmap( m->encodedData( "image/bmp" ) );
    if ( m && m->provides( "image/jpeg" ) )
	pix = QPixmap( m->encodedData( "image/jpeg" ) );

    if ( pix.isNull() )
	return pix;

    d->m_map[ pix.serialNumber() ] = name;

    return pix;
}

QString QDomMimeSourceFactory::pixmapName( const QPixmap& pix ) const
{
    QMap<int,QString>::Iterator it = d->m_map.find( pix.serialNumber() );
    if ( it == d->m_map.end() )
	return QString::null;
    return it.data();
}

QDomMimeSourceFactory* QDomMimeSourceFactory::defaultDomFactory()
{
    if ( !QDOM_MimeSourceFactoryPrivate::s_factory )	
	QDOM_MimeSourceFactoryPrivate::s_factory = new QDomMimeSourceFactory;
    return QDOM_MimeSourceFactoryPrivate::s_factory;
}

void QDomMimeSourceFactory::setDefaultDomFactory( QDomMimeSourceFactory* f )
{
    QDOM_MimeSourceFactoryPrivate::s_factory = f;
}
