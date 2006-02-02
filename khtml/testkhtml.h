#ifndef TESTKHTML_H
#define TESTKHTML_H

#include <kdebug.h>

/**
 * @internal
 */
class Dummy : public QObject
{
  Q_OBJECT
public:
  Dummy( KHTMLPart *part ) : QObject( part ) { m_part = part; };

private Q_SLOTS:
  void slotOpenURL( const KUrl &url, const KParts::URLArgs &args )
  {
    m_part->browserExtension()->setURLArgs( args );
    m_part->openURL( url );
  }
  void reload()
  {
      KParts::URLArgs args; args.reload = true;
      m_part->browserExtension()->setURLArgs( args );
      m_part->openURL( m_part->url() );
  }
  
  void toggleNavigable(bool s)
  {
      m_part->setCaretMode(s);
  }

  void toggleEditable(bool s)
  {
  kDebug() << "editable: " << s << endl;
      m_part->setEditable(s);
  }

private:
  KHTMLPart *m_part;
};

#endif
