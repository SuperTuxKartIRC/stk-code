////  SuperTuxKart - a fun racing game with go-kart
////  Copyright (C) 2013-2015 Lionel Fuentes
////  Copyright (C) 2023 William Lussier
////
////  This program is free software; you can redistribute it and/or
////  modify it under the terms of the GNU General Public License
////  as published by the Free Software Foundation; either version 3
////  of the License, or (at your option) any later version.
////
////  This program is distributed in the hope that it will be useful,
////  but WITHOUT ANY WARRANTY; without even the implied warranty of
////  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
////  GNU General Public License for more details.
////
////  You should have received a copy of the GNU General Public License
////  along with this program; if not, write to the Free Software
////  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//#ifndef HEADER_TEAMS_SETUP_SCREEN_HPP
//#define HEADER_TEAMS_SETUP_SCREEN_HPP
//
//#include "guiengine/screen.hpp"
//#include "network/remote_kart_info.hpp"
//
//namespace GUIEngine { class Widget; class LabelWidget; class ModelViewWidget; }
//
///**
//  * \brief Screen with soccer setup options
//  * \ingroup states_screens
//  */
//class TeamsSetupScreen : public GUIEngine::Screen,
//    public GUIEngine::ScreenSingleton<TeamsSetupScreen>
//{
//    friend class GUIEngine::ScreenSingleton<TeamsSetupScreen>;
//
//    TeamsSetupScreen();
//
//    struct KartViewInfo
//    {
//        GUIEngine::ModelViewWidget* view;
//        bool                        confirmed;
//        bool                        support_colorization;
//        KartTeam                    teams;
//        //KartTeamsColor              teamColor;
//
//        KartViewInfo() : view(), confirmed(false), support_colorization(false),
//            teams(KART_TEAM_NONE) {}
//    };
//
//    AlignedArray<KartViewInfo>  m_kart_view_info;
//
//    bool m_schedule_continue;
//
//public:
//
//    /** \brief implement callback from parent class GUIEngine::Screen */
//    virtual void loadedFromFile() OVERRIDE;
//
//    /** \brief implement callback from parent class GUIEngine::Screen */
//    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
//        const int playerID) OVERRIDE;
//
//    /** \brief implement callback from parent class GUIEngine::Screen */
//    virtual void beforeAddingWidget() OVERRIDE;
//
//    /** \brief implement callback from parent class GUIEngine::Screen */
//    virtual void init() OVERRIDE;
//
//    /** \brief implement callback from parent class GUIEngine::Screen */
//    virtual void tearDown() OVERRIDE;
//
//    virtual void onUpdate(float delta) OVERRIDE;
//
//    /** \brief implement callback from parent class GUIEngine::Screen */
//    virtual GUIEngine::EventPropagation filterActions(PlayerAction action,
//        int deviceID,
//        const unsigned int value,
//        Input::InputType type,
//        int playerId) OVERRIDE;
//
//    virtual bool onEscapePressed() OVERRIDE;
//
//private:
//    bool areAllKartsConfirmed() const;
//    int getNumConfirmedKarts();
//    void updateKartViewsLayout();
//    //void changeTeam(int player_id, KartTeam teams, KartTeamsColor teamsColor);
//    void changeTeam(int player_id, KartTeam teams);
//    void prepareGame();
//    //const float getHueColor(KartTeamsColor teamsColor);
//    void TeamsSetupScreen::changeTeamByDirection(int player_id, int direction);
//    void TeamsSetupScreen::changeTeamColor(int player_id, int direction);
//    int m_teamSelection = 1;
//
//    //KartTeamsColor m_team1Color = KART_TEAM_COLOR_RED;
//    //KartTeamsColor m_team2Color = KART_TEAM_COLOR_BLUE;
//    //KartTeamsColor m_team3Color = KART_TEAM_COLOR_GREEN;
//    //KartTeamsColor m_team4Color = KART_TEAM_COLOR_ORANGE;
//};
//
//#endif // HEADER_TEAMS_SETUP_SCREEN_HPP
