#ifndef __k_reg_factories_h__
#define __k_reg_factories_h__

#include "kregistry.h"

class KMimeType;
class KService;

class KMimeTypeEntry : public KRegEntry
{
public:
  KMimeTypeEntry( KRegistry* _reg, const char *_file, KMimeType *_mime );
  virtual ~KMimeTypeEntry();
  
protected:
  virtual bool updateIntern();
  
  KMimeType* m_pMimeType;
};

class KMimeTypeFactory : public KRegFactory
{
  friend KMimeTypeEntry;
  
public:
  KMimeTypeFactory();
  virtual ~KMimeTypeFactory() { };
  
  virtual KRegEntry* create( KRegistry* _reg, const char *_file, KSimpleConfig &_cfg );
  virtual const char* type();
  /**
   * @return the path for which this factory is responsible.
   *         Please note that the return value may not have a trailing '/'.
   */
  virtual QStrList& pathList();

protected:
  KMimeType* createMimeType( const char *_file, KSimpleConfig &_cfg );
  
  QStrList m_lst;
};



class KServiceEntry : public KRegEntry
{
public:
  KServiceEntry( KRegistry* _reg, const char *_file, KService *_service );
  virtual ~KServiceEntry();
  
protected:
  virtual bool updateIntern();
  
  KService* m_pService;
};

class KServiceFactory : public KRegFactory
{
  friend KServiceEntry;
  
public:
  KServiceFactory();
  virtual ~KServiceFactory() { };
  
  virtual KRegEntry* create( KRegistry* _reg, const char *_file, KSimpleConfig &_cfg );
  virtual const char* type();
  /**
   * @return the path for which this factory is responsible.
   *         Please note that the return value may not have a trailing '/'.
   */
  virtual QStrList& pathList();

protected:
  KService* createService( const char *_file, KSimpleConfig &_cfg );
  
  QStrList m_lst;
};

#endif
