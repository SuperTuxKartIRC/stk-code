//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 Marc Coll
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "states_screens/edit_track_screen.hpp"

#include "graphics/stk_tex_manager.hpp"

#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace irr::video;
using namespace GUIEngine;
using namespace irr::core;

const char* EditTrackScreen::ALL_TRACKS_GROUP_ID = "all";

// -----------------------------------------------------------------------------
EditTrackScreen::EditTrackScreen()
    : Screen("edit_track.stkgui"), m_track_group("standard"),
    m_track(NULL), m_laps(0), m_reverse(false), m_powerup(false), m_nitro(false), m_banana(false), m_result(false)
{

}

// -----------------------------------------------------------------------------
EditTrackScreen::~EditTrackScreen()
{

}

// -----------------------------------------------------------------------------
void EditTrackScreen::setSelection(Track* track, unsigned int laps, bool reverse, bool powerup, bool nitro, bool banana)
{
    assert(laps > 0);
    m_track = track;
    m_laps = laps;
    m_reverse = reverse;
    m_powerup = powerup;
    m_nitro = nitro;
    m_banana = banana;
}

// -----------------------------------------------------------------------------
Track* EditTrackScreen::getTrack() const
{
    return m_track;
}

// -----------------------------------------------------------------------------
unsigned int EditTrackScreen::getLaps() const
{
    return m_laps;
}

// -----------------------------------------------------------------------------
bool EditTrackScreen::getReverse() const
{
    return m_reverse;
}

// -----------------------------------------------------------------------------
bool EditTrackScreen::getPowerup() const
{
    return m_powerup;
}

// -----------------------------------------------------------------------------
bool EditTrackScreen::getNitro() const
{
    return m_nitro;
}

// -----------------------------------------------------------------------------
bool EditTrackScreen::getBanana() const
{
    return m_banana;
}

// -----------------------------------------------------------------------------
bool EditTrackScreen::getResult() const
{
    return m_result;
}

// -----------------------------------------------------------------------------
void EditTrackScreen::loadedFromFile()
{
    static const int MAX_LABEL_LENGTH = 35;

    DynamicRibbonWidget* tracks_widget = getWidget<DynamicRibbonWidget>("tracks");
    assert(tracks_widget != NULL);
    tracks_widget->setMaxLabelLength(MAX_LABEL_LENGTH);

    m_screenshot = getWidget<IconButtonWidget>("screenshot");
    m_screenshot->setFocusable(false);
    m_screenshot->m_tab_stop = false;
}

// -----------------------------------------------------------------------------
void EditTrackScreen::beforeAddingWidget()
{
    RibbonWidget* tabs = getWidget<RibbonWidget>("trackgroups");
    assert (tabs != NULL);

    tabs->clearAllChildren();

    const std::vector<std::string>& groups = track_manager->getAllTrackGroups();
    if (groups.size() > 1)
    {
        tabs->addTextChild(_("All"), ALL_TRACKS_GROUP_ID);
        for (unsigned int i = 0; i < groups.size(); i++)
            tabs->addTextChild(_(groups[i].c_str()), groups[i]);
    }
}

// -----------------------------------------------------------------------------
void EditTrackScreen::init()
{
    RibbonWidget* tabs = getWidget<RibbonWidget>("trackgroups");
    assert (tabs != NULL);
    SpinnerWidget* laps = getWidget<SpinnerWidget>("laps");
    assert(laps != NULL);
    CheckBoxWidget* reverse = getWidget<CheckBoxWidget>("reverse");
    assert(reverse != NULL);
    IconButtonWidget* powerup = getWidget<IconButtonWidget>("powerup");
    assert(powerup != NULL);
    IconButtonWidget* nitro = getWidget<IconButtonWidget>("nitro");
    assert(nitro != NULL);
    IconButtonWidget* banana = getWidget<IconButtonWidget>("banana");
    assert(banana != NULL);

    if (m_track_group.empty())
        tabs->select (ALL_TRACKS_GROUP_ID, PLAYER_ID_GAME_MASTER);
    else
        tabs->select (m_track_group, PLAYER_ID_GAME_MASTER);
    laps->setValue(m_laps);
    reverse->setState(m_reverse);
    powerup->setState(m_powerup);
    nitro->setState(m_nitro);
    banana->setState(m_banana);

    loadTrackList();
    if (m_track == NULL)
        selectTrack("");
    else
        selectTrack(m_track->getIdent());
}

// -----------------------------------------------------------------------------
void EditTrackScreen::eventCallback(GUIEngine::Widget* widget, const std::string& name,
    const int playerID)
{
    if (name == "ok")
    {
        m_result = true;
        StateManager::get()->popMenu();
    }
    else if (name == "cancel")
    {
        m_result = false;
        StateManager::get()->popMenu();
    }
    else if (name == "tracks")
    {
        DynamicRibbonWidget* tracks = getWidget<DynamicRibbonWidget>("tracks");
        assert(tracks != NULL);
        selectTrack(tracks->getSelectionIDString(PLAYER_ID_GAME_MASTER));
    }
    else if (name == "trackgroups")
    {
        RibbonWidget* tabs = getWidget<RibbonWidget>("trackgroups");
        assert(tabs != NULL);
        m_track_group = tabs->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        loadTrackList();
    }
    else if (name == "laps")
    {
        SpinnerWidget* laps = getWidget<SpinnerWidget>("laps");
        assert(laps != NULL);
        m_laps = laps->getValue();
    }
    else if (name == "reverse")
    {
        CheckBoxWidget* reverse = getWidget<CheckBoxWidget>("reverse");
        assert(reverse != NULL);
        m_reverse = reverse->getState();
    }
    else if (name == "powerup")
    {
        IconButtonWidget* powerup = getWidget<IconButtonWidget>("powerup");
        assert(powerup != NULL);

        powerup->setTooltip("Disable power-ups");
        RaceManager::get()->setPowerupTrack(powerup->getState());
        powerup = changeIconButtonImage(powerup, "gift");
        m_powerup = powerup->getState();
    }
    else if (name == "nitro")
    {
        IconButtonWidget* nitro = getWidget<IconButtonWidget>("nitro");
        assert(nitro != NULL);

        nitro->setTooltip("Disable nitro");
        RaceManager::get()->setNitroTrack(nitro->getState());
        nitro = changeIconButtonImage(nitro, "nitro");
        m_nitro = nitro->getState();
    }
    else if (name == "banana")
    {
        IconButtonWidget* banana = getWidget<IconButtonWidget>("banana");
        assert(banana != NULL);

        banana->setTooltip("Disable banana");
        RaceManager::get()->setBananaTrack(banana->getState());
        banana = changeIconButtonImage(banana, "banana");
        m_banana = banana->getState();
    }
}

// -----------------------------------------------------------------------------
void EditTrackScreen::loadTrackList()
{
    DynamicRibbonWidget* tracks_widget = getWidget<DynamicRibbonWidget>("tracks");
    assert(tracks_widget != NULL);

    tracks_widget->clearItems();

    for (unsigned int i = 0; i < track_manager->getNumberOfTracks(); i++)
    {
        Track* t = track_manager->getTrack(i);
        bool belongs_to_group = (m_track_group.empty()                ||
                          m_track_group == ALL_TRACKS_GROUP_ID ||
                          t->isInGroup(m_track_group)                );
        if (!t->isArena()    && !t->isSoccer() &&
            !t->isInternal() && belongs_to_group       )
        {
            tracks_widget->addItem(t->getName(),
                                   t->getIdent(),
                                   t->getScreenshotFile(), 0,
                                   IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
        }
    }

    tracks_widget->updateItemDisplay();
}

// -----------------------------------------------------------------------------
void EditTrackScreen::selectTrack(const std::string& id)
{
    DynamicRibbonWidget* tracks = getWidget<DynamicRibbonWidget>("tracks");
    assert(tracks != NULL);
    LabelWidget* selected_track = getWidget<LabelWidget>("selected_track");
    assert(selected_track != NULL);
    SpinnerWidget* laps = getWidget<SpinnerWidget>("laps");
    assert(laps != NULL);
    LabelWidget* label_reverse = getWidget<LabelWidget>("reverse_label");
    assert(label_reverse != NULL);
    CheckBoxWidget* reverse = getWidget<CheckBoxWidget>("reverse");
    assert(reverse != NULL);
    IconButtonWidget* powerup = getWidget<IconButtonWidget>("powerup");
    powerup = setIconButtonImage(powerup, "gift");
    assert(powerup != NULL);
    IconButtonWidget* nitro = getWidget<IconButtonWidget>("nitro");
    nitro = setIconButtonImage(nitro, "nitro");
    assert(nitro != NULL);
    IconButtonWidget* banana = getWidget<IconButtonWidget>("banana");
    banana = setIconButtonImage(banana, "banana");
    assert(banana != NULL);
    ButtonWidget* ok_button = getWidget<ButtonWidget>("ok");
    assert(ok_button != NULL);



    m_track = track_manager->getTrack(id);
    ok_button->setActive(m_track!=NULL);
    if (m_track)
    {
        tracks->setSelection(m_track->getIdent(), PLAYER_ID_GAME_MASTER, true);
        selected_track->setText(m_track->getName(), true);

        m_laps = m_track->getDefaultNumberOfLaps();
        laps->setValue(m_laps);

        reverse->setVisible(m_track->reverseAvailable());
        label_reverse->setVisible(m_track->reverseAvailable());

        powerup->setVisible(m_track->powerupAvailable());
        nitro->setVisible(m_track->nitroAvailable());
        banana->setVisible(m_track->bananaAvailable());

        // Display the track's preview picture in a box,
        // so that the current selection remains obvious even
        // if the player doesn't notice the track name in title
        irr::video::ITexture* image = STKTexManager::getInstance()
            ->getTexture(m_track->getScreenshotFile(),
            "While loading screenshot for track '%s':", m_track->getFilename());
        if(!image)
        {
            image = STKTexManager::getInstance()->getTexture(GUIEngine::getSkin()->getThemedIcon("gui/icons/track_unknown.png"),
                "While loading screenshot for track '%s':", m_track->getFilename());
        }
        if (image != NULL)
            m_screenshot->setImage(image);
    }
    else
    {
        tracks->setSelection("", PLAYER_ID_GAME_MASTER, true);
        selected_track->setText(_("Select a track"), true);

        // We can't set a better default for number of laps. On the other
        // hand, if a track is selected, the number of laps will be updated.
        laps->setValue(3);

        reverse->setVisible(true);
        reverse->setState(false);

        powerup->setVisible(true);
        powerup->setState(true);
        nitro->setVisible(true);
        nitro->setState(true);
        banana->setVisible(true);
        banana->setState(true);
    }
}

IconButtonWidget* EditTrackScreen::changeIconButtonImage(IconButtonWidget* iconButton, std::string name)
{
    ITexture* image;

    if (iconButton->getState() == true) {
        iconButton->setState(false);
        image = STKTexManager::getInstance()->getTexture(GUIEngine::getSkin()->getThemedIcon("gui/icons/no_" + name + ".png"));
    }
    else {
        iconButton->setState(true);
        image = STKTexManager::getInstance()->getTexture(GUIEngine::getSkin()->getThemedIcon(("gui/icons/" + name + ".png")));
    }

    if (image != NULL)
        iconButton->setImage(image);

    return iconButton;
}

GUIEngine::IconButtonWidget* EditTrackScreen::setIconButtonImage(GUIEngine::IconButtonWidget* iconButton, std::string name)
{
    ITexture* image;

    if (iconButton->getState() == true) {
        image = STKTexManager::getInstance()->getTexture(GUIEngine::getSkin()->getThemedIcon(("gui/icons/" + name + ".png")));
    }
    else {
        image = STKTexManager::getInstance()->getTexture(GUIEngine::getSkin()->getThemedIcon("gui/icons/no_" + name + ".png"));
    }

    if (image != NULL)
        iconButton->setImage(image);

    return iconButton;
}
