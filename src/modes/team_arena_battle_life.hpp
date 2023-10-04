//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2023 SuperTuxKart-Team
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
#ifndef TEAM_ARENA_BATTLE_LIFE_HPP
#define TEAM_ARENA_BATTLE_LIFE_HPP

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

#include "modes/world_with_rank.hpp"
#include "tracks/track_object.hpp"
#include "states_screens/race_gui_base.hpp"
#include <algorithm>
#include <utility>

#include <string>



class PhysicalObject;

class TeamArenaBattlelife : public WorldWithRank {

private:


    int m_team1_scores = 0, m_team2_scores = 0, m_team3_scores = 0, m_team4_scores = 0;

    std::map<int, int> m_swatter_reset_kart_ticks;
    bool m_count_down_reached_zero = false;

    /** Profiling usage */
    int m_total_rescue;
    int m_total_hit;

    std::vector<int> m_lifes;

    // ------------------------------------------------------------------------
    virtual video::SColor getColor(unsigned int kart_id) const;
protected:
    std::vector<int> m_scores;
    std::vector<int> m_scoresTeams;
    void handleScoreInServer(int kart_id, int hitter);
public:
    // ------------------------------------------------------------------------
    TeamArenaBattlelife();
    // ------------------------------------------------------------------------
    virtual ~TeamArenaBattlelife();
    // ------------------------------------------------------------------------
    virtual void init() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void reset(bool restart = false) OVERRIDE;
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
    virtual bool hasTeams() const OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    virtual bool isRaceOver() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool kartHit(int kart_id, int hitter = -1) OVERRIDE;
    // ------------------------------------------------------------------------
    void increaseRescueCount() { m_total_rescue++; }
    // ------------------------------------------------------------------------
    virtual void countdownReachedZero() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void terminateRace() OVERRIDE;
    // ------------------------------------------------------------------------
    //virtual unsigned int getRescuePositionIndex(AbstractKart* kart) OVERRIDE;
    // 
    void setKartScoreFromServer(NetworkString& ns);
    // ------------------------------------------------------------------------
    void setKartLifeFromServer(NetworkString& ns);
    // ------------------------------------------------------------------------
    int getKartScore(int kart_id) const { return m_scores.at(kart_id); }

    int getKartLife(int kart_id) const { return m_lifes.at(kart_id); }

    int getTeamsKartScore(int kart_id);

    // ------------------------------------------------------------------------
    bool getKartFFAResult(int kart_id) const;
    // ------------------------------------------------------------------------
    bool getKartCTFResult(unsigned int kart_id) const
    {
        return false;
    }
    // ------------------------------------------------------------------------
    int getTeam1Score() const { return m_team1_scores; }
    // ------------------------------------------------------------------------
    int getTeam2Score() const { return m_team2_scores; }
    // ------------------------------------------------------------------------
    int getTeam3Score() const { return m_team3_scores; };
    // ------------------------------------------------------------------------
    int getTeam4Score() const { return m_team4_scores; };
    // ------------------------------------------------------------------------
    void resetKartForSwatterHit(int kart_id, int at_world_ticks)
    {
        m_swatter_reset_kart_ticks[kart_id] = at_world_ticks;
    }
    // ------------------------------------------------------------------------
    virtual void saveCompleteState(BareNetworkString* bns, STKPeer* peer) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void restoreCompleteState(const BareNetworkString& b) OVERRIDE;

};


#endif // TEAM_ARENA_BATTLE_LIFE_HPP