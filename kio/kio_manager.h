#ifndef __manager_h__
#define __manager_h__

#include <string>
#include <list>

class K2Config;

class ProtocolManager
{
public:
  enum Type { T_STREAM, T_FILESYSTEM, T_NONE, T_ERROR };

  ProtocolManager();
  
  string find( const char *_protocol );
  Type inputType( const char *_protocol );
  Type outputType( const char *_protocol );
  bool listing( const char *_protocol, list<string>& _listing );
  bool supportsListing( const char *_protocol );
  
  static ProtocolManager* self() { 
    if ( ! s_pManager )
      s_pManager = new ProtocolManager;
    return s_pManager;
  }
  
protected:
  void init();
  K2Config* findIntern( const char *_protocol );
  
  K2Config *m_pConfig;
  
  static ProtocolManager *s_pManager;
};

#endif
