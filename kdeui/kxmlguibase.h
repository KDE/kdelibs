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
#ifndef _KXMLGUIBASE_H
#define _KXMLGUIBASE_H

#include <kxmlgui.h>

class QAction;
class QActionCollection;
class KInstance;
class KXMLGUIBasePrivate;

class KXMLGUIBase : public KXMLGUIServant
{
public:
  KXMLGUIBase();
  virtual ~KXMLGUIBase();

  QAction* action( const char* name );
  virtual QAction *action( const QDomElement &element );
  QActionCollection* actionCollection() const;

  /**
   * @return The instance (@ref KInstance) for this part.
   */
  virtual KInstance *instance() const;

  /**
   * @return The parsed XML in a @ref QDomDocument, set by @ref
   * setXMLFile() or @ref setXML()
   */
  virtual QDomDocument document() const;

protected:  
  /**
   * Set the instance (@ref KInstance) for this part.
   *
   * Call this first in the inherited class constructor.
   * (At least before @ref setXMLFile().)
   */
  virtual void setInstance( KInstance *instance );

  /**
   * Set the name of the rc file containing the XML for the part.
   *
   * Call this in the Part-inherited class constructor.
   *
   * @param file Either an absolute path for the file, or simply the
   *             filename, which will then be assumed to be installed
   *             in the "data" resource, under a directory named like
   *             the instance.
   **/
  virtual void setXMLFile( QString file, bool merge = false );

  /**
   * Set the XML for the part.
   *
   * Call this in the Part-inherited class constructor if you
   *  don't call @ref setXMLFile().
   **/
  virtual void setXML( const QString &document, bool merge = false );

private:
  bool mergeXML( QDomElement &base, const QDomElement &additive,
                 QActionCollection *actionCollection );
  QDomElement findMatchingElement( const QDomElement &base,
                                   const QDomElement &additive );

  KXMLGUIBasePrivate *d;
};

#endif
