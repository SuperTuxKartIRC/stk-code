//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
//  Copyright (C) 2023 SuperTuxKart-Team
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

#ifndef HEADER_SERVER_CONFIGURATION_MORE_OPTIONS_DIALOG_HPP
#define HEADER_SERVER_CONFIGURATION_MORE_OPTIONS_DIALOG_HPP

#include "guiengine/modaldialog.hpp"
#include "race/race_manager.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets.hpp"

namespace GUIEngine
{
    class SpinnerWidget;
    class LabelWidget;
    class RibbonWidget;
    class IconButtonWidget;
    class Widget;
    class CheckBoxWidget;
}

class ServerConfigurationMoreOptionsDialog : public GUIEngine::ModalDialog
{
private:
    bool m_self_destroy;

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

    GUIEngine::RibbonWidget* m_options_widget;

    GUIEngine::IconButtonWidget* m_ok_widget;
    GUIEngine::IconButtonWidget* m_cancel_widget;

public:
    ServerConfigurationMoreOptionsDialog(float width, float height) : ModalDialog(width, height) // (0.8f, 0.8f)
    {
        m_self_destroy = false;
       
        loadFromFile("online/server_configuration_more_options_dialog.stkgui");
    }
    // ------------------------------------------------------------------------
    void beforeAddingWidgets();
    // ------------------------------------------------------------------------
    GUIEngine::EventPropagation processEvent(const std::string& source);
    // ------------------------------------------------------------------------
    void init();
    // ------------------------------------------------------------------------
    void onEnterPressedInternal() { m_self_destroy = true; }
    // ------------------------------------------------------------------------
    bool onEscapePressed()
    {
        m_self_destroy = true;
        return false;
    }
    // ------------------------------------------------------------------------
    void onUpdate(float dt)
    {
        if (m_self_destroy)
            ModalDialog::dismiss();
    }
};   // class ServerConfigurationDialog

#endif
