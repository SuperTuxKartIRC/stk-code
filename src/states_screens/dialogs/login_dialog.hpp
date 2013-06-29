//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
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


#ifndef HEADER_LOGIN_DIALOG_HPP
#define HEADER_LOGIN_DIALOG_HPP

#include <irrString.h>

#include "guiengine/modaldialog.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"

/**
 * \brief Dialog that allows a user to sign in
 * \ingroup states_screens
 */
class LoginDialog : public GUIEngine::ModalDialog
{

private:
    
    bool m_self_destroy;
    bool m_open_registration_dialog;
    bool m_reshow_current_screen;
    GUIEngine::LabelWidget * m_message_widget;
    GUIEngine::LabelWidget * m_info_widget;
    
public:
    
    enum Message
    {
        Normal                  = 1,    // If the user presses the sign in button himself
        Signing_In_Required     = 2,    // If the user needs to be signed in
        Registration_Required   = 3     // If the user needs to be registered
    };

    /**
     * Creates a modal dialog with given percentage of screen width and height
     */
    LoginDialog(const Message);
    ~LoginDialog();

    void onEnterPressedInternal();
    GUIEngine::EventPropagation processEvent(const std::string& eventSource);
    void beforeAddingWidgets();
    
    virtual void onUpdate(float dt);
    //virtual void onTextUpdated();
};

#endif
