//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

#include "states_screens/dialogs/server_configuration_more_options_dialog.hpp"

#include "config/user_config.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "network/network_string.hpp"
#include "network/protocols/lobby_protocol.hpp"
#include "network/stk_host.hpp"
#include "network/server_config.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/dialogs/server_configuration_dialog.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
#include <network/protocols/client_lobby.hpp>

using namespace GUIEngine;

// ----------------------------------------------------------------------------
void ServerConfigurationMoreOptionsDialog::beforeAddingWidgets()
{
	m_options_widget = getWidget<RibbonWidget>("options");
	assert(m_options_widget != NULL);
	m_ok_widget = getWidget<IconButtonWidget>("ok");
	assert(m_ok_widget != NULL);
	m_cancel_widget = getWidget<IconButtonWidget>("cancel");
	assert(m_cancel_widget != NULL);

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

	m_nb_ai_widget_value_spinner->setValue(ServerConfig::m_server_game_nb_ai);
	m_nb_team_value_spinner->setValue(ServerConfig::m_server_game_nb_team);
	m_tag_value_spinner->setValue(ServerConfig::m_server_game_nb_tag);
	m_teams_selection_checkbox->setState(ServerConfig::m_server_game_teams_selection);

	if (ServerConfig::m_server_game_duration < 0) {
		m_time_widget_value_spinner->setValue(0);
	}
	else if (ServerConfig::m_server_game_duration != 0 && ServerConfig::m_server_game_duration > 59) {
		m_time_widget_value_spinner->setValue(ServerConfig::m_server_game_duration / 60);
	}
	else {
		m_time_widget_value_spinner->setValue(ServerConfig::m_server_game_duration / 60);
	}
	m_life_value_spinner->setValue(ServerConfig::m_server_game_life);
	m_winning_value_spinner->setValue(ServerConfig::m_server_game_point);
	m_tag_value_spinner->setValue(ServerConfig::m_server_game_nb_tag);


	m_time_div_widget->setCollapsed(true);
	m_nb_ai_div_widget->setCollapsed(true);
	m_winning_div_widget->setCollapsed(true);
	m_life_div_widget->setCollapsed(true);
	m_team_div_widget->setCollapsed(true);
	m_tag_div_widget->setCollapsed(true);
	m_teams_selection_div_widget->setCollapsed(true);

	// La this dans le setCollapsed() est important
	//m_time_div_widget->setCollapsed(true, this);
	//m_nb_ai_div_widget->setCollapsed(true, this);
	//m_winning_div_widget->setCollapsed(true, this);
	//m_life_div_widget->setCollapsed(true, this);
	//m_team_div_widget->setCollapsed(true, this);

	//m_life_value_spinner->setVisible(false);
	//m_life_value_label->setVisible(false);
	//m_life_value_label->setActive(false);
	//m_winning_value_spinner->setVisible(false);
	//m_point_value_label->setVisible(false);
	//m_point_value_label->setActive(false);

	switch (RaceManager::get()->getMinorMode())
	{
	case RaceManager::MINOR_MODE_NORMAL_RACE:
	{
		m_time_div_widget->setCollapsed(false);
		break;
	}
	case RaceManager::MINOR_MODE_TIME_TRIAL:
	{
		m_time_div_widget->setCollapsed(false);
		break;
	}
	case RaceManager::MINOR_MODE_FREE_FOR_ALL:
	{
		m_time_div_widget->setCollapsed(false);
		break;
	}
	case RaceManager::MINOR_MODE_CAPTURE_THE_FLAG:
	{
		m_time_div_widget->setCollapsed(false);
		break;
	}
	case RaceManager::MINOR_MODE_SOCCER:
	{
		m_time_div_widget->setCollapsed(false);
		break;
	}
	case RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_TEAM:
	case RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_PLAYER:
	case RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_ALL_POINTS_PLAYER:
	{
		m_time_div_widget->setCollapsed(false);
		m_nb_ai_div_widget->setCollapsed(false);
		m_winning_div_widget->setCollapsed(false);
		m_nb_team_value_spinner->setCollapsed(false);
		break;
	}
	case RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_LIFE:
	{
		m_time_div_widget->setCollapsed(false);
		m_nb_ai_div_widget->setCollapsed(false);
		m_team_div_widget->setCollapsed(false);
		m_life_div_widget->setCollapsed(false);
		break;
	}
	case RaceManager::MINOR_MODE_TAG_ZOMBIE_ARENA_BATTLE:
	{
		m_time_div_widget->setCollapsed(false);
		m_nb_ai_div_widget->setCollapsed(false);
		m_tag_div_widget->setCollapsed(false);
		break;
	}
	case RaceManager::MINOR_MODE_TAG_ZOMBIE_SURVIROR_ARENA_BATTLE:
	{
		m_time_div_widget->setCollapsed(false);
		m_nb_ai_div_widget->setCollapsed(false);
		break;
	}
	case RaceManager::MINOR_MODE_TAG_ZOMBIE_LAST_SURVIROR_ARENA_BATTLE:
	{

		break;
	}
	case RaceManager::MINOR_MODE_HOT_POTATO_ARENA_BATTLE:
	{

		break;
	}
	case RaceManager::MINOR_MODE_HOT_POTATO_TIME_ARENA_BATTLE:
	{

		break;
	}
	case RaceManager::MINOR_MODE_KING_HAT_ARENA_BATTLE:
	{

		break;
	}
	default:
	{

		break;
	}
	}
}
// ----------------------------------------------------------------------------
void ServerConfigurationMoreOptionsDialog::init()
{
	ModalDialog::init();
	m_nb_ai_widget_value_spinner->setActive(false);
}   // init

// ----------------------------------------------------------------------------
GUIEngine::EventPropagation
ServerConfigurationMoreOptionsDialog::processEvent(const std::string& source)
{
	if (source == m_options_widget->m_properties[PROP_ID])
	{
		const std::string& selection =
			m_options_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
		if (selection == m_cancel_widget->m_properties[PROP_ID])
		{
			m_self_destroy = true;
			auto cl = LobbyProtocol::get<ClientLobby>();
			if (cl)
			{
				new ServerConfigurationDialog(false);
				//RaceManager::get()->isSoccerMode() &&
				//    cl->getGameSetup()->isSoccerGoalTarget()
			}
			return GUIEngine::EVENT_BLOCK;
		}
		else if (selection == m_ok_widget->m_properties[PROP_ID])
		{
			// Qu'est-ce qui est arriver ici // Le code doit être vérifier // William Lussier // 2023-11-24 14h46
			int mode = 0;
			switch (RaceManager::get()->getMinorMode())
			{
			case RaceManager::MINOR_MODE_CAPTURE_THE_FLAG:
			{
				mode = 8;
				break;
			}
			case RaceManager::MINOR_MODE_FREE_FOR_ALL:
			{
				mode = 7;
				break;
			}
			case RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_TEAM:
			{
				mode = 9;
				break;
			}
			case RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_PLAYER:
			{
				mode = 10;
				break;
			}
			case RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_ALL_POINTS_PLAYER:
			{
				mode = 11;
				break;
			}
			case RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_LIFE:
			{
				mode = 12;
				break;
			}
			case RaceManager::MINOR_MODE_TAG_ZOMBIE_ARENA_BATTLE:
			{
				mode = 13;
				break;
			}
			case RaceManager::MINOR_MODE_TAG_ZOMBIE_SURVIROR_ARENA_BATTLE:
			{
				mode = 14;
				break;
			}
			case RaceManager::MINOR_MODE_TAG_ZOMBIE_LAST_SURVIROR_ARENA_BATTLE:
			{
				mode = 15;
				break;
			}
			case RaceManager::MINOR_MODE_HOT_POTATO_ARENA_BATTLE:
			{
				mode = 16;
				break;
			}
			case RaceManager::MINOR_MODE_HOT_POTATO_TIME_ARENA_BATTLE:
			{
				mode = 17;
				break;
			}
			case RaceManager::MINOR_MODE_KING_HAT_ARENA_BATTLE:
			{
				mode = 18;
				break;
			}
			}

			// TODO : Besoins de modifications // William Lussier 
			NetworkString change(PROTOCOL_LOBBY_ROOM);
			change.addUInt8(LobbyProtocol::LE_CONFIG_SERVER_OPTION);
			change.addUInt8(0)
				.addUInt8(mode)
				.addUInt8(0)
				.addUInt8(m_time_widget_value_spinner->getValue())
				.addUInt8(m_nb_ai_widget_value_spinner->getValue());



			if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_TEAM || 
					 RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_PLAYER ||
					 RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_ALL_POINTS_PLAYER || 
					 RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_LIFE)
			{
				if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_LIFE)
					change.addUInt8(m_life_value_spinner->getValue());
				else 
					change.addUInt8(m_winning_value_spinner->getValue());

				change.addUInt8(m_nb_team_value_spinner->getValue());
			}
			else if ((int)RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TAG_ZOMBIE_ARENA_BATTLE) {
				change.addUInt8(m_tag_value_spinner->getValue());
			}

			STKHost::get()->sendToServer(&change, true);
			m_self_destroy = true;
			//auto cl = LobbyProtocol::get<ClientLobby>();
			//if (cl)
			//{
			//    new ServerConfigurationDialog(false);
			//    //RaceManager::get()->isSoccerMode() &&
			//    //    cl->getGameSetup()->isSoccerGoalTarget()
			//}
			return GUIEngine::EVENT_BLOCK;
		}
	}


	else if (source == m_time_widget_value_spinner->m_properties[PROP_ID])
	{
		RaceManager::get()->setTimeTarget(m_time_widget_value_spinner->getValue() * 60);
		ServerConfig::m_server_game_duration = m_time_widget_value_spinner->getValue() * 60;
		return GUIEngine::EVENT_BLOCK;
	}
	else if (source == m_nb_ai_widget_value_spinner->m_properties[PROP_ID])
	{
		// TODO : Besoins de modifications 
		ServerConfig::m_server_game_nb_ai = m_nb_ai_widget_value_spinner->getValue();
		return GUIEngine::EVENT_BLOCK;
	}
	else if (source == m_winning_value_spinner->m_properties[PROP_ID])
	{
		// TODO : Besoins de modifications 
		RaceManager::get()->setHitCaptureLimit(m_winning_value_spinner->getValue());
		ServerConfig::m_server_game_point = m_winning_value_spinner->getValue();
		return GUIEngine::EVENT_BLOCK;
	}
	else if (source == m_life_value_spinner->m_properties[PROP_ID])
	{
		RaceManager::get()->setLifeTarget(m_life_value_spinner->getValue());
		ServerConfig::m_server_game_life = m_life_value_spinner->getValue();
		// TODO : Besoins de modifications 
		return GUIEngine::EVENT_BLOCK;
	}
	else if (source == m_nb_team_value_spinner->m_properties[PROP_ID])
	{
		// TODO : Besoins de modifications 
		RaceManager::get()->setNbTeams(m_nb_team_value_spinner->getValue());
		ServerConfig::m_server_game_nb_team = m_nb_team_value_spinner->getValue();
		return GUIEngine::EVENT_BLOCK;
	}
	else if (source == m_tag_value_spinner->m_properties[PROP_ID])
	{
		RaceManager::get()->setTagTarget(m_tag_value_spinner->getValue());
		ServerConfig::m_server_game_nb_tag = m_tag_value_spinner->getValue();
	}
	return GUIEngine::EVENT_LET;
}   // eventCallback