
#include "config.h"
#include "config.moc"
#include <QApplication>
#include <QSettings>
#include <QTimer>

typedef struct Info {
   char *oxygen;
   char *bgMode[9];
   char *roleButton, *roleProgress, *roleTab, *rolePopup;
   char *tabTrans[11];
   char *inverseButtons, *acceleration;
} Info;
static const Info info = {
   "<div align=\"center\">\
      <img src=\":/oxygen.png\"/><br>\
   </div>\
   <b>Oxygen Style</b><hr>\
   &copy;&nbsp;2006/2007 by Thomas L&uuml;bking<br>\
   Design by\
   <ul type=\"disc\">\
      <li>Nuno Pinheiro</li>\
      <li>David Vignoni</li>\
      <li>Kenneth Wimer</li>\
   </ul>\
   <hr>\
   Visit <a href=\"http://www.oxygen-icons.org\">www.oxygen-icons.org</a>",
   {
      "<b>Plain (color)</b><hr>Select if you have a really lousy machine or just hate structured backgrounds.",
      "<b>Scanlines</b><hr>Wanna Aqua feeling?",
      "Don't choose for the moment - falls back to plain",
      "<b>Complex</b><hr>Several light gradients covering the whole window.<br>\
      This can be VRAM and CPU hungry - better have GPU Hardware acceleration.",
      "<b>Vertical Top/Bottom Gradient</b><hr>Simple gradient that brightens on the upper and darkens on the lower end<br>(cheap, fallback suggestion 1)",
      "<b>Horizontal Left/Right Gradient</b><hr>Simple gradient that darkens on left and right side.",
      "<b>Vertical Center Gradient</b><hr>The window vertically brightens to the center",
      "<b>Horizontal Center Gradient</b><hr>The window horizontally brightens to the center (similar to Apples Brushed Metal, less cheap, fallback suggestion 2)",
      "<b>Glass</b><hr>Like the glass buttons, works best with dark colors"
   },
   "<b>Button Color Role</b><hr>\
   <p>\
      Set the buttons background color role.<br>\
      The button label role is chosen automatically.\
   </p><p>\
      It's strongly suggested to choose <b>Button</b> for at least either the\
      hovered or unhovered buttons.<br>\
      This role will usually be used to stress special buttons.\
   </p>",
   "<b>ProgressBar Roles</b><hr>\
   Just as you like, but you should use different roles for the Progress and\
   the Groove to make it visible ;)",
   "<b>Tabbar Role</b><hr>\
   The color of the not active tabs (the active one is allways part of the window)<br>\
   Text color is chosen automatically",
   "<b>Popup Menus</b><hr>\
   The background of rightclick and menubar menus as well as of non editable\
   combobox dropdowns.<br>\
   Text color is chosen automatically",
   {
      "<b>Jump</b><hr>No transition at all - fastest but looks stupid",
      "<b>CrossFade</b><hr>What you would expect - one fades out while the other fades in.<br>\
      This is CPU hungry - better have GPU Hardware acceleration.",
      "<b>ScanlineBlend</b><hr>Similar to CrossFade, but won't require Hardware acceleration.",
      "<b>SlideIn</b><hr>The new tab falls down from top",
      "<b>SlideOut</b><hr>The new tab rises from bottom",
      "<b>RollIn</b><hr>The new tab appears from Top/Bottom to center",
      "<b>RollOut</b><hr>The new tab appears from Center to Top/Bottom",
      "<b>OpenVertically</b><hr>The <b>old</b> Tab slides <b>out</b< to Top and Bottom",
      "<b>CloseVertically</b><hr>The <b>new</b> Tab slides <b>in</b> from Top and Bottom",
      "<b>OpenHorizontally</b><hr>The <b>old</b> Tab slides <b>out</b< to Left and Right",
      "<b>CloseHorizontally</b><hr>The <b>new</b> Tab slides <b>in</b> from Left and Right"
   },
   "<b>Inverse Pushbuttons</b><hr>The Buttons appear sunken by default.<br><b>Notice:</b> This currently makes toggled buttons appear like not toggled ones - use with care!",
      "<b>Acceleration</b><hr>Use XRender for the moment, the only one really usable implemented.<br>OpenGL needs a GPU supported Texture -> Pixmap functionm, unaccelerated ones require a lot of CPU.<p>If you wanna use the tiling mechanism, you MUST run the oxygenPP daemon (currently a qt3 app and i don't know if it will survive for the final release)</p>"
};

typedef struct Values {
   int bgMode, //3
      acceleration, // 2
      tabTransition, // 1
      checkMark; // 1
   bool inverseButtons, // false
      useGlass, // true
      showMenuIcons, // false!
      glassProgress, //false
      menuRuphy, //true ;)
      menuShadow; // false - i have a compmgr running :P
   QPalette::ColorRole crProgressBar, // Window
      crProgressBarGroove, // WindowText
      crTabBar, // WindowText
      crButtons, // Window
      crButtonsHover, // Button
      crPopup; // Window
} Values;
static const Values defaultValues = {
   3, 2, 1, 1,
   false, true, false, false, true, false,
   QPalette::Window,QPalette::WindowText, QPalette::WindowText,
      QPalette::Window, QPalette::Button, QPalette::Window
};

static Values initValues;

Config::Config(QWidget *parent) : QDialog(parent) {
   infoItemHovered = false;
   ui.setupUi(this);
   ui.info->setMinimumWidth( 160 );
   resize(640,-1);
   generateColorModes(ui.crProgressBar);
   generateColorModes(ui.crProgressBarGroove);
   generateColorModes(ui.crTabBar);
   generateColorModes(ui.crButtons);
   generateColorModes(ui.crButtonsHover);
   generateColorModes(ui.crPopup);
   loadSettings();
#define handleEvents(_ELEMENT_) ui._ELEMENT_->installEventFilter(this)
   handleEvents(info);
   handleEvents(bgMode);
   ((QWidget*)ui.bgMode->view())->installEventFilter(this);
   handleEvents(crButtons);
   handleEvents(crButtonsHover);
   handleEvents(crProgressBar);
   handleEvents(crProgressBarGroove);
   handleEvents(crTabBar);
   handleEvents(crPopup);
   handleEvents(inverseButtons);
   handleEvents(acceleration);
   handleEvents(tabTransition);
   connect(ui.buttonBox, SIGNAL(clicked ( QAbstractButton *)),
           this, SLOT(handleButton(QAbstractButton *)));
   connect(ui.bgMode, SIGNAL(highlighted(int)), this, SLOT(setBgModeInfo(int)));
   connect(ui.tabTransition, SIGNAL(highlighted(int)), this, SLOT(setTabTransInfo(int)));
   connect(ui.bgMode, SIGNAL(currentIndexChanged(int)), this, SLOT(bgModeSet(int)));
}

void Config::handleButton(QAbstractButton *btn) {
   QDialogButtonBox::StandardButton stdBtn = ui.buttonBox->standardButton(btn);
   switch (stdBtn) {
   case QDialogButtonBox::Ok:
   case QDialogButtonBox::Save:
   case QDialogButtonBox::Apply:
      save();
      if (stdBtn == QDialogButtonBox::Ok)
         accept();
      break;
   case QDialogButtonBox::Reset:
      resetValues();
      break;
   case QDialogButtonBox::RestoreDefaults:
      setDefaults();
      break;
   case QDialogButtonBox::Cancel:
      reject();
   default:
      break;
   }
}

void Config::resetValues() {
#define SET(_ELEMENT_) ui._ELEMENT_->setCurrentIndex( initValues._ELEMENT_ )
   SET(bgMode);
   SET(acceleration);
   SET(tabTransition);
   SET(checkMark);
#undef SET
#define SET(_ELEMENT_) ui._ELEMENT_->setCurrentIndex( ui._ELEMENT_->findData( initValues._ELEMENT_ ))
   SET(crProgressBar);
   SET(crProgressBarGroove);
   SET(crTabBar);
   SET(crButtons);
   SET(crButtonsHover);
   SET(crPopup);
#undef SET
#define SET(_ELEMENT_) ui._ELEMENT_->setChecked( initValues._ELEMENT_ )
   SET(inverseButtons);
   SET(useGlass);
   SET(showMenuIcons);
   SET(glassProgress);
   SET(menuRuphy);
   SET(menuShadow);
#undef SET
}

void Config::setDefaults() {
#define SET(_ELEMENT_) ui._ELEMENT_->setCurrentIndex( defaultValues._ELEMENT_ )
   SET(bgMode);
   SET(acceleration);
   SET(tabTransition);
   SET(checkMark);
#undef SET
#define SET(_ELEMENT_) ui._ELEMENT_->setCurrentIndex( ui._ELEMENT_->findData( defaultValues._ELEMENT_ ))
   SET(crProgressBar);
   SET(crProgressBarGroove);
   SET(crTabBar);
   SET(crButtons);
   SET(crButtonsHover);
   SET(crPopup);
#undef SET
#define SET(_ELEMENT_) ui._ELEMENT_->setChecked( defaultValues._ELEMENT_ )
   SET(inverseButtons);
   SET(useGlass);
   SET(showMenuIcons);
   SET(glassProgress);
   SET(menuRuphy);
   SET(menuShadow);
#undef SET
}

void Config::bgModeSet(int bgMode) {
   ui.acceleration->setEnabled(bgMode == 3);
}

void Config::setBgModeInfo(int index) {
   infoItemHovered = true;
   ui.info->setHtml(info.bgMode[index]);
}

void Config::setTabTransInfo(int index) {
   infoItemHovered = true;
   ui.info->setHtml(info.tabTrans[index]);
}

void Config::resetInfo() {
   if (!infoItemHovered)
      ui.info->setHtml(info.oxygen);
}

void Config::loadSettings() {
   QSettings settings("Oxygen", "Style");
   settings.beginGroup("Style");
   ui.info->setHtml(info.oxygen);
   
#define readInt(_ENTRY_, _ELEMENT_)\
   initValues._ELEMENT_ = settings.value( _ENTRY_, defaultValues._ELEMENT_).toInt();\
   ui._ELEMENT_->setCurrentIndex( initValues._ELEMENT_ )
   readInt("BackgroundMode", bgMode);
   readInt("Acceleration", acceleration);
   readInt("TabTransition", tabTransition);
   readInt("CheckType", checkMark);
   
#define readBool(_ENTRY_, _ELEMENT_)\
   initValues._ELEMENT_ = settings.value( _ENTRY_, defaultValues._ELEMENT_).toBool();\
   ui._ELEMENT_->setChecked( initValues._ELEMENT_ )
   readBool("InverseButtons", inverseButtons);
   readBool("Aqua", useGlass);
   readBool("ShowMenuIcons", showMenuIcons);
   readBool("GlassProgress", glassProgress);
   readBool("RuphysMenu", menuRuphy);
   readBool("MenuShadow", menuShadow);

#define readColor(_ENTRY_, _ELEMENT_)\
   initValues._ELEMENT_ = (QPalette::ColorRole)settings.value( _ENTRY_, defaultValues._ELEMENT_).toInt();\
   ui._ELEMENT_->setCurrentIndex( ui._ELEMENT_->findData( initValues._ELEMENT_ ))
   readColor("role_progress", crProgressBar);
   readColor("role_progressGroove", crProgressBarGroove);
   readColor("role_tab", crTabBar);
   readColor("role_button", crButtons);
   readColor("role_buttonHover", crButtonsHover);
   readColor("role_popup", crPopup);
   settings.endGroup();
}

void Config::save() {
   QSettings settings("Oxygen", "Style");
   settings.beginGroup("Style");
   settings.setValue("BackgroundMode", ui.bgMode->currentIndex());
   settings.setValue("Acceleration", ui.acceleration->currentIndex());
   settings.setValue("TabTransition", ui.tabTransition->currentIndex());
   
#define saveBool(_ENTRY_, _ELEMENT_) settings.setValue(_ENTRY_, ui._ELEMENT_->isChecked());
   saveBool("InverseButtons", inverseButtons);
   saveBool("Aqua", useGlass);
   saveBool("ShowMenuIcons", showMenuIcons);
   saveBool("GlassProgress", glassProgress);
   saveBool("RuphysMenu", menuRuphy);
   saveBool("MenuShadow", menuShadow);
   
#define saveColor(_ENTRY_, _ELEMENT_) settings.setValue(_ENTRY_,\
                     ui._ELEMENT_->itemData(ui._ELEMENT_->currentIndex()))
   saveColor("role_progress", crProgressBar);
   saveColor("role_progressGroove", crProgressBarGroove);
   saveColor("role_tab", crTabBar);
   saveColor("role_button", crButtons);
   saveColor("role_buttonHover", crButtonsHover);
   saveColor("role_popup", crPopup);
   settings.endGroup();
}

void Config::generateColorModes(QComboBox *box) {
   box->clear();
   box->addItem("Window", QPalette::Window);
   box->addItem("Window Text", QPalette::WindowText);
   box->addItem("Base (text editor)", QPalette::Base);
   box->addItem("Text (text editor)", QPalette::Text);
   box->addItem("Button", QPalette::Button);
   box->addItem("Button Text", QPalette::ButtonText);
   box->addItem("Highlight", QPalette::Highlight);
   box->addItem("Highlighted Text", QPalette::HighlightedText);
}

bool Config::eventFilter ( QObject * o, QEvent * e) {
   if (e->type() == QEvent::Enter) {
      infoItemHovered = true;
      if (o == ui.crButtons || o == ui.crButtonsHover)
         ui.info->setHtml(info.roleButton);
      else if (o == ui.bgMode)
         ui.info->setHtml("<b>Background Mode</b><hr>More info on the dropdown...");
      else if (o == ui.tabTransition)
         ui.info->setHtml("<b>Tab Transition</b><hr>More info on the dropdown...");
      else if (o == ui.inverseButtons)
         ui.info->setHtml(info.inverseButtons);
      else if (o == ui.acceleration)
         ui.info->setHtml(info.acceleration);
      else if (o == ui.crProgressBar || o == ui.crProgressBarGroove)
         ui.info->setHtml(info.roleProgress);
      else if (o == ui.crTabBar)
         ui.info->setHtml(info.roleTab);
      else if (o == ui.crPopup)
         ui.info->setHtml(info.rolePopup);
      else if (o != ui.info)
         infoItemHovered = false;
      return false;
   }
   else if (e->type() == QEvent::Leave) {
      infoItemHovered = false;
      QTimer::singleShot(300, this, SLOT(resetInfo()));
      return false;
   }
   return false;
}


int main(int argc, char *argv[])
{
   QApplication app(argc, argv);
   Config *window = new Config;
   window->show();
   return app.exec();
}
