/*
  configmodule.h - module definition for the KDE control center

  written 1997 by Matthias Hoelzer
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.
  
  You should have received a copy of the GNU Library General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */


#ifndef _CONFIG_MODULE_
#define _CONFIG_MODULE_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

class KConfig;
#include <qtabdlg.h>
#include <qstrlist.h>

#include <klocale.h>

#ifdef klocale
#undef klocale
#endif

#define klocale kcclocale

class KCCLocale;

extern KCCLocale *kcclocale;

class KCCLocale : public KLocale {

public:
    KCCLocale( const char *catalogue = 0L );
    ~KCCLocale() {};

    const char *translate( const char *index );
    void addDomain( const char *catalogue );

private:
    QStrList domains;
};

class KPeerConfig
{
public:

  KPeerConfig(char *peerName);
  ~KPeerConfig();

  KConfig *getConfig();

private:

  KConfig     *conf;
};


class KConfigWidget : public QTabDialog
{
  Q_OBJECT;

public:

  KConfigWidget(QWidget *parent, const char *name=0, bool modal=FALSE);
};


class KConfigModule
{ 
public:

  KConfigModule(const QString &title);
  virtual ~KConfigModule();
  
  virtual const QString &getName();
  virtual const QString &getGroup();
  virtual const QString &getTitle();
  
  virtual const QString &getHint();
  virtual const QString &getIcon();
  virtual const QString &getHelpFile();

  virtual bool getNeedsRoot();
  
  virtual bool getSupportsInit();
  virtual void init();
    
  virtual void showAboutDialog(QWidget *);
  
  virtual KConfigWidget *getNewWidget(QWidget *parent, const char *name,
				      bool modal=FALSE) = 0;

protected:

  void setName(const QString &n);
  void setGroup(const QString &g);
  void setHint(const QString &h);
  void setIcon(const QString &i);
  void setHelpFile(const QString &h);
  void setSupportsInit(bool s);
  void setNeedsRoot(bool n);
    
private:

  QString name;
  QString group;
  QString hint;
  QString icon;
  QString help;
  QString title;
  KCCLocale *pLocale;

  bool    supportsInit;
  bool    needsRoot;
};

#endif
