#ifndef __kservices_h__
#define __kservices_h__

#include <string>
#include <list>

#include <qlist.h>

#include <ksimpleconfig.h>

class KServiceTypeProfile;

class KService
{
public:
  struct Offer
  {
    KService* m_pService;

    bool allowAsDefault();
    int preference();
    
    ///////
    // For internal use only. These variables are needed for sorting
    ///////
    string m_strServiceType;
    KServiceTypeProfile *m_pServiceTypeProfile;
    
    bool operator< ( Offer& _o );
  };
  
  KService( const char *_name, const char *_exec, const char *_icon, list<string>& _lstServiceTypes,
	    const char *_comment = 0L, bool _allow_as_default = true,
	    const char *_path = 0L, const char *_terminal = 0L, bool _put_in_list = true );
  ~KService();
  
  const char* name() { return m_strName.c_str(); }
  const char* exec() { return m_strExec.c_str(); }
  const char* icon() { return m_strIcon.c_str(); }
  const char* terminalOptions() { return m_strTerminalOptions.c_str(); }
  const char* path() { return m_strPath.c_str(); }
  const char* comment() { return m_strComment.c_str(); }
  bool hasServiceType( const char *_service );
  bool allowAsDefault() { return m_bAllowAsDefault; }
  
  static void initStatic();
  /**
   * @param _result is filled with all matching offers. The list is sorted accrodingly
   *        to the users profile ( @ref KServiceTypeProfile ).
   */
  static void findServiceByServiceType( const char* _servicetype, list<Offer>& _result );

  static KService* findByName( const char *_name );
  
  /**
   * @param _file is only used while displaying error messages.
   */
  static KService* parseService( const char *_file, KSimpleConfig &config, bool _put_in_list = true );
  
protected:
  static void initServices( const char * _path );
  
  string m_strName;
  string m_strExec;
  string m_strIcon;
  string m_strTerminalOptions;
  string m_strPath;
  string m_strComment;
  list<string> m_lstServiceTypes;
  bool m_bAllowAsDefault;
  
  static QList<KService>* s_lstServices;
};

#endif
