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
#ifndef _KXMLGUICLIENT_H
#define _KXMLGUICLIENT_H

#include <qdom.h>
#include <qptrlist.h>
#include <qmap.h>
#include <qstringlist.h>

class QWidget;
class KAction;
class KActionCollection;
class KInstance;
class KXMLGUIClientPrivate;
class KXMLGUIFactory;
class KXMLGUIBuilder;

/**
 * A KXMLGUIClient can be used with @ref KXMLGUIFactory to create a GUI from actions
 * and an XML document, and can be dynamically merged with other KXMLGUIClients.
 */
class KXMLGUIClient
{
public:
  /**
   * Constructs a KXMLGUIClient which can be used with a
   * @ref KXMLGUIFactory to create a GUI from actions and an XML document, and
   * which can be dynamically merged with other KXMLGUIClients.
   */
  KXMLGUIClient();

  /**
   * Constructs a KXMLGUIClient which can be used with a @ref KXMLGUIFactory
   * to create a GUI from actions and an XML document,
   * and which can be dynamically merged with other KXMLGUIClients.
   *
   * This constructor takes an additional @p parent argument, which makes
   * the client a child client of the parent.
   *
   * Child clients are automatically added to the GUI if the parent is added.
   *
   */
  KXMLGUIClient( KXMLGUIClient *parent );

  /**
   * Destructs the KXMLGUIClient.
   */
  virtual ~KXMLGUIClient();

  /**
   * Retrieves an action of the client by name.  If not found, it looks in its child clients.
   * This method is provided for convenience, as it uses @ref #actionCollection()
   * to get the action object.
   */
  KAction* action( const char* name ) const;

  /**
   * Retrieves an action for a given @ref QDomElement. The default
   * implementation uses the "name" attribute to query the action
   * object via the other action() method.
   */
  virtual KAction *action( const QDomElement &element ) const;

  /**
   * Retrieves the entire action collection for the GUI client
   */
  virtual KActionCollection* actionCollection() const;

  /**
   * @return The instance (@ref KInstance) for this part.
   */
  virtual KInstance *instance() const;

  /**
   * @return The parsed XML in a @ref QDomDocument, set by @ref
   * setXMLFile() or @ref setXML().
   * This document describes the layout of the GUI.
   */
  virtual QDomDocument domDocument() const;

  /**
   * This will return the name of the XML file as set by @ref #setXMLFile().
   * If @ref #setXML() is used directly, then this will return NULL.
   *
   * The filename that this returns is obvious for components as each
   * component has exactly one XML file.  In non-components, however,
   * there are usually two: the global file and the local file.  This
   * function doesn't really care about that, though.  It will always
   * return the last XML file set.  This, in almost all cases, will
   * be the local XML file.
   *
   * @return The name of the XML file or QString::null
   */
  virtual QString xmlFile() const;

  virtual QString localXMLFile() const;

  /**
   * @internal
   */
  void setXMLGUIBuildDocument( const QDomDocument &doc );
  /**
   * @internal
   */
  QDomDocument xmlguiBuildDocument() const;

  /**
   * This method is called by the @ref KXMLGUIFactory as soon as the client
   * is added to the KXMLGUIFactory's GUI.
   */
  void setFactory( KXMLGUIFactory *factory );
  /**
   * Retrieves a pointer to the @ref KXMLGUIFactory this client is
   * associated with (will return 0L if the client's GUI has not been built
   * by a KXMLGUIFactory.
   */
  KXMLGUIFactory *factory() const;

  /**
   * KXMLGUIClients can form a simple child/parent object tree. This
   * method returns a pointer to the parent client or 0L if it has no
   * parent client assigned.
   */
  KXMLGUIClient *parentClient() const;

  /**
   * Use this method to make a client a child client of another client.
   * Usually you don't need to call this method, as it is called
   * automatically when using the second constructor, which takes a
   * arent argument.
   */
  void insertChildClient( KXMLGUIClient *child );

  /**
   * Removes the given @p child from the client's children list.
   */
  void removeChildClient( KXMLGUIClient *child );

  /**
   * Retrieves a list of all child clients.
   */
  const QPtrList<KXMLGUIClient> *childClients();

  /**
   * A client can have an own @ref KXMLGUIBuilder.
   * Use this method to assign your builder instance to the client (so that the
   * @ref KXMLGUIFactory can use it when building the client's GUI)
   *
   * Client specific guibuilders are useful if you want to create
   * custom container widgets for your GUI.
   */
  void setClientBuilder( KXMLGUIBuilder *builder );

  /**
   * Retrieves the client's GUI builder or 0L if no client specific
   * builder has been assigned via @ref #setClientBuilder()
   */
  KXMLGUIBuilder *clientBuilder() const;

  /**
   * Forces this client to re-read its XML resource file.  This is
   * intended to be used when you know that the resource file has
   * changed and you will soon be rebuilding the GUI.  It has no
   * useful effect with non-KParts GUIs, so don't bother using it
   * unless your app is component based.
   */
  void reloadXML();

  void plugActionList( const QString &name, const QPtrList<KAction> &actionList );
  void unplugActionList( const QString &name );

  static QString findMostRecentXMLFile( const QStringList &files, QString &doc );

  void addStateActionEnabled(const QString& state, const QString& action);

  void addStateActionDisabled(const QString& state, const QString& action);

  enum ReverseStateChange { StateNoReverse, StateReverse };
  struct StateChange
  {
    QStringList actionsToEnable;
    QStringList actionsToDisable;
  };

  StateChange getActionsToChangeForState(const QString& state);

  void beginXMLPlug( QWidget * );
  void endXMLPlug();
  void prepareXMLUnplug( QWidget * );

protected:
  /**
   * Returns true if client was added to super client list.
   * Returns false if client was already in list.
   */
  //bool addSuperClient( KXMLGUIClient * );

  /**
   * Sets the instance (@ref KInstance) for this part.
   *
   * Call this first in the inherited class constructor.
   * (At least before @ref setXMLFile().)
   */
  virtual void setInstance( KInstance *instance );

  /**
   * Sets the name of the rc file containing the XML for the part.
   *
   * Call this in the Part-inherited class constructor.
   *
   * @param file Either an absolute path for the file, or simply the
   *             filename, which will then be assumed to be installed
   *             in the "data" resource, under a directory named like
   *             the instance.
   * @param setXML Specify whether to call setXML. Default is true.
   *               and the DOM document at once.
   **/
  virtual void setXMLFile( const QString& file, bool merge = false, bool setXMLDoc = true );

  virtual void setLocalXMLFile( const QString &file );

  /**
   * Sets the XML for the part.
   *
   * Call this in the Part-inherited class constructor if you
   *  don't call @ref setXMLFile().
   **/
  virtual void setXML( const QString &document, bool merge = false );

  /**
   * Sets the Document for the part, describing the layout of the GUI.
   *
   * Call this in the Part-inherited class constructor if you don't call
   * @ref setXMLFile or @ref setXML .
   */
  virtual void setDOMDocument( const QDomDocument &document, bool merge = false );

  /**
   * This function will attempt to give up some memory after the GUI
   * is built.  It should never be used in apps where the GUI may be
   * rebuilt at some later time (components, for instance).
   */
  virtual void conserveMemory();

  /**
   * Actions can collectively be assigned a "State". To accomplish this
   * the respective actions are tagged as <enable> or <disable> in
   * a <State> </State> group of the XMLfile. During program execution the
   * programmer can call stateChanged() to set actions to a defined state.
   *
   * @param newstate Name of a State in the XMLfile.
   * @param reverse If the flag reverse is set to StateReverse, the State is reversed.
   * (actions to be enabled will be disabled and action to be disabled will be enabled)
   * Default is reverse=false.
   */
   virtual void stateChanged(const QString &newstate, ReverseStateChange reverse = StateNoReverse);

private:
  struct DocStruct
  {
    QString file;
    QString data;
  };

  bool mergeXML( QDomElement &base, const QDomElement &additive,
                 KActionCollection *actionCollection );

  QDomElement findMatchingElement( const QDomElement &base,
                                   const QDomElement &additive );

  typedef QMap<QString, QMap<QString, QString> > ActionPropertiesMap;

  static ActionPropertiesMap extractActionProperties( const QDomDocument &doc );

  static void storeActionProperties( QDomDocument &doc, const ActionPropertiesMap &properties );

  static QString findVersionNumber( const QString &_xml );

  // Actions to enable/disable on a state change
  QMap<QString,StateChange> m_actionsStateMap;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KXMLGUIClientPrivate *d;
};

#endif
