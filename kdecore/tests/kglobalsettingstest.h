#ifndef kglobalsettingstest_h
#define kglobalsettingstest_h

#include <qobject.h>

class MyObject: public QObject
{
    Q_OBJECT
public:
    MyObject();

public Q_SLOTS:
    void slotPaletteChanged() { printf("SIGNAL: Palette changed\n"); }
    void slotStyleChanged() { printf("SIGNAL: Style changed\n"); }
    void slotFontChanged() { printf("SIGNAL: Font changed\n"); }
    void slotSettingsChanged(int i) { printf("SIGNAL: Settings %d changed\n", i); }
    void slotAppearanceChanged() { printf("SIGNAL: Appearance changed\n"); }
};

#endif
