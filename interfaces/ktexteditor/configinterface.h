/* This file is part of the KDE libraries
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>

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

#ifndef __ktexteditor_configinterface_h__
#define __ktexteditor_configinterface_h__

#include <kdelibs_export.h>

#include <qwidget.h>
#include <qpixmap.h>
#include <kicontheme.h>

class KConfig;

namespace KTextEditor
{

class KTEXTEDITOR_EXPORT ConfigPage : public QWidget
{
  Q_OBJECT

  public:
    ConfigPage ( QWidget *parent=0, const char *name=0 ) : QWidget (parent, name) {}
    virtual ~ConfigPage () {}

  //
  // slots !!!
  //
  public:
    /**
      Applies the changes to all documents
    */
    virtual void apply () = 0;
    
    /**
      Reset the changes
    */
    virtual void reset () = 0;
    
    /**
      Sets default options
    */
    virtual void defaults () = 0;

  signals:
    /**
      Emitted when something changes
    */
    void changed();
};

/**
* This is an interface for accessing the configuration of the
* Document and Plugin classes.
*/
class KTEXTEDITOR_EXPORT ConfigInterface
{
  public:
    virtual ~ConfigInterface() {}

  //
  // slots !!!
  //
  public:    
    /**
      Read/Write the config to the standard place where this editor
      part saves it config, say: read/save default values for that
      editor part, which means for all current open documents
    */
    virtual void readConfig () = 0;
    virtual void writeConfig () = 0;           
                                                                         
    /**
      Read/Write the config of the part to a given kconfig object
      to store the settings in a different place than the standard
    */
    virtual void readConfig (KConfig *) = 0;
    virtual void writeConfig (KConfig *) = 0;
    
    /**
      Shows a config dialog for the part, changes will be applied
      to the part, but not saved anywhere automagically, call
      writeConfig to save it
    */
    virtual void configDialog () = 0;
    
   //
  // slots !!!
  //
  public:
    /**
      Number of available config pages
    */
    virtual uint configPages () const = 0;
    
    /**
      returns config page with the given number,
      config pages from 0 to configPages()-1 are available
      if configPages() > 0
    */ 
    virtual ConfigPage *configPage (uint number = 0, QWidget *parent = 0, const char *name=0 ) = 0;
  
    virtual QString configPageName (uint number = 0) const = 0;
    virtual QString configPageFullName (uint number = 0) const = 0;
    virtual QPixmap configPagePixmap (uint number = 0, int size = KIcon::SizeSmall) const = 0;
};

class Plugin;
class Document;

KTEXTEDITOR_EXPORT ConfigInterface *configInterface (Document *doc);
KTEXTEDITOR_EXPORT ConfigInterface *configInterface (Plugin *plugin);

}

#endif
