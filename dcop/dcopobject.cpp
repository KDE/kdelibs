#include <dcopobject.h>

static QMap<QCString, DCOPObject *> *objMap_ = 0;

inline QMap<QCString, DCOPObject *> *objMap()
{
  if (!objMap_)
    objMap_ = new QMap<QCString, DCOPObject *>;
  return objMap_;
}

DCOPObject::DCOPObject(QObject *obj)
{
  QObject *currentObj = obj;
  while (currentObj != 0L) {
    ident.prepend( currentObj->name() );
    ident.prepend("/");
    currentObj = currentObj->parent();
  }
  ident = ident.mid(2);
  qDebug("set dcopobject id to %s",ident.data());

  objMap()->insert(ident, this);
}

DCOPObject::DCOPObject(const QCString &objId)
  : ident(objId)
{
  ident = objId;
  objMap()->insert(ident, this);
}

DCOPObject::~DCOPObject()
{
  objMap()->remove(ident);
}

QCString DCOPObject::id() const
{
  return ident;
}

bool DCOPObject::process(const QCString &fun, const QByteArray &data,
			 QByteArray &replyData)
{
  qDebug("called DCOPObject::process");
  return false;
}

bool DCOPObject::hasObject(const QCString &objId)
{
  if (objMap()->contains(objId))
    return true;
  else
    return false;
}

DCOPObject *DCOPObject::find(const QCString &objId)
{
  QMap<QCString, DCOPObject *>::ConstIterator it;
  it = objMap()->find(objId);
  if (it != objMap()->end())
    return *it;
  else
    return 0L;
}
