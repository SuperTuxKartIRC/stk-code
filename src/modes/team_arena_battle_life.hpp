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

#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "modes/world_with_rank.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/stk_host.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track.hpp"
#include "states_screens/race_gui_base.hpp"
#include "utils/string_utils.hpp"

#include <algorithm>
#include <utility>
#include <string>


class TeamArenaBattlelife : public WorldWithRank {

private:
    std::map<int, int> m_swatter_reset_kart_ticks;
    bool m_count_down_reached_zero = false;

    /** Profiling usage */
    int m_total_rescue;
    int m_total_hit;

    int life = RaceManager::get()->getLifeTarget();
    struct BattleInfo
    {
        int  m_lifes = RaceManager::get()->getLifeTarget();
    };

    struct TeamsInfo
    {
        int  m_inlife_player;
        int  m_total_life;
        int  m_total_player;
    };

    /** This vector contains an 'BattleInfo' struct for every kart in the race.*/
    std::vector<BattleInfo> m_kart_info;

    // ------------------------------------------------------------------------
    virtual video::SColor getColor(unsigned int kart_id) const;
protected:
    int m_nb_player_inlife;
    std::vector<TeamsInfo> m_teams;
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
    virtual void getKartsDisplayInfo(std::vector<RaceGUIBase::KartIconDisplayInfo>* info) OVERRIDE;
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
    virtual void countdownReachedZero() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void terminateRace() OVERRIDE;
    // ------------------------------------------------------------------------
    void setKartLifeFromServer(NetworkString& ns);
    // ------------------------------------------------------------------------
    int getKartLife(int kart_id) const { return m_kart_info.at(kart_id).m_lifes; }
    // ------------------------------------------------------------------------
    int getTeamsKartScore(int kart_id);
    // ------------------------------------------------------------------------
    int getTeamTotalLife(int team) const { return m_teams[(int)team].m_total_life; }
    // ------------------------------------------------------------------------
    int getTeamInlifePlayer(int team) const { return m_teams[(int)team].m_inlife_player; }
    // ------------------------------------------------------------------------
    void setWinningTeams();
    // ------------------------------------------------------------------------
    bool hasWin(int kartId);
    // ------------------------------------------------------------------------
    void resetKartForSwatterHit(int kart_id, int at_world_ticks)
    {
        m_swatter_reset_kart_ticks[kart_id] = at_world_ticks;
    }
    // ------------------------------------------------------------------------
    virtual std::pair<uint32_t, uint32_t> getGameStartedProgress() const OVERRIDE
    {
        // TODO : Dois être modifier pour fonctionner avec les 4 équipes (pas juste avec l'équipe 1 et 2)
        std::pair<uint32_t, uint32_t> progress(
            std::numeric_limits<uint32_t>::max(),
            std::numeric_limits<uint32_t>::max());
        if (RaceManager::get()->hasTimeTarget())
        {
            progress.first = (uint32_t)m_time;
        }
        if (m_teams[0].m_total_life > m_teams[1].m_total_life) // m_red_scores > m_blue_scores
        {
            progress.second = (uint32_t)((float)m_teams[0].m_total_life /
                (float)RaceManager::get()->getHitCaptureLimit() * 100.0f);
        }
        else
        {
            progress.second = (uint32_t)((float)m_teams[1].m_total_life /
                (float)RaceManager::get()->getHitCaptureLimit() * 100.0f);
        }
        return progress;
    }
    // ------------------------------------------------------------------------
    virtual void saveCompleteState(BareNetworkString* bns, STKPeer* peer) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void restoreCompleteState(const BareNetworkString& b) OVERRIDE;
};


#endif // TEAM_ARENA_BATTLE_LIFE_HPP