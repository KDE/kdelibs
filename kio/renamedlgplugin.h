

#ifndef renamedlgplugin_h
#define renamedlgplugin_h

#include <kio/renamedlg.h>
#include <qdialog.h>
#include <sys/types.h>
#include <qstring.h>
#include <qstringlist.h>

/** This is the base class for all plugins. 
 *  
 *
 */
class RenameDlgPlugin : public QWidget
{
public:
  /**This is the c'tor
   * 
   */
  RenameDlgPlugin(QDialog *dialog, const char *name, const QStringList &/*list*/ = QStringList() ): QWidget(dialog, name ) {};

  /** This function will be called by RenameDlg. The params are infos about the files
   *  @return If the plugin want's to display it return true if not return false
   */
  bool initialize(KIO::RenameDlg_Mode /*mod*/, const QString &/*_src*/, const QString &/*_dest*/,
		  const QString &/*mimeSrc*/,
		  const QString &/*mimeDest*/,
		  KIO::filesize_t /*sizeSrc*/,
		  KIO::filesize_t /*sizeDest*/,
		  time_t /*ctimeSrc*/,
		  time_t /*ctimeDest*/,
		  time_t /*mtimeSrc*/,
		  time_t /*mtimeDest*/ ) {return false;};

};

#endif
