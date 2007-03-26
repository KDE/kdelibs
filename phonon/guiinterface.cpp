#include "guiinterface.h"
#include "kiofallback.h"

namespace Phonon
{

GuiInterface *_kde_internal_GuiInterface;
GuiInterface *GuiInterface::s_instance = 0;

GuiInterface *GuiInterface::instance()
{
    if (_kde_internal_GuiInterface) {
        return _kde_internal_GuiInterface;
    }
    if (!s_instance) {
        s_instance = new GuiInterface();
    }
    return s_instance;
}

GuiInterface::~GuiInterface()
{
    if (s_instance == this) {
        s_instance = 0;
    }
}

void GuiInterface::notification(const char *notificationName, const QString &text,
        const QStringList &actions, QObject *receiver, const char *actionSlot)
{
    Q_UNUSED(notificationName);
    Q_UNUSED(text);
    Q_UNUSED(actions);
    Q_UNUSED(receiver);
    Q_UNUSED(actionSlot);
}

KioFallback *GuiInterface::newKioFallback(MediaObject *parent)
{
    return 0;
}

} // namespace Phonon
// vim: sw=4 sts=4 et tw=100
