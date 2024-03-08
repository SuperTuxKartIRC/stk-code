//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2024 SuperTuxKart-Team
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
#ifndef HOT_POTATO_HPP
#define HOT_POTATO_HPP

#include "modes/world_with_rank.hpp"
#include "tracks/track_object.hpp"
#include "states_screens/race_gui_base.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/stk_host.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"
#include "karts/kart_model.hpp"
#include <ge_render_info.hpp>
#include <algorithm>
#include <utility>

#include <string>

class HotPotato : public WorldWithRank {

private:
    std::map<int, int> m_swatter_reset_kart_ticks;
    bool m_count_down_reached_zero = false;

    /** Profiling usage */
    int m_total_rescue;
    int m_total_hit;

    struct PlayerInfo
    {
        float m_total_time_with_object;
        float m_last_start_with_object; // To help calculate m_total_time_with_object
        int m_number_times_with_object;
        int m_last_player_touched = -1; // PlayerId 
        bool m_has_object;
        int m_number_points = 0; // For the victory screen
        // Usefull or not ??
        int  m_lives = RaceManager::get()->getLifeTarget();
    };

protected:
    std::vector<PlayerInfo> m_kart_info;
    int m_nb_player_inlife;
    int m_nb_total_player;
    bool m_have_to_play_speed_music = false;
    float m_duration_of_object = 15; // For dynamite (Potato) and hat
    float m_time_set_object = getTime();

    int m_last_player_id;
    int m_player_id_with_object = -1;
    //int hit_capture_limit = RaceManager::get()->getHitCaptureLimit();
public:
    // ------------------------------------------------------------------------
    HotPotato();
    // ------------------------------------------------------------------------
    virtual ~HotPotato();
    // ------------------------------------------------------------------------
    virtual void init() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void reset(bool restart = false) OVERRIDE;
    // ------------------------------------------------------------------------
    void initGameInfo();
    // ------------------------------------------------------------------------
    virtual void getKartsDisplayInfo(
        std::vector<RaceGUIBase::KartIconDisplayInfo>* info) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool raceHasLaps() OVERRIDE { return false; }
    // ------------------------------------------------------------------------
    virtual const std::string& getIdent() const OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void update(int ticks) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void updateGraphics(float dt) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool hasTeam() const OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    virtual bool hasTeamPlus() const OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    virtual bool isRaceOver() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool kartHit(int kart_id, int hitter = -1) OVERRIDE;
    // ------------------------------------------------------------------------
    void increaseRescueCount() { m_total_rescue++; }
    // ------------------------------------------------------------------------
    int getKartLife(unsigned int id) const { return m_kart_info[id].m_lives; }
    // ------------------------------------------------------------------------
    virtual void countdownReachedZero() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void terminateRace() OVERRIDE;
    // ------------------------------------------------------------------------
    void handleScoreInServer(int kart_id, int hitter);
    // ------------------------------------------------------------------------
    virtual void setKartScoreFromServer(NetworkString& ns);
    // ------------------------------------------------------------------------
    int getKartNumTimeWithObject(int kart_id) const { return m_kart_info.at(kart_id).m_number_times_with_object; }
    // ------------------------------------------------------------------------
    bool hasWin(int kartId);
    // ------------------------------------------------------------------------
    void setObjectDuration();
    // ------------------------------------------------------------------------
    void setPlayerObject();
    // ------------------------------------------------------------------------
    void setPlayerVisualChangeObject();
    // ------------------------------------------------------------------------
    void setRandomPlayer();
    // ------------------------------------------------------------------------
    void resetKartForSwatterHit(int kart_id, int at_world_ticks)
    {
        m_swatter_reset_kart_ticks[kart_id] = at_world_ticks;
    }
    // ------------------------------------------------------------------------

    // ------------------------------------------------------------------------
    virtual void addReservedKart(int kart_id) OVERRIDE
    {
        WorldWithRank::addReservedKart(kart_id);
        //m_scores.at(kart_id) = 0;
    }
    // ------------------------------------------------------------------------
    //virtual std::pair<uint32_t, uint32_t> getGameStartedProgress() const OVERRIDE
    //{
    //    return;
    //}
    // ------------------------------------------------------------------------
    virtual void saveCompleteState(BareNetworkString* bns, STKPeer* peer) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void restoreCompleteState(const BareNetworkString& b) OVERRIDE;
    float rgbToHue(int r, int g, int b) {
        float hue = 0.0f;

        // Convert RGB values to the range [0, 1]
        float r_normalized = r / 255.0f;
        float g_normalized = g / 255.0f;
        float b_normalized = b / 255.0f;

        float cmax = std::max(r_normalized, std::max(g_normalized, b_normalized));
        float cmin = std::min(r_normalized, std::min(g_normalized, b_normalized));
        float delta = cmax - cmin;

        // Calculate the hue value
        if (delta != 0) {
            if (cmax == r_normalized) {
                hue = fmod((g_normalized - b_normalized) / delta, 6.0f);
            }
            else if (cmax == g_normalized) {
                hue = (b_normalized - r_normalized) / delta + 2.0f;
            }
            else {
                hue = (r_normalized - g_normalized) / delta + 4.0f;
            }
        }

        hue *= 60.0f; // Convert hue to degrees

        if (hue < 0) {
            hue += 360.0f;
        }

        return hue;
    }
};
#endif // HOT_POTATO_HPP