#ifndef TESTKHTML_H
#define TESTKHTML_H

#include <kdebug.h>
#include <qvaluelist.h>
#include <qdatetime.h>

/**
 * @internal
 */
class Dummy : public QObject
{
  Q_OBJECT
public:
  Dummy( KHTMLPart *part ) : QObject( part ) { m_part = part; };

private slots:
  void slotOpenURL( const KURL &url, const KParts::URLArgs &args )
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
  kdDebug() << "editable: " << s << endl;
      m_part->setEditable(s);
  }

  void doBenchmark();

  void handleDone();

private:
  KHTMLPart *m_part;
  QValueList<QString> filesToBenchmark;
  QMap<QString, QValueList<int> > results;
  int                 benchmarkRun;
  QTime               loadTimer;

  void nextRun();
};

#endif
