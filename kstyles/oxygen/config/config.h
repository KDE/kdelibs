#ifndef CONFIG_H
#define CONFIG_H

#include "ui_config.h"
#include <QDialog>

class Config : public QWidget
{
   Q_OBJECT
public:
   Config(QWidget *parent = 0L);
protected:
   bool eventFilter ( QObject * watched, QEvent * event );
private:
   Ui::Config ui;
   bool infoItemHovered;
   void generateColorModes(QComboBox *box);
   void generateGradientTypes(QComboBox *box);
   void loadSettings();
signals:
   void changed(bool);
public slots:
   void save();
   void defaults();
   void reset();
private slots:
   void checkDirty();
   void resetInfo();
   void setBgModeInfo(int index);
   void setTabTransInfo(int index);
};

class ConfigDialog : public QDialog {
   Q_OBJECT
public:
   ConfigDialog(QWidget *parent = 0L);
};

#endif
