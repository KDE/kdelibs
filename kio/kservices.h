#ifndef __kservices_h__
#define __kservices_h__

#include <qlist.h>
#include <qvaluelist.h>
#include <qstring.h>

#include <ksimpleconfig.h>

class KServiceTypeProfile;

/** Represents a service, i.e. an application bound to one or several mimetypes
 * as written in its kdelnk file.
 *
 * IMPORTANT : to use the public static methods of this class, you must do 
 * the following registry initialisation (in main() for instance)
 * <pre>
 * #include <kregistry.h>
 * #include <kregfactories.h> 
 *
 *   KRegistry registry;
 *   registry.addFactory( new KServiceFactory );
 *   registry.load();
 * </pre>
 */
class KService
{
public:  
  KService( const QString& _name, const QString& _exec, const QString& _icon,
	    const QStringList& _lstServiceTypes, const QString& _comment = QString::null,
	    bool _allow_as_default = true, const QString& _path = QString::null,
	    const QString& _terminal = QString::null, const QString& _file = QString::null, 
	    bool _put_in_list = true );
  ~KService();
  
  QString name() const { return m_strName; }
  QString exec() const { return m_strExec; }
  QString icon() const { return m_strIcon; }
  QString terminalOptions() const { return m_strTerminalOptions; }
  QString path() const { return m_strPath; }
  QString comment() const { return m_strComment; }
  QString file() const { return m_strFile; };
  QStringList serviceTypes() const { return m_lstServiceTypes; }
  bool hasServiceType( const QString& _service ) const;
  /**
   * @return TRUE if the service may be used as a default setting, for
   *         example in a file manager. Usually that is the case, but
   *         some services may only be started when the user selected
   *         them. This kind of services returns FALSE here.
   */
  bool allowAsDefault() const { return m_bAllowAsDefault; }
  
  /**
   * @return a pointer to the requested service or 0 if the service is
   *         unknown.
   */
  static KService* find( const QString& _name );
  
  /**
   * @param _file is only used while displaying error messages.
   * @param _put_in_list will add the service to the list of known
   *        services. But sometimes you may just want to create
   *        a service object for internal purposes.
   * @return a new service or 0 on error. If _put_in_list is FALSE, then
   *         you have to destroy the object somewhen, otherwise not.
   */
  static KService* parseService( const QString& _file, KSimpleConfig &config,
				 bool _put_in_list = true );

  /**
   * @return the whole list of services. Useful to display them.
   */
  static QList<KService>* allServices() { return s_lstServices; }

protected:  
  static void initServices( const QString&  _path );

private:
  static void initStatic();

  QString m_strName;
  QString m_strExec;
  QString m_strIcon;
  QString m_strTerminalOptions;
  QString m_strPath;
  QString m_strComment;
  QString m_strFile;
  QStringList m_lstServiceTypes;
  bool m_bAllowAsDefault;
  
  static QList<KService>* s_lstServices;
};

#endif
