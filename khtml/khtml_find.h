#ifndef __khtml_find_h__
#define __khtml_find_h__

#include <keditcl.h>

class KHTMLPart;

class KHTMLFind : public KEdFind
{
  Q_OBJECT
public:
  KHTMLFind( KHTMLPart *part, const char *name );
  virtual ~KHTMLFind();

private slots:
  void slotDone();
  void slotSearch();

private:
  bool m_first;
  KHTMLPart *m_part;
};

#endif
