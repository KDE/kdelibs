#include "kautoconfigdialog.h"
#include "kautoconfig.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <qlayout.h>

QAsciiDict<QObject> KAutoConfigDialog::openDialogs;

KAutoConfigDialog::KAutoConfigDialog(QWidget *parent,const char *name,
		KDialogBase::DialogType dialogType, bool modal) :
		QObject(parent, name), shown(false), type(dialogType) {

  openDialogs.insert(name, this);
  kdialogbase = new KDialogBase( dialogType, i18n("Configure"), parent, name, modal,
	Qt::WStyle_DialogBorder | Qt::WDestructiveClose,
	KDialogBase::Default | KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel);

  kautoconfig = new KAutoConfig(kdialogbase, "kautoconfig");
  connect(kautoconfig, SIGNAL(settingsChanged()), this, SIGNAL(settingsChanged()));
  connect(kautoconfig, SIGNAL(widgetModified()), this, SLOT(settingModified()));
 
  connect(kdialogbase, SIGNAL(destroyed()), this, SLOT(deleteLater()));
  connect(kdialogbase, SIGNAL(okClicked()), kautoconfig, SLOT(saveSettings()));
  connect(kdialogbase, SIGNAL(applyClicked()), kautoconfig, SLOT(saveSettings()));
  connect(kdialogbase, SIGNAL(defaultClicked()), kautoconfig, SLOT(resetSettings()));

  kdialogbase->enableButton(KDialogBase::Apply, false);
}

KAutoConfigDialog::~KAutoConfigDialog(){
  openDialogs.remove(name());
}
  
void KAutoConfigDialog::addPage(QWidget *page,
		                  const QString &itemName,
				  const QString &groupName,
		                  const QString &pixmapName,
				  const QString &header){
  if(shown){
    kdDebug() << "KAutoConfigDialog::addPage, can not a page after the dialog has been shown.";
    return;
  }
  switch(type){
    case KDialogBase::TreeList:
    case KDialogBase::IconList:
    case KDialogBase::Tabbed: {
      QString Header = header;
      if(Header.isEmpty())
        Header = itemName;
      QVBox *frame = kdialogbase->addVBoxPage(itemName, Header, SmallIcon(pixmapName, 32));
      page->reparent(((QWidget*)frame), 0, QPoint());
    }
    break;
    
    case KDialogBase::Swallow: {
      page->reparent(((QWidget*)kdialogbase), 0, QPoint());
      kdialogbase->setMainWidget(page);
    }
    break;
    
    case KDialogBase::Plain:{
      page->reparent(((QWidget*)kdialogbase), 0, QPoint());
      QFrame *page = kdialogbase->plainPage();
      QVBoxLayout *topLayout = new QVBoxLayout( page, 0, 6 );
      page->reparent(((QWidget*)page), 0, QPoint());
      topLayout->addWidget( page );
      kdialogbase->setMainWidget(page);
    }
    break;
    
    default:
      kdDebug() << "KAutoConfigDialog::addWidget" << " unknown type.";
  }
  kautoconfig->addWidget(page, groupName);
}

KAutoConfigDialog* KAutoConfigDialog::exists(const char* name){
  return ((KAutoConfigDialog*)openDialogs.find(name));
}

bool KAutoConfigDialog::showDialog(const char* name){
  KAutoConfigDialog *dialog = exists(name);
  if(dialog)
    dialog->show();
  return (dialog != NULL);
}

void KAutoConfigDialog::settingModified(){
  kdialogbase->enableButton(KDialogBase::Apply, kautoconfig->hasChanged());
  kdialogbase->enableButton(KDialogBase::Default, !kautoconfig->isDefault());
}


void KAutoConfigDialog::show(){
  if(!shown){
    kdialogbase->enableButton(KDialogBase::Default, kautoconfig->retrieveSettings(true));
    shown = true;
  }
  kdialogbase->show();
}

