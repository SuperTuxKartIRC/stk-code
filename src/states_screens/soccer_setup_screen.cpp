#include "states_screens/soccer_setup_screen.hpp"

#include "audio/sfx_manager.hpp"
#include "config/user_config.hpp"
#include <ge_render_info.hpp>
#include "guiengine/widgets/bubble_widget.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/model_view_widget.hpp"
#include "guiengine/scalable_font.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "modes/world.hpp"
#include "karts/kart_properties_manager.hpp"
#include "states_screens/arenas_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IrrlichtDevice.h>

using namespace GUIEngine;

#define KART_CONTINUOUS_ROTATION_SPEED      35.f
#define KART_CONFIRMATION_ROTATION_SPEED    4.f
#define KART_CONFIRMATION_TARGET_ANGLE      10.f

// -----------------------------------------------------------------------------

SoccerSetupScreen::SoccerSetupScreen() : Screen("soccer_setup.stkgui")
{
    //World::getWorld()->setTeamsInGame(KART_TEAM_GREEN);
    //World::getWorld()->setTeamsInGame(KART_TEAM_CYAN);
}

// -----------------------------------------------------------------------------

void SoccerSetupScreen::loadedFromFile()
{
}

// ----------------------------------------------------------------------------
void SoccerSetupScreen::eventCallback(Widget* widget, const std::string& name,
    const int playerID)
{
    if (m_schedule_continue)
        return;

    if (name == "continue")
    {
        int nb_players = (int)m_kart_view_info.size();

        if (!areAllKartsConfirmed())
        {
            for (int i = 0; i < nb_players; i++)
            {
                if (!m_kart_view_info[i].confirmed)
                {
                    m_kart_view_info[i].confirmed = true;
                    m_kart_view_info[i].view->setRotateTo(KART_CONFIRMATION_TARGET_ANGLE, KART_CONFIRMATION_ROTATION_SPEED);
                    m_kart_view_info[i].view->setBadge(OK_BADGE);
                }
            }
            SFXManager::get()->quickSound("wee");
            m_schedule_continue = true;
        }
        else
        {
            m_schedule_continue = true;
        }
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "team1_team")
    {
        if (m_kart_view_info.size() == 1)
        {
            changeTeam(0, RaceManager::get()->getTeamsInGame()[0]);
        }
    }
    else if (name == "team2_team")
    {
        if (m_kart_view_info.size() == 1)
        {
            changeTeam(0, RaceManager::get()->getTeamsInGame()[1]);
        }
    }
}   // eventCallback

// -----------------------------------------------------------------------------
void SoccerSetupScreen::beforeAddingWidget()
{
    bool multitouch_enabled = (UserConfigParams::m_multitouch_active == 1 &&
        irr_driver->getDevice()->supportsTouchDevice()) ||
        UserConfigParams::m_multitouch_active > 1;

    RaceManager::get()->setTeamsInGame(2);

    if (multitouch_enabled)
    {
        Widget* team = getWidget<Widget>("choose_team");
        //I18N: In soccer setup screen
        core::stringw team1_name = (World::getWorld()->getKartTeamsColorName(RaceManager::get()->getTeamsInGame()[0])).c_str();
        core::stringw team2_name = (World::getWorld()->getKartTeamsColorName(RaceManager::get()->getTeamsInGame()[1])).c_str();
        // team->setText(_("Press team1_name or team2_name soccer icon to change team"));

        std::wstringstream wss;
        wss << L"Press " << team1_name.c_str() << L" or " << team2_name.c_str() << L" soccer icon to change team";

        core::stringw text(wss.str().c_str());
        team->setText(text.c_str());

    }
    Widget* central_div = getWidget<Widget>("central_div");

    // Add the 3D views for the karts
    int nb_players = RaceManager::get()->getNumPlayers();
    for (int i = 0; i < nb_players; i++)
    {
        const RemoteKartInfo& kart_info = RaceManager::get()->getKartInfo(i);
        const std::string& kart_name = kart_info.getKartName();

        const KartProperties* props = kart_properties_manager->getKart(kart_name);
        const KartModel& kart_model = props->getMasterKartModel();

        // Add the view
        ModelViewWidget* kart_view = new ModelViewWidget();
        // These values will be overriden by updateKartViewsLayout() anyway
        kart_view->m_x = 0;
        kart_view->m_y = 0;
        kart_view->m_w = 200;
        kart_view->m_h = 200;
        kart_view->clearModels();

        // Record info about it for further update
        KartViewInfo info;

        int team_default = RaceManager::get()->getTeamsInGame()[0];

        int single_team = RaceManager::get()->getTeamsInGame()[UserConfigParams::m_soccer_default_team -1];
        info.team = (nb_players == 1 ? (KartTeam)single_team :
            (i & 1 ? RaceManager::get()->getTeamsInGame()[1] : RaceManager::get()->getTeamsInGame()[0]));

        // addModel requires loading the RenderInfo first
        info.support_colorization = kart_model.supportColorization();
        if (info.support_colorization)
        {
            kart_view->getModelViewRenderInfo()->setHue
            (info.team == RaceManager::get()->getTeamsInGame()[1] ? World::getWorld()->getHueValueForTeam(RaceManager::get()->getTeamsInGame()[1])
                : World::getWorld()->getHueValueForTeam(RaceManager::get()->getTeamsInGame()[0]));
        }

        core::matrix4 model_location;
        model_location.setScale(core::vector3df(35.0f, 35.0f, 35.0f));
        // Add the kart model (including wheels and speed weight objects)
        kart_view->addModel(kart_model.getModel(), model_location,
            kart_model.getBaseFrame(), kart_model.getBaseFrame());

        model_location.setScale(core::vector3df(1.0f, 1.0f, 1.0f));
        for (unsigned i = 0; i < 4; i++)
        {
            model_location.setTranslation(kart_model
                .getWheelGraphicsPosition(i).toIrrVector());
            kart_view->addModel(kart_model.getWheelModel(i), model_location);
        }

        for (unsigned i = 0; i < kart_model.getSpeedWeightedObjectsCount();
            i++)
        {
            const SpeedWeightedObject& obj =
                kart_model.getSpeedWeightedObject(i);
            core::matrix4 swol = obj.m_location;
            if (!obj.m_bone_name.empty())
            {
                core::matrix4 inv =
                    kart_model.getInverseBoneMatrix(obj.m_bone_name);
                swol = inv * obj.m_location;
            }
            kart_view->addModel(obj.m_model, swol, -1, -1, 0.0f,
                obj.m_bone_name);
        }

        kart_view->setRotateContinuously(KART_CONTINUOUS_ROTATION_SPEED);
        kart_view->update(0);

        central_div->getChildren().push_back(kart_view);

        info.view = kart_view;
        info.confirmed = false;
        m_kart_view_info.push_back(info);
        RaceManager::get()->setKartTeam(i, info.team);
    }

    // Update layout
    updateKartViewsLayout();
}   // beforeAddingWidget

// -----------------------------------------------------------------------------
void SoccerSetupScreen::init()
{
    m_schedule_continue = false;

    Screen::init();

    // Set focus on "continue"
    ButtonWidget* bt_continue = getWidget<ButtonWidget>("continue");
    bt_continue->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    // We need players to be able to choose their teams
    input_manager->setMasterPlayerOnly(false);

    // This flag will cause that a 'fire' event will be mapped to 'select' (if
    // 'fire' is not assigned to a GUI event). This is done to support the old
    // way of player joining by pressing 'fire' instead of 'select'.
    input_manager->getDeviceManager()->mapFireToSelect(true);
}   // init

// -----------------------------------------------------------------------------
void SoccerSetupScreen::tearDown()
{
    Widget* central_div = getWidget<Widget>("central_div");

    // Reset the 'map fire to select' option of the device manager
    input_manager->getDeviceManager()->mapFireToSelect(false);

    // Remove all ModelViewWidgets we created manually
    PtrVector<Widget>& children = central_div->getChildren();
    for (int i = children.size() - 1; i >= 0; i--)
    {
        if (children[i].getType() == WTYPE_MODEL_VIEW)
            children.erase(i);
    }
    m_kart_view_info.clear();

    Screen::tearDown();
}   // tearDown

void SoccerSetupScreen::changeTeam(int player_index, KartTeam new_team)
{
    m_kart_view_info[player_index].team = new_team;

    KartViewInfo& view_info = m_kart_view_info[player_index];
    view_info.team = new_team;

    if (view_info.support_colorization)
    {
        view_info.view->getModelViewRenderInfo()->setHue
        (new_team == RaceManager::get()->getTeamsInGame()[1] ? World::getWorld()->getHueValueForTeam(RaceManager::get()->getTeamsInGame()[1])
            : World::getWorld()->getHueValueForTeam(RaceManager::get()->getTeamsInGame()[0]));
    }

    RaceManager::get()->setKartTeam(player_index, new_team);
    updateKartViewsLayout();
}

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation SoccerSetupScreen::filterActions(PlayerAction action,
    int deviceID,
    const unsigned int value,
    Input::InputType type,
    int playerId)
{
    if (m_schedule_continue)
        return EVENT_BLOCK;

    const bool pressed_down = value > Input::MAX_VALUE * 2 / 3;

    if (!pressed_down)
        return EVENT_BLOCK;


    ButtonWidget* bt_continue = getWidget<ButtonWidget>("continue");
    BubbleWidget* bubble = getWidget<BubbleWidget>("choose_team");

    switch (action)
    {
    case PA_MENU_LEFT:
        if (bt_continue->isFocusedForPlayer(PLAYER_ID_GAME_MASTER) ||
            bubble->isFocusedForPlayer(PLAYER_ID_GAME_MASTER))
        {
            if (m_kart_view_info[playerId].confirmed == false)
                changeTeam(playerId, RaceManager::get()->getTeamsInGame()[0]);

            return EVENT_BLOCK;
        }
        break;
    case PA_MENU_RIGHT:
        if (bt_continue->isFocusedForPlayer(PLAYER_ID_GAME_MASTER) ||
            bubble->isFocusedForPlayer(PLAYER_ID_GAME_MASTER))
        {
            if (m_kart_view_info[playerId].confirmed == false)
                changeTeam(playerId, RaceManager::get()->getTeamsInGame()[1]);

            return EVENT_BLOCK;
        }
        break;
    case PA_MENU_UP:
        if (playerId != PLAYER_ID_GAME_MASTER)
            return EVENT_BLOCK;
        break;
    case PA_MENU_DOWN:
        if (playerId != PLAYER_ID_GAME_MASTER)
            return EVENT_BLOCK;
        break;
    case PA_MENU_SELECT:
    {
        if (!bt_continue->isFocusedForPlayer(PLAYER_ID_GAME_MASTER) &&
            !bubble->isFocusedForPlayer(PLAYER_ID_GAME_MASTER) &&
            playerId == PLAYER_ID_GAME_MASTER)
        {
            return EVENT_LET;
        }

        if (!m_kart_view_info[playerId].confirmed)
        {
            // Confirm team selection
            m_kart_view_info[playerId].confirmed = true;
            m_kart_view_info[playerId].view->setRotateTo(
                KART_CONFIRMATION_TARGET_ANGLE,
                KART_CONFIRMATION_ROTATION_SPEED);
            m_kart_view_info[playerId].view->setBadge(OK_BADGE);
            m_kart_view_info[playerId].view->unsetBadge(BAD_BADGE);
            SFXManager::get()->quickSound("wee");
        }

        if (areAllKartsConfirmed())
            m_schedule_continue = true;

        return EVENT_BLOCK;
    }
    case PA_MENU_CANCEL:
    {
        if (!bt_continue->isFocusedForPlayer(PLAYER_ID_GAME_MASTER) &&
            !bubble->isFocusedForPlayer(PLAYER_ID_GAME_MASTER) &&
            playerId == PLAYER_ID_GAME_MASTER)
        {
            return EVENT_LET;
        }

        if ((!m_kart_view_info[playerId].confirmed) &&
            (playerId == PLAYER_ID_GAME_MASTER))
        {
            return EVENT_LET;
        }

        if (m_kart_view_info[playerId].confirmed)
        {
            // Un-confirm team selection
            m_kart_view_info[playerId].confirmed = false;
            m_kart_view_info[playerId].view->setRotateContinuously(
                KART_CONTINUOUS_ROTATION_SPEED);
            m_kart_view_info[playerId].view->unsetBadge(OK_BADGE);

            for (unsigned int i = 0; i < m_kart_view_info.size(); i++)
            {
                m_kart_view_info[i].view->unsetBadge(BAD_BADGE);
            }
        }

        return EVENT_BLOCK;
    }
    default:
        break;
    }

    return EVENT_LET;
}   // filterActions

// -----------------------------------------------------------------------------
void SoccerSetupScreen::onUpdate(float delta)
{
    int nb_players = (int)m_kart_view_info.size();

    if (m_schedule_continue)
    {
        for (int i = 0; i < nb_players; i++)
        {
            if (m_kart_view_info[i].view->isRotating() == true)
                return;
        }
        m_schedule_continue = false;
        prepareGame();
        ArenasScreen::getInstance()->push();
    }
}   // onUpdate


// ----------------------------------------------------------------------------
bool SoccerSetupScreen::areAllKartsConfirmed() const
{
    bool all_confirmed = true;
    int nb_players = (int)m_kart_view_info.size();
    for (int i = 0; i < nb_players; i++)
    {
        if (!m_kart_view_info[i].confirmed)
        {
            all_confirmed = false;
            break;
        }
    }
    return all_confirmed;
}   // areAllKartsConfirmed

// -----------------------------------------------------------------------------
int SoccerSetupScreen::getNumConfirmedKarts()
{
    int confirmed_karts = 0;
    int nb_players = (int)m_kart_view_info.size();
    for (int i = 0; i < nb_players; i++)
    {
        if (m_kart_view_info[i].confirmed == true)
            confirmed_karts++;
    }
    return confirmed_karts;
}

// -----------------------------------------------------------------------------
void SoccerSetupScreen::updateKartViewsLayout()
{
    Widget* central_div = getWidget<Widget>("central_div");

    // Compute/get some dimensions
    const int nb_columns = 2;   // two karts maximum per column
    const int kart_area_width = (int)((central_div->m_w) / 2 * 0.8f); // size of one half of the screen with some margin
    const int kart_view_size = kart_area_width / nb_columns;  // Size (width and height) of a kart view
    const int center_x = central_div->m_x + central_div->m_w / 2;
    const int center_y = central_div->m_y + central_div->m_h / 2;

    // Get the teams
    const std::vector<KartTeam>& teams = RaceManager::get()->getTeamsInGame();
    const int num_teams = teams.size();

    // Ensure there are exactly 2 teams
    if (num_teams != 2) {
        Log::error("SoccerSetupScreen", "Expected exactly 2 teams, but got %d teams", num_teams);
        return;
    }

    // Count the number of karts per team
    int nb_players = (int)m_kart_view_info.size();
    std::map<KartTeam, int> nb_karts_per_team;
    for (KartTeam team : teams) {
        nb_karts_per_team[team] = 0;
    }
    for (int i = 0; i < nb_players; i++) {
        nb_karts_per_team[m_kart_view_info[i].team]++;
    }

    // - number of rows displayed for each team = ceil(nb_karts_per_team[i] / nb_columns)
    std::map<KartTeam, int> nb_rows_per_team;
    std::map<KartTeam, int> start_y;
    std::map<KartTeam, int> center_x_per_team;

    for (KartTeam team : teams) {
        nb_rows_per_team[team] = (nb_karts_per_team[team] + nb_columns - 1) / nb_columns;
        start_y[team] = center_y - nb_rows_per_team[team] * kart_view_size / 2;
    }

    // - center of each half-screen
    center_x_per_team[teams[0]] = (central_div->m_x + center_x) / 2;
    center_x_per_team[teams[1]] = (central_div->m_x + central_div->m_w + center_x) / 2;

    // Update the layout of the 3D views for the karts
    std::map<KartTeam, int> cur_kart_per_team;
    for (KartTeam team : teams) {
        cur_kart_per_team[team] = 0;
    }

    for (int i = 0; i < nb_players; i++)
    {
        const KartViewInfo& view_info = m_kart_view_info[i];
        const KartTeam team = view_info.team;

        // Compute the position
        const int cur_row = cur_kart_per_team[team] / nb_columns;
        const int pos_y = start_y[team] + cur_row * kart_view_size;

        const int cur_col = cur_kart_per_team[team] % nb_columns;
        int nb_karts_in_this_row = (nb_karts_per_team[team] - cur_row * nb_columns) % nb_columns;
        if (nb_karts_in_this_row == 0 || nb_karts_per_team[team] > 1)
            nb_karts_in_this_row = nb_columns;  // TODO: not sure of the computation here...
        const int pos_x = center_x_per_team[team] + cur_col * kart_view_size - nb_karts_in_this_row * kart_view_size / 2;
        cur_kart_per_team[team]++;

        // Move the view
        view_info.view->move(pos_x, pos_y, kart_view_size, kart_view_size);
    }
}

// -----------------------------------------------------------------------------
bool SoccerSetupScreen::onEscapePressed()
{
    RaceManager::get()->setTimeTarget(0.0f);
    return true;
}   // onEscapePressed

// -----------------------------------------------------------------------------
void SoccerSetupScreen::prepareGame()
{
    input_manager->setMasterPlayerOnly(true);
}   // prepareGame
