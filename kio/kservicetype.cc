#include "kservicetype.h"

QList<KServiceType>* KServiceType::s_lstServiceTypes = 0L;

KServiceType::KServiceType( const QString& _type, const QString& _icon, const QString& _comment )
{
  initStatic();
  s_lstServiceTypes->append( this );
  
  m_strName = _type;
  m_strIcon = _icon;
  m_strComment = _comment;
}

KServiceType::~KServiceType()
{
  s_lstServiceTypes->removeRef( this );
}

void KServiceType::initStatic()
{
  if ( s_lstServiceTypes == 0 )
    s_lstServiceTypes = new QList<KServiceType>;
}
