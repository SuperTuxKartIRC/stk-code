//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#include "states_screens/dialogs/high_score_info_dialog.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "guiengine/CGUISpriteBank.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "race/grand_prix_data.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/highscores.hpp"
#include "race/highscore_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/high_score_selection.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::core;

// -----------------------------------------------------------------------------
HighScoreInfoDialog::HighScoreInfoDialog(Highscores* highscore, bool is_linear, RaceManager::MajorRaceModeType major_mode)
                      : ModalDialog(0.80f,0.82f)
{
    m_hs = highscore;
    m_major_mode = major_mode;
    m_curr_time = 0.0f;

    loadFromFile("high_score_info_dialog.stkgui");

    m_track_screenshot_widget = getWidget<IconButtonWidget>("track_screenshot");
    m_track_screenshot_widget->setFocusable(false);
    m_track_screenshot_widget->m_tab_stop = false;

    // temporary icon, will replace it just after (but it will be shown if the given icon is not found)
    m_track_screenshot_widget->m_properties[PROP_ICON] = "gui/icons/main_help.png";

    Track* track;
    core::stringw track_name;
    core::stringw track_type_name;

    if (m_major_mode == RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        m_gp = *grand_prix_manager->getGrandPrix(m_hs->m_track);
        m_gp.checkConsistency();
        track = track_manager->getTrack(m_gp.getTrackId(0));
        track_name = m_gp.getName();
        track_type_name = _("Grand Prix");
        m_minor_mode = (RaceManager::MinorRaceModeType)m_hs->m_gp_minor_mode;
    }
    else
    {
        track = track_manager->getTrack(m_hs->m_track);
        track_name = track->getName();
        track_type_name = _("Track");
        m_minor_mode = HighScoreSelection::getInstance()->getActiveMode();
    }

    irr::video::ITexture* image = STKTexManager::getInstance()
        ->getTexture(track->getScreenshotFile(),
        "While loading screenshot for track '%s':", track->getFilename());
    if(!image)
    {
        image = STKTexManager::getInstance()->getTexture("main_help.png",
            "While loading screenshot for track '%s':", track->getFilename());
    }
    if (image != NULL)
        m_track_screenshot_widget->setImage(image);

    // TODO : small refinement, add the possibility to tab stops for lists
    //        to make this unselectable by keyboard/mouse
    m_high_score_list = getWidget<GUIEngine::ListWidget>("high_score_list");
    assert(m_high_score_list != NULL);
    m_high_score_info_list = getWidget<GUIEngine::ListWidget>("high_score_info_list");
    assert(m_high_score_info_list != NULL);

    /* Used to display kart icons for the entries */
    irr::gui::STKModifiedSpriteBank *icon_bank = HighScoreSelection::getInstance()->getIconBank();
    int icon_height = GUIEngine::getFontHeight() * 3 / 2;

    icon_bank->setScale(icon_height/128.0f);
    icon_bank->setTargetIconSize(128, 128);
    m_high_score_list->setIcons(icon_bank, (int)icon_height);

    updateHighscoreEntries();

    stringw is_reverse;
    stringw is_powerup;
    stringw is_nitro;
    stringw is_banana;
    std::vector<GUIEngine::ListWidget::ListCell> row;

    if (is_linear)
    {
        if (m_major_mode == RaceManager::MAJOR_MODE_GRAND_PRIX)
        {
            is_reverse = GrandPrixData::reverseTypeToString((GrandPrixData::GPReverseType)m_hs->m_gp_reverse_type);
            is_powerup = GrandPrixData::powerupTypeToString((GrandPrixData::GPPowerupType)m_hs->m_gp_powerup_type);
            is_nitro = GrandPrixData::nitroTypeToString((GrandPrixData::GPNitroType)m_hs->m_gp_nitro_type);
            is_banana = GrandPrixData::bananaTypeToString((GrandPrixData::GPBananaType)m_hs->m_gp_banana_type);
            //m_num_laps_label->setText(_("Game mode: %s", RaceManager::getNameOf(m_minor_mode)), true);
        }
        else
        {
            is_reverse = m_hs->m_reverse ? _("Yes") : _("No");
            is_powerup = m_hs->m_powerup ? _("Yes") : _("No");
            is_nitro = m_hs->m_nitro ? _("Yes") : _("No");
            is_banana = m_hs->m_banana ? _("Yes") : _("No");
        }

        row.push_back(GUIEngine::ListWidget::ListCell(_("Track name: %s", track_name), -1, 5, false));
        m_high_score_info_list->addItem(StringUtils::toString(1), row);
        row.clear();
        row.push_back(GUIEngine::ListWidget::ListCell(_("Difficulty: %s", m_hs->m_difficulty), -1, 5, false));
        m_high_score_info_list->addItem(StringUtils::toString(1), row);
        row.clear();
        row.push_back(GUIEngine::ListWidget::ListCell(_("Number of karts: %d", m_hs->m_number_of_karts), -1, 5, false));
        m_high_score_info_list->addItem(StringUtils::toString(1), row);
        row.clear();
        if (m_minor_mode == RaceManager::MINOR_MODE_LAP_TRIAL)
            row.push_back(GUIEngine::ListWidget::ListCell(_("Time target: %s", StringUtils::toWString(StringUtils::timeToString(m_hs->m_number_of_laps))), -1, 5, false));
        else
            row.push_back(GUIEngine::ListWidget::ListCell(_("Laps: %d", m_hs->m_number_of_laps), -1, 5, false));
        m_high_score_info_list->addItem(StringUtils::toString(1), row);
        row.clear();
        row.push_back(GUIEngine::ListWidget::ListCell(_("Reverse: %s", is_reverse), -1, 5, false));
        m_high_score_info_list->addItem(StringUtils::toString(1), row);
        row.clear();
        row.push_back(GUIEngine::ListWidget::ListCell(_("Power-ups: %s", is_powerup), -1 ,5, false));
        m_high_score_info_list->addItem(StringUtils::toString(1), row);
        row.clear();
        row.push_back(GUIEngine::ListWidget::ListCell(_("Nitro: %s", is_nitro), -1, 5, false));
        m_high_score_info_list->addItem(StringUtils::toString(1), row);
        row.clear();
        row.push_back(GUIEngine::ListWidget::ListCell(_("Banana: %s", is_banana), -1, 5, false));
        m_high_score_info_list->addItem(StringUtils::toString(1), row);
    }
    else
    {
        row.push_back(GUIEngine::ListWidget::ListCell(_("Track name: %s", track_name), -1, 5, false));
        m_high_score_info_list->addItem(StringUtils::toString(1), row);
        row.clear();

        row.push_back(GUIEngine::ListWidget::ListCell(_("Difficulty: %s", m_hs->m_difficulty), -1, 5, false));
        m_high_score_info_list->addItem(StringUtils::toString(1), row);
        row.clear();

        row.push_back(GUIEngine::ListWidget::ListCell(_("Power-ups: %s", is_powerup), -1, 5, false));
        m_high_score_info_list->addItem(StringUtils::toString(1), row);
        row.clear();

        row.push_back(GUIEngine::ListWidget::ListCell(_("Nitro: %s", is_nitro), -1, 5, false));
        m_high_score_info_list->addItem(StringUtils::toString(1), row);
        row.clear();

        row.push_back(GUIEngine::ListWidget::ListCell(_("Banana: %s", is_banana), -1, 5, false));
        m_high_score_info_list->addItem(StringUtils::toString(1), row);
    }

    m_start_widget = getWidget<IconButtonWidget>("start");

    if (m_major_mode == RaceManager::MAJOR_MODE_GRAND_PRIX)
        m_start_widget->setActive(!PlayerManager::getCurrentPlayer()->isLocked(m_gp.getId()));
    else
        m_start_widget->setActive(!PlayerManager::getCurrentPlayer()->isLocked(track->getIdent()));

    m_action_widget = getWidget<RibbonWidget>("actions");

    m_action_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    m_action_widget->select("back", PLAYER_ID_GAME_MASTER);
}   // HighScoreInfoDialog
// -----------------------------------------------------------------------------
HighScoreInfoDialog::~HighScoreInfoDialog()
{
}   // ~HighScoreInfoDialog

// -----------------------------------------------------------------------------
void HighScoreInfoDialog::updateHighscoreEntries()
{
    m_high_score_list->clear();

    const int amount = m_hs->getNumberEntries();

    std::string kart_name;
    core::stringw name;
    float time;

    int time_precision = RaceManager::get()->currentModeTimePrecision();

    // Fill highscore entries
    for (int n = 0; n < m_hs->HIGHSCORE_LEN; n++)
    {
        irr::core::stringw line;
        int icon = -1;

        // Check if this entry is filled or still empty
        if (n < amount)
        {
            m_hs->getEntry(n, kart_name, name, &time);

            std::string highscore_string;
            if (m_minor_mode == RaceManager::MINOR_MODE_LAP_TRIAL)
            {
                highscore_string = std::to_string(static_cast<int>(time));
            }
            else
            {
                if (time > 60.0f * 60.0f)
                {
                    highscore_string = StringUtils::timeToString(time, time_precision,
                        /*display_minutes_if_zero*/true, /*display_hours*/true);
                }
                else
                {
                    highscore_string = StringUtils::timeToString(time, time_precision);
                }
            }

            for(unsigned int i=0; i<kart_properties_manager->getNumberOfKarts(); i++)
            {
                const KartProperties* prop = kart_properties_manager->getKartById(i);
                if (kart_name == prop->getIdent())
                {
                    icon = i;
                    break;
                }
            }

            line = name + "    " + core::stringw(highscore_string.c_str());
        }
        else
        {
            //I18N: for empty highscores entries
            line = _("(Empty)");
        }

        if (icon == -1)
        {
            icon = HighScoreSelection::getInstance()->getUnknownKartIcon();
        }

        std::vector<GUIEngine::ListWidget::ListCell> row;
        row.push_back(GUIEngine::ListWidget::ListCell(line.c_str(), icon, 5, false));
        m_high_score_list->addItem(StringUtils::toString(n), row);
    }
} // updateHighscoreEntries

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation
    HighScoreInfoDialog::processEvent(const std::string& event_source)
{
    if (event_source == "actions")
    {
        const std::string& selection =
                m_action_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "start")
        {
            // Use the last used device
            InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();

            // Create player and associate player with device
            StateManager::get()->createActivePlayer(PlayerManager::getCurrentPlayer(), device);

            RaceManager::get()->setMinorMode(m_minor_mode);
            RaceManager::get()->setDifficulty((RaceManager::Difficulty)m_hs->m_difficulty);

            RaceManager::get()->setNumKarts(m_hs->m_number_of_karts);
            RaceManager::get()->setNumPlayers(1);

            if (kart_properties_manager->getKart(UserConfigParams::m_default_kart) == NULL)
            {
                Log::warn("HighScoreInfoDialog", "Cannot find kart '%s', will revert to default",
                    UserConfigParams::m_default_kart.c_str());
                UserConfigParams::m_default_kart.revertToDefaults();
            }
            RaceManager::get()->setPlayerKart(0, UserConfigParams::m_default_kart);

            // Disable accidentally unlocking of a challenge
            PlayerManager::getCurrentPlayer()->setCurrentChallenge("");

            // ASSIGN should make sure that only input from assigned devices is read
            input_manager->getDeviceManager()->setAssignMode(ASSIGN);
            input_manager->getDeviceManager()
                ->setSinglePlayer( StateManager::get()->getActivePlayer(0) );

            bool reverse = m_hs->m_reverse;
            GrandPrixData::GPReverseType gp_reverse = (GrandPrixData::GPReverseType)m_hs->m_gp_reverse_type;
            bool powerup = m_hs->m_powerup;
            GrandPrixData::GPPowerupType gp_powerup = (GrandPrixData::GPPowerupType)m_hs->m_gp_powerup_type;
            bool nitro = m_hs->m_nitro;
            GrandPrixData::GPNitroType gp_nitro = (GrandPrixData::GPNitroType)m_hs->m_gp_nitro_type;
            bool banana = m_hs->m_banana;
            GrandPrixData::GPBananaType gp_banana = (GrandPrixData::GPBananaType)m_hs->m_gp_banana_type;

            std::string track_name = m_hs->m_track;
            int laps = m_hs->m_number_of_laps;
            RaceManager::MajorRaceModeType major_mode = m_major_mode;

            if (RaceManager::get()->isLapTrialMode())
            {
                RaceManager::get()->setTimeTarget(static_cast<float>(m_hs->m_number_of_laps));
            }

            ModalDialog::dismiss();

            if (major_mode == RaceManager::MAJOR_MODE_GRAND_PRIX)
            {
                GrandPrixData gp = *grand_prix_manager->getGrandPrix(track_name);
                gp.changeReverse(gp_reverse);
                gp.changePowerup(gp_powerup);
                gp.changeNitro(gp_nitro);
                gp.changeBanana(gp_banana);
                RaceManager::get()->startGP(gp, false, false);
            }
            else
            {
                RaceManager::get()->setReverseTrack(reverse);
                RaceManager::get()->setPowerupTrack(powerup);
                RaceManager::get()->setNitroTrack(nitro);
                RaceManager::get()->setBananaTrack(banana);
                RaceManager::get()->startSingleRace(track_name, RaceManager::get()->isLapTrialMode() ? -1 : laps, false);
            }
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "remove")
        {
            ModalDialog::dismiss();

            dynamic_cast<HighScoreSelection*>(GUIEngine::getCurrentScreen())
                ->onDeleteHighscores();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "back")
        {
            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent

// ----------------------------------------------------------------------------
/** Called every update. Used to cycle the screenshots for grand prix entries.
 *  \param dt Time step size.
 */
void HighScoreInfoDialog::onUpdate(float dt)
{
    if (m_major_mode == RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        if (dt == 0)
            return; // if nothing changed, return right now

        m_curr_time += dt;
        int frame_after = (int)(m_curr_time / 1.5f);

        const std::vector<std::string> tracks = m_gp.getTrackNames();
        if (frame_after >= (int)tracks.size())
        {
            frame_after = 0;
            m_curr_time = 0;
        }

        Track* track = track_manager->getTrack(tracks[frame_after]);
        std::string file = track->getScreenshotFile();
        m_track_screenshot_widget->setImage(file, IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
        m_track_screenshot_widget->m_properties[PROP_ICON] = file;
    }
}   // onUpdate
