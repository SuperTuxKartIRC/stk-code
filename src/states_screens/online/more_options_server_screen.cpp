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
#include "network/network_config.hpp"
#include "network/server.hpp"
#include "network/server_config.hpp"
#include "network/child_loop.hpp"
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
    //m_max_players_widget = getWidget<SpinnerWidget>("max_players");
    //assert(m_max_players_widget != NULL);
    //int max = UserConfigParams::m_max_players.getDefaultValue();
    //m_max_players_widget->setMax(max);

    //if (UserConfigParams::m_max_players > max)
    //    UserConfigParams::m_max_players = max;

    //m_max_players_widget->setValue(UserConfigParams::m_max_players);

    m_time_div_widget = getWidget<Widget>("time-div");
    assert(m_time_widget != NULL);
    m_nb_ia_div_widget = getWidget<Widget>("nb-ai-div");
    assert(m_nb_ia_div_widget != NULL);
    m_winning_div_widget = getWidget<Widget>("winning-div");
    assert(m_winning_div_widget != NULL);
    m_life_div_widget = getWidget<Widget>("number-life-div");
    assert(m_life_div_widget != NULL);
    m_team_div_widget = getWidget<Widget>("nb-team-div");
    assert(m_team_div_widget != NULL);

    m_time_widget = getWidget<SpinnerWidget>("time-value-spinner");
    assert(m_time_widget != NULL);
    m_nb_ai_widget = getWidget<SpinnerWidget>("nb-ai-value-spinner");
    assert(m_nb_ai_widget != NULL);
    m_winning_value_spinner = getWidget<SpinnerWidget>("winning-value-spinner");
    assert(m_winning_value_spinner != NULL);
    m_life_value_spinner = getWidget<SpinnerWidget>("life-value-spinner");
    assert(m_life_value_spinner != NULL);
    m_nb_team_value_spinner = getWidget<SpinnerWidget>("nb-team-value-spinner");
    assert(m_nb_team_value_spinner != NULL);
    m_back_widget = getWidget<IconButtonWidget>("back");
    assert(m_back_widget != NULL);

    m_nb_ai_widget->setValue(ServerConfig::m_server_game_nb_ai);
    m_time_widget->setValue(ServerConfig::m_server_game_duration);
    m_life_value_spinner->setValue(ServerConfig::m_server_game_life);
    m_winning_value_spinner->setValue(ServerConfig::m_server_game_point);
}   // loadedFromFile

// ----------------------------------------------------------------------------
void MoreOptionsServerScreen::init()
{
    Screen::init();
    if (NetworkConfig::get()->isLAN())
        m_supports_ai = !UserConfigParams::m_lan_server_gp;
    else
        m_supports_ai = !UserConfigParams::m_wan_server_gp;

    if (!m_supports_ai) {
        m_nb_ai_widget->setCollapsed(false);
    }


    //m_life_value_spinner->setVisible(false);
    //m_life_value_label->setVisible(false);
    //m_life_value_label->setActive(false);
    //m_winning_value_spinner->setVisible(false);
    //m_point_value_label->setVisible(false);
    //m_point_value_label->setActive(false);

    //m_time_div_widget->setCollapsed(true, this);
    //m_nb_ia_div_widget->setCollapsed(true, this);
    //m_winning_div_widget->setCollapsed(true, this);
    //m_life_div_widget->setCollapsed(true, this);

    //m_time_div_widget->setVisible(false);
    //m_nb_ia_div_widget->setVisible(false);
    //m_winning_div_widget->setVisible(false);
    //m_life_div_widget->setVisible(false);

    if (ServerConfig::m_server_mode == 9 || ServerConfig::m_server_mode == 10 || ServerConfig::m_server_mode == 11 || ServerConfig::m_server_mode == 12) {
        //m_time_div_widget->setVisible(false);
        //m_nb_ia_div_widget->setVisible(false);

        //m_time_div_widget->setVisible(true);
        //m_nb_ia_div_widget->setVisible(true);
        //m_winning_div_widget->setVisible(true);
        //m_life_div_widget->setVisible(true);
        m_nb_team_value_spinner->setMax(4);
    }

    if (!(ServerConfig::m_server_mode == 9 || ServerConfig::m_server_mode == 10 || ServerConfig::m_server_mode == 11)) {
        //m_winning_div_widget->setVisible(false);

    }
    if (!(ServerConfig::m_server_mode == 12)) {
        //m_life_div_widget->setVisible(false);
        //m_life_value_spinner->setVisible(false);
    }

}   // init

// ----------------------------------------------------------------------------
void MoreOptionsServerScreen::beforeAddingWidget()
{
}   // beforeAddingWidget

// ----------------------------------------------------------------------------
/** Event callback which starts the server creation process.
 */
void MoreOptionsServerScreen::eventCallback(Widget* widget, const std::string& name, const int playerID) {
    if (name == m_time_widget->m_properties[PROP_ID])
    {
        ServerConfig::m_server_game_duration = m_time_widget->getValue();
    }
    else if (name == m_nb_ai_widget->m_properties[PROP_ID])
    {
        ServerConfig::m_server_game_nb_ai = m_time_widget->getValue();
    }
    else if (name == m_winning_value_spinner->m_properties[PROP_ID])
    {
        ServerConfig::m_server_game_point = m_time_widget->getValue();
    }
    else if (name == m_life_value_spinner->m_properties[PROP_ID])
    {
        ServerConfig::m_server_game_life = m_time_widget->getValue();
    }
    else if (name == m_back_widget->m_properties[PROP_ID])
    {
        //NetworkConfig::get()->unsetNetworking();
        //CreateServerScreen::getInstance()->push();
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