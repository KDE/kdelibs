#ifndef KAUTOCONFIGWIDGETS_H
#define KAUTOCONFIGWIDGETS_H

/**
 * This is the basic widgets that are supported by kautoconfig
 * 
 * The macros are here to make it easy to update all of the widgets at one
 * time and make it wasy to add new widgets to this file.  Unless you have a
 * really really good way of getting rid of these macros* _don't_ touch them.
 * 
 * *without simply manually exanding the code!
 **/

#include "kautoconfig.h"
#include <kconfig.h>

#define retrieveConfig(get, set, read)\
    defaultValue = widget->get(); \
    widget->set(config->read(widgetName, defaultValue)); \

#define saveConfig(read, get) \
    if(config->read(widgetName, defaultValue) == widget->get()) \
      return false; \
    config->writeEntry(widgetName, widget->get()); \
    return true;

#define resetConfig(get, set) \
    if(defaultValue == widget->get()) \
      return false; \
    widget->set(defaultValue); \
    return true; 

#define newKAutoConfigWidget(name) \
  KAutoConfigWidget* makeKAutoConfigWidget(QWidget *w) const { \
    return (new name(config, w)); \
  };

#include <kcolorbutton.h>
#include <qcolor.h>
class KAutoConfig_KColorButton : public KAutoConfigWidget {
public:
  KAutoConfig_KColorButton( KConfig *kconfig=0, QWidget *w=0 ) : 
   KAutoConfigWidget(kconfig, w), widget((KColorButton*)w) {};
  
  void retrieveSettings(){ 
    defaultValue = widget->color(); 
    widget->setColor(config->readColorEntry(widgetName, &defaultValue)); 
  }
  
  bool saveSettings() const {
    if(config->readColorEntry(widgetName, &defaultValue) == widget->color()) 
      return false; 
    config->writeEntry(widgetName, widget->color()); 
    return true;
  }
	  
  bool resetSettings() const { resetConfig(color, setColor); };
  
  newKAutoConfigWidget(KAutoConfig_KColorButton);

private:
  KColorButton *widget;
  QColor defaultValue;
};

#include <qradiobutton.h>
class KAutoConfig_QRadioButton : public KAutoConfigWidget {
public:
  KAutoConfig_QRadioButton ( KConfig *kconfig=0, QWidget *w=0 ) : 
   KAutoConfigWidget(kconfig, w), widget((QRadioButton*)w), defaultValue(0){
     if(widget)
       widgetName = widget->parent()->name();
   };
  
  void retrieveSettings(){
    defaultValue = widget->isChecked();
    widget->setChecked((config->readEntry(widgetName, widget->name()) == 
			    widget->name())); 
  }
  
  bool saveSettings() const {
    if(config->readEntry(widgetName, widget->name()) == widget->name()) 
      return false; 
    config->writeEntry(widgetName, widget->name());
    return true;
  };

  bool resetSettings() const { resetConfig(isChecked, setChecked); };
  
  newKAutoConfigWidget(KAutoConfig_QRadioButton);

private:
  QRadioButton *widget;
  bool defaultValue;
};

#include <qcheckbox.h>
class KAutoConfig_QCheckBox : public KAutoConfigWidget {
public:
  KAutoConfig_QCheckBox( KConfig *kconfig=0, QWidget *w=0 ) : 
   KAutoConfigWidget(kconfig, w), widget((QCheckBox*)w), defaultValue(false){};
  
  void retrieveSettings(){ retrieveConfig(isChecked, setChecked, readBoolEntry); };
  
  bool saveSettings() const { saveConfig(readBoolEntry, isChecked); };

  bool resetSettings() const { resetConfig(isChecked, setChecked); };
  
  newKAutoConfigWidget(KAutoConfig_QCheckBox);

private:
  QCheckBox *widget;
  bool defaultValue;
};


#include <qspinbox.h>
class KAutoConfig_QSpinBox : public KAutoConfigWidget {
public:
  KAutoConfig_QSpinBox( KConfig *kconfig=0, QWidget *w=0 ) : 
   KAutoConfigWidget(kconfig, w), widget((QSpinBox*)w), defaultValue(0){};
  
  void retrieveSettings(){ retrieveConfig(value, setValue, readNumEntry); };
  
  bool saveSettings() const { saveConfig(readNumEntry, value); };

  bool resetSettings() const { resetConfig(value, setValue); };
  
  newKAutoConfigWidget(KAutoConfig_QSpinBox);

private:
  QSpinBox *widget;
  int defaultValue;
};

#include <qlineedit.h>
class KAutoConfig_QLineEdit : public KAutoConfigWidget {
public:
  KAutoConfig_QLineEdit( KConfig *kconfig=0, QWidget *w=0 ) : 
   KAutoConfigWidget(kconfig, w), widget((QLineEdit*)w), defaultValue(""){};
  
  void retrieveSettings(){ retrieveConfig(text, setText, readEntry); };
  
  bool saveSettings() const { saveConfig(readEntry, text); };

  bool resetSettings() const { resetConfig(text, setText); };
  
  newKAutoConfigWidget(KAutoConfig_QLineEdit);

private:
  QLineEdit *widget;
  QString defaultValue;
};

#include <qcombobox.h>
class KAutoConfig_QComboBox : public KAutoConfigWidget {
public:
  KAutoConfig_QComboBox ( KConfig *kconfig=0, QWidget *w=0 ) : 
   KAutoConfigWidget(kconfig, w), widget((QComboBox*)w), defaultValue(0){};
  
  void retrieveSettings(){ retrieveConfig(currentItem, setCurrentItem, readNumEntry); };
  
  bool saveSettings() const { saveConfig(readNumEntry, currentItem); };

  bool resetSettings() const { resetConfig(currentItem, setCurrentItem); };
  
  newKAutoConfigWidget(KAutoConfig_QComboBox);

private:
  QComboBox *widget;
  int defaultValue;
};

#include <qslider.h>
class KAutoConfig_QSlider : public KAutoConfigWidget {
public:
  KAutoConfig_QSlider( KConfig *kconfig=0, QWidget *w=0 ) : 
   KAutoConfigWidget(kconfig, w), widget((QSlider*)w), defaultValue(0){};
  
  void retrieveSettings(){ retrieveConfig(value, setValue, readNumEntry); };
  
  bool saveSettings() const { saveConfig(readNumEntry, value); };

  bool resetSettings() const { resetConfig(value, setValue); };
  
  newKAutoConfigWidget(KAutoConfig_QSlider);

private:
  QSlider *widget;
  int defaultValue;
};

#endif

