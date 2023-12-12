//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2023 William Lussier
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

#include "states_screens/online/more_options_server_screen.hpp"

#include "audio/sfx_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "karts/controller/network_ai_controller.hpp"
#include "network/child_loop.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include <network/protocol.hpp>
#include "network/protocols/lobby_protocol.hpp"
#include "network/server.hpp"
#include "network/server_config.hpp"
#include "network/socket_address.hpp"
#include "network/stk_host.hpp"
#include "online/online_profile.hpp"
#include "states_screens/online/create_server_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/stk_process.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <irrString.h>

#include <string>
#include <iostream>


using namespace GUIEngine;

// ----------------------------------------------------------------------------

MoreOptionsServerScreen::MoreOptionsServerScreen() : Screen("online/create_server_more_options.stkgui")
{
}   // MoreOptionsServerScreen

void MoreOptionsServerScreen::loadedFromFile()
{
    m_time_div_widget = getWidget<Widget>("time-div");
    assert(m_time_widget_value_spinner != NULL);
    m_nb_ai_div_widget = getWidget<Widget>("nb-ai-div");
    assert(m_nb_ai_div_widget != NULL);
    m_winning_div_widget = getWidget<Widget>("winning-div");
    assert(m_winning_div_widget != NULL);
    m_life_div_widget = getWidget<Widget>("number-life-div");
    assert(m_life_div_widget != NULL);
    m_team_div_widget = getWidget<Widget>("nb-team-div");
    assert(m_team_div_widget != NULL);
    m_tag_div_widget = getWidget<Widget>("number-tag-div");
    assert(m_tag_div_widget != NULL);
    m_teams_selection_div_widget = getWidget<Widget>("teams-selection-div");
    assert(m_teams_selection_div_widget != NULL);

    m_time_widget_value_spinner = getWidget<SpinnerWidget>("time-value-spinner");
    assert(m_time_widget_value_spinner != NULL);
    m_nb_ai_widget_value_spinner = getWidget<SpinnerWidget>("nb-ai-value-spinner");
    assert(m_nb_ai_widget_value_spinner != NULL);
    m_winning_value_spinner = getWidget<SpinnerWidget>("winning-value-spinner");
    assert(m_winning_value_spinner != NULL);
    m_life_value_spinner = getWidget<SpinnerWidget>("life-value-spinner");
    assert(m_life_value_spinner != NULL);
    m_nb_team_value_spinner = getWidget<SpinnerWidget>("nb-team-value-spinner");
    assert(m_nb_team_value_spinner != NULL);
    m_tag_value_spinner = getWidget<SpinnerWidget>("tag-value-spinner");
    assert(m_tag_value_spinner != NULL);
    m_teams_selection_checkbox = getWidget<CheckBoxWidget>("teams-selection-checkbox");
    assert(m_teams_selection_checkbox != NULL);

    m_back_widget = getWidget<IconButtonWidget>("back");
    assert(m_back_widget != NULL);

    m_nb_ai_widget_value_spinner->setValue(ServerConfig::m_server_game_nb_ai);
    m_nb_team_value_spinner->setValue(ServerConfig::m_server_game_nb_team);
    if (ServerConfig::m_server_game_duration==0 || ServerConfig::m_server_game_duration<59) {
        m_time_widget_value_spinner->setValue(ServerConfig::m_server_game_duration);
    }
    else {
        m_time_widget_value_spinner->setValue(ServerConfig::m_server_game_duration/60);
    }

    m_life_value_spinner->setValue(ServerConfig::m_server_game_life);
    m_winning_value_spinner->setValue(ServerConfig::m_server_game_point);
    m_tag_value_spinner->setValue(ServerConfig::m_server_game_nb_tag);
    m_teams_selection_checkbox->setState(ServerConfig::m_server_game_teams_selection);

    //m_nb_ai_widget_value_spinner->setMax(10); Nb jour max dans le serveur
    //m_tag_value_spinner->setMax(10); // Nb jour max dans le serveur
}   // loadedFromFile

// ----------------------------------------------------------------------------
void MoreOptionsServerScreen::init()
{
    Screen::init();
}   // init

// ----------------------------------------------------------------------------
void MoreOptionsServerScreen::beforeAddingWidget()
{
    if (NetworkConfig::get()->isLAN())
        m_supports_ai = !UserConfigParams::m_lan_server_gp;
    else
        m_supports_ai = !UserConfigParams::m_wan_server_gp;

    if (!m_supports_ai) {
        m_nb_ai_widget_value_spinner->setCollapsed(false);
    }

    m_life_div_widget->setCollapsed(true, this);
    m_winning_div_widget->setCollapsed(true, this);
    m_time_div_widget->setCollapsed(true, this);
    m_nb_ai_div_widget->setCollapsed(true, this);
    m_team_div_widget->setCollapsed(true, this);
    m_tag_div_widget->setCollapsed(true, this);
    m_teams_selection_div_widget->setCollapsed(true, this);
    m_time_div_widget->setCollapsed(false, this);
    // Dois setter les valeurs des spinner avec les variable de configurations 
    // Au minimum pour team_arena (correspond au 4 mode de jeux bataille en équipe) + tag_zombie
    // Si possible une variable différente pour team_arena_life (pour le temps)
    if ((ServerConfig::m_server_mode == 9 || ServerConfig::m_server_mode == 10 || ServerConfig::m_server_mode == 11 || ServerConfig::m_server_mode == 12)) {
      
        //m_nb_ai_div_widget->setCollapsed(false, this);
        m_team_div_widget->setCollapsed(false, this);
        m_teams_selection_div_widget->setCollapsed(false, this);

        m_nb_team_value_spinner->setMin(1); // 2
        m_nb_team_value_spinner->setMax(4);
        if ((ServerConfig::m_server_mode == 12))
            m_life_div_widget->setCollapsed(false, this);
        else
            m_winning_div_widget->setCollapsed(false, this);
    }
    if ((ServerConfig::m_server_mode == 13)) {
        m_time_div_widget->setCollapsed(false, this);
        //m_nb_ai_div_widget->setCollapsed(false, this);
        m_tag_div_widget->setCollapsed(false, this); 
    }
}   // beforeAddingWidget

// ----------------------------------------------------------------------------
/** Event callback which starts the server creation process.
 */
void MoreOptionsServerScreen::eventCallback(Widget* widget, const std::string& name, const int playerID) {
    if (name == m_time_widget_value_spinner->m_properties[PROP_ID])
    {
        if (m_time_widget_value_spinner->getValue()>59) {
            m_time_widget_value_spinner->setValue(0);
        }
        RaceManager::get()->setTimeTarget(m_time_widget_value_spinner->getValue()*60);
        ServerConfig::m_server_game_duration = m_time_widget_value_spinner->getValue()*60;
    }
    else if (name == m_winning_value_spinner->m_properties[PROP_ID])
    {
        RaceManager::get()->setHitCaptureLimit(m_winning_value_spinner->getValue());
        ServerConfig::m_server_game_point = m_winning_value_spinner->getValue();
    }
    else if (name == m_life_value_spinner->m_properties[PROP_ID])
    {
        RaceManager::get()->setLifeTarget(m_life_value_spinner->getValue());
        ServerConfig::m_server_game_life = m_life_value_spinner->getValue();
    }
    else if (name == m_nb_ai_widget_value_spinner->m_properties[PROP_ID])
    {
        if (m_nb_ai_widget_value_spinner->getValue() > 59) {
            m_nb_ai_widget_value_spinner->setValue(0);
        }
        //RaceManager::get()->setLifeTarget(m_nb_ai_widget_value_spinner->getValue());
        ServerConfig::m_server_game_nb_ai = m_nb_ai_widget_value_spinner->getValue();
    }
    else if (name == m_nb_team_value_spinner->m_properties[PROP_ID])
    {
        RaceManager::get()->setNbTeams(m_nb_team_value_spinner->getValue());
        ServerConfig::m_server_game_nb_team = m_nb_team_value_spinner->getValue();
    }
    else if (name == m_tag_value_spinner->m_properties[PROP_ID])
    {
        RaceManager::get()->setTagTarget(m_tag_value_spinner->getValue());
        ServerConfig::m_server_game_nb_tag = m_tag_value_spinner->getValue();
    }
    else if (name == m_back_widget->m_properties[PROP_ID])
    {
        StateManager::get()->escapePressed();
    }
}

// ----------------------------------------------------------------------------
/** Called once per framce to check if the server creation request has
 *  finished. If so, if pushes the server creation sceen.
 */
void MoreOptionsServerScreen::onUpdate(float delta)
{
    // If no host has been created, keep on waiting.
    if (!STKHost::existHost())
        return;

    //NetworkingLobby::getInstance()->push();
}   // onUpdate

void MoreOptionsServerScreen::tearDown()
{

}   // tearDown