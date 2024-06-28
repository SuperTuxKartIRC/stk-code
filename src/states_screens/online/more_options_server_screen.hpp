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


#ifndef HEADER_MORE_OPTIONS_SERVER_SCREEN_HPP
#define HEADER_MORE_OPTIONS_SERVER_SCREEN_HPP

#include "guiengine/screen.hpp"
#include "guiengine/widgets.hpp"

namespace GUIEngine { class Widget; class ListWidget; class CheckBoxWidget;}


/**
* \brief Handles the main menu
* \ingroup states_screens
*/
class MoreOptionsServerScreen : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<MoreOptionsServerScreen>
{
private:
    friend class GUIEngine::ScreenSingleton<MoreOptionsServerScreen>;

    bool m_supports_ai;

    MoreOptionsServerScreen();

    GUIEngine::Widget* m_time_div_widget;
    GUIEngine::Widget* m_nb_ai_div_widget;
    GUIEngine::Widget* m_winning_div_widget;
    GUIEngine::Widget* m_life_div_widget;
    GUIEngine::Widget* m_team_div_widget;
    GUIEngine::Widget* m_tag_div_widget;
    GUIEngine::Widget* m_teams_selection_div_widget;

    GUIEngine::SpinnerWidget* m_time_widget_value_spinner;
    GUIEngine::SpinnerWidget* m_nb_ai_widget_value_spinner;
    GUIEngine::SpinnerWidget* m_winning_value_spinner;
    GUIEngine::SpinnerWidget* m_life_value_spinner;
    GUIEngine::SpinnerWidget* m_nb_team_value_spinner;
    GUIEngine::SpinnerWidget* m_tag_value_spinner;
    GUIEngine::CheckBoxWidget* m_teams_selection_checkbox;

    GUIEngine::IconButtonWidget* m_back_widget;

public:
    virtual void onUpdate(float delta) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    ///** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
        const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void tearDown() OVERRIDE;

};   // class MoreOptionsServerScreen

#endif
