#include "kserviceoffer.h"

KServiceOffer::KServiceOffer()
{
  m_iPreference = -1;
}

KServiceOffer::KServiceOffer( const KServiceOffer& _o )
{
  m_pService = _o.m_pService;
  m_iPreference = _o.m_iPreference;
  m_bAllowAsDefault = _o.m_bAllowAsDefault;
}

KServiceOffer::KServiceOffer( KService::Ptr _service, int _pref, bool _default )
{
  m_pService = _service;
  m_iPreference = _pref;
  m_bAllowAsDefault = _default;
}

bool KServiceOffer::operator< ( const KServiceOffer& _o ) const
{
  // Put offers allowed as default FIRST.
  if ( _o.m_bAllowAsDefault && !m_bAllowAsDefault )
    return false; // _o is default and not 'this'.
  if ( !_o.m_bAllowAsDefault && m_bAllowAsDefault )
    return true; // 'this' is default but not _o.
 // Both offers are allowed or not allowed as default
 // -> use preferences to sort them
 // The bigger the better, but we want the better FIRST
  return _o.m_iPreference < m_iPreference;
}
