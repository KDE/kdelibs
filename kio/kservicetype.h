#ifndef __kservicetype_h__
#define __kservicetype_h__

#include <qstring.h>
#include <qlist.h>

class KServiceTypeFactory;

class KServiceType
{
  friend KServiceTypeFactory;
public:
  KServiceType( const QString& _name, const QString& _icon, const QString& _comment );
  virtual ~KServiceType();
  
  /**
   * @return the icon associated with this service type. Some derived classes offer
   *         special functions which take for example an URL and returns a special icon
   *         for this URL. An example is @ref KMimeType, @ref KFoldeType and others.
   */
  virtual QString icon() const { return m_strIcon; }
  /**
   * @param _url may be 0L
   */
  virtual QString comment() const { return m_strComment; }
  
  /**
   * @return the name of this service type.
   */
  virtual QString name() const { return m_strName; }
  
  /**
   * @return a pointer to the servicetype '_name' or 0L if the service type is unknown.
   */
  static KServiceType* serviceType( const QString& _name );

  /**
   * Get all the servicetypes dict. Useful for showing the list of available mimetypes.
   */
  static const QList<KServiceType>& serviceTypes() { return *s_lstServiceTypes; }

protected:
  /**
   * Check for static variables initialised. Called by constructor.
   */
  static void initStatic();

  QString m_strName;
  QString m_strIcon;
  QString m_strComment;
  
  static QList<KServiceType>* s_lstServiceTypes;
};

#endif
