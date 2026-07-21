#ifndef SHIP_DESTROYED_POPUP_H
#define SHIP_DESTROYED_POPUP_H

#include "gui/gui2_element.h"
#include "timer.h"


class GuiPanel;
class GuiCanvas;
class GuiOverlay;

class GuiShipDestroyedPopup : public GuiElement
{
private:
    GuiOverlay* ship_destroyed_overlay;
    GuiCanvas* owner;
    sp::SystemTimer show_timeout;
    sp::SystemTimer return_timeout;
    bool popup_visible = false;

public:
    GuiShipDestroyedPopup(GuiCanvas* owner);

    virtual void onUpdate() override;

private:
    void returnFromDestroyedPopup();
};

#endif//SHIP_DESTROYED_POPUP_H
