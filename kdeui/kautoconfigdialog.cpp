#include "kautoconfigdialog.h"
#include "kautoconfigdialog.moc"
#include "kautoconfig.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <qlayout.h>
#include <qvbox.h>

QAsciiDict<QObject> KAutoConfigDialog::openDialogs;

class KAutoConfigDialog::KAutoConfigDialogPrivate {

public:
  KAutoConfigDialogPrivate(KDialogBase::DialogType t) : shown(false), type(t){ }

  bool shown;
  KDialogBase::DialogType type;
};

KAutoConfigDialog::KAutoConfigDialog(QWidget *parent,const char *name,
		KDialogBase::DialogType dialogType, KConfig *kconfig, bool modal) :
		QObject(parent, name), d(new KAutoConfigDialogPrivate(dialogType)) {

  openDialogs.insert(name, this);
  kdialogbase = new KDialogBase( dialogType, i18n("Configure"), parent, name, modal,
	Qt::WStyle_DialogBorder | Qt::WDestructiveClose,
	KDialogBase::Default | KDialogBase::Ok | KDialogBase::Apply | KDialogBase::Cancel | KDialogBase::Help );

  if(!kconfig)
    kconfig = KGlobal::config();
  kautoconfig = new KAutoConfig(kconfig, kdialogbase, "kautoconfig");
  connectKAutoConfig(kautoconfig);

  connect(kdialogbase, SIGNAL(destroyed()), this, SLOT(deleteLater()));
  connect(kdialogbase, SIGNAL(okClicked()), this, SIGNAL(okClicked()));
  connect(kdialogbase, SIGNAL(applyClicked()), this, SIGNAL(applyClicked()));
  connect(kdialogbase, SIGNAL(defaultClicked()), this, SIGNAL(defaultClicked()));

  kdialogbase->enableButton(KDialogBase::Apply, false);
}

KAutoConfigDialog::~KAutoConfigDialog(){
  openDialogs.remove(name());
  delete d;
}

void KAutoConfigDialog::connectKAutoConfig( KAutoConfig *kautoconfig ){
  connect(kautoconfig, SIGNAL(settingsChanged()), this, SIGNAL(settingsChanged()));
  connect(kautoconfig, SIGNAL(settingsChanged()), this, SLOT(settingsChangedSlot()));
  connect(kautoconfig, SIGNAL(widgetModified()), this, SLOT(settingModified()));

  connect(kdialogbase, SIGNAL(okClicked()), kautoconfig, SLOT(saveSettings()));
  connect(kdialogbase, SIGNAL(applyClicked()), kautoconfig, SLOT(saveSettings()));
  connect(kdialogbase, SIGNAL(defaultClicked()), kautoconfig, SLOT(resetSettings()));
}

void KAutoConfigDialog::addPage(QWidget *page,
		                  const QString &itemName,
				  const QString &groupName,
		                  const QString &pixmapName,
				  const QString &header,
				  bool manage){
  if(d->shown){
    kdDebug() << "KAutoConfigDialog::addPage, can not a page after the dialog has been shown.";
    return;
  }
  switch(d->type){
    case KDialogBase::TreeList:
    case KDialogBase::IconList:
    case KDialogBase::Tabbed: {
      QString Header = header;
      if(Header.isEmpty())
        Header = itemName;
      QVBox *frame = kdialogbase->addVBoxPage(itemName, Header, SmallIcon(pixmapName, 32));
      frame->setSpacing( 0 );
      frame->setMargin( 0 );
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
      QVBoxLayout *topLayout = new QVBoxLayout( page, 0, 0 );
      page->reparent(((QWidget*)page), 0, QPoint());
      topLayout->addWidget( page );
      kdialogbase->setMainWidget(page);
    }
    break;

    default:
      kdDebug() << "KAutoConfigDialog::addWidget" << " unknown type.";
  }
  if(manage)
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

void KAutoConfigDialog::settingsChangedSlot(){
  emit (settingsChanged(name()));
}

void KAutoConfigDialog::setCaption(const QString &caption){
  kdialogbase->setCaption(caption);
}

void KAutoConfigDialog::show(bool track){
  if(!d->shown){
    kdialogbase->enableButton(KDialogBase::Default, kautoconfig->retrieveSettings(track));
    d->shown = true;
    if(!track){
      kdialogbase->enableButton(KDialogBase::Apply, true);
      kdialogbase->enableButton(KDialogBase::Default, true);
    }
  }
  kdialogbase->show();
}

