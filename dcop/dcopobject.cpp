#include <dcopobject.h>

static QMap<QString, DCOPObject *> *objMap_ = 0;

inline QMap<QString, DCOPObject *> *objMap()
{
  if (!objMap_)
    objMap_ = new QMap<QString, DCOPObject *>;
  return objMap_;
}

DCOPObject::DCOPObject(QObject *obj)
{
  QObject *currentObj = obj;
  while (currentObj != 0L) {
    ident.prepend(QString::fromLatin1(currentObj->name()));
    ident.prepend('/');
    currentObj = currentObj->parent();
  }
  ident = ident.mid(2);
  qDebug("set dcopobject id to %s",ident.latin1());

  objMap()->insert(ident, this);
}

DCOPObject::DCOPObject(const QString &objId)
  : ident(objId)
{
  ident = objId;
  objMap()->insert(ident, this);
}

DCOPObject::~DCOPObject()
{
  objMap()->remove(ident);
}

QString DCOPObject::id() const
{
  return ident;
}

bool DCOPObject::process(const QString &fun, const QByteArray &data,
			 QByteArray &replyData)
{
  qDebug("called DCOPObject::process");
  return false;
}

bool DCOPObject::hasObject(const QString &objId)
{
  if (objMap()->contains(objId))
    return true;
  else
    return false;
}

DCOPObject *DCOPObject::find(const QString &objId)
{
  QMap<QString, DCOPObject *>::ConstIterator it;
  it = objMap()->find(objId);
  if (it != objMap()->end())
    return *it;
  else
    return 0L;
}
