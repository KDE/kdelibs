#pragma implementation

#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "kmimesourcefactory.h"

class KMimeSourceFactory::KMimeSourceFactoryPrivate
{
public:
  inline KMimeSourceFactoryPrivate (KIconLoader* loader)
	: kil (loader)
  {}
  
  KIconLoader* kil;
};

KMimeSourceFactory::KMimeSourceFactory (KIconLoader* loader)
  : QMimeSourceFactory (),
	d (new KMimeSourceFactoryPrivate (loader))
{
}

KMimeSourceFactory::~KMimeSourceFactory()
{
  delete d;
}

QString KMimeSourceFactory::makeAbsolute (const QString& absOrRelName, const QString& context) const
{
  QString myName;
  QString myContext;

  const int pos = absOrRelName.find ('|');
  if (pos > -1)
	{
	  myContext = absOrRelName.left (pos);
	  myName = absOrRelName.right (absOrRelName.length() - myContext.length() - 1);
	}

  QString result;

  if (myContext == "desktop")
	{
	  result = d->kil->iconPath (myName, KIcon::Desktop);
	}
  else if (myContext == "toolbar")
	{	 
	  result = d->kil->iconPath (myName, KIcon::Toolbar);
	}
  else if (myContext == "maintoolbar")
	{
	  result = d->kil->iconPath (myName, KIcon::MainToolbar);
	}
  else if (myContext == "small", true)
	{
	  result = d->kil->iconPath (myName, KIcon::Small);
	}
  else if (myContext == "user")
	{	  
	  result = d->kil->iconPath (myName, KIcon::User);
	}

  if (result.isEmpty())
	result = QMimeSourceFactory::makeAbsolute (absOrRelName, context);
  
  return result;
}
