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
#ifndef TEAM_ARENA_BATTLE_HPP
#define TEAM_ARENA_BATTLE_HPP

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

class TeamArenaBattle : public WorldWithRank {

private:
    std::map<int, int> m_swatter_reset_kart_ticks;
    bool m_count_down_reached_zero = false;

    /** Profiling usage */
    int m_total_rescue;
    int m_total_hit;

    struct BattleInfo
    {   
        int  m_scores;
        int  m_number_of_times_hit; // 
        bool m_has_score = false;
        // For team_arena_battle_life
        int  m_lives = RaceManager::get()->getLifeTarget();
        // For team_arena_battle_all_player_points 
        std::map<int, int> m_opposing_team_touches; // Map of opposing team IDs and their number of times hit // hasAllTeamVictoryConditions
        std::map<int, bool> m_opposing_team_touches_v; // Map of opposing team IDs and if the points off a team have benn reach // hasAllTeamVictoryConditions
        int m_opposing_team_touches_win_nb;
    };

    struct TeamsInfo
    {
        int  m_scores_teams;
        int  m_total_player_get_score; // 
        int  m_total_player;
        // For team_arena_battle_life
        int  m_inlife_player;
        int  m_total_life;
        std::map<int, int> m_opposing_team_touches; // Map of opposing team IDs and their number of times hit // hasAllTeamVictoryConditions
        std::map<int, bool> m_opposing_team_touches_v; // Map of opposing team IDs and if the points off a team have benn reach // hasAllTeamVictoryConditions
        int m_opposing_team_touches_win_nb;
    };

protected:
    std::vector<BattleInfo> m_kart_info;
    std::vector<TeamsInfo> m_teams;
    int m_team_death = 0; // For battle life
    int hit_capture_limit = RaceManager::get()->getHitCaptureLimit();
    int m_winning_team = -1; // For isRaceOver()
    irr::core::stringw m_winning_text;
    bool m_hit = false; // Un test pour améliorer les performances

    // Steals a point from the other player if he has at least 1 point. 
    // The player will also lost another point. 
    bool m_hasThiefMode = false; 
    // This means, for example, 10 points for each team. 
    // Touch, for example, 10 players from each team
    bool m_hasAllTeamVictoryConditions = false;
    
    // For the thief Mode 
    int m_nb_point_thief;
    int m_nb_point_player_lose;
    // A player may steal or cause another player to lose more points 
    // depending on how many people he touches during the game. 
    bool hasMultiplierPointThiefMode = false; 
    int multiplierPointThiefNb; // Usefull or not 
public:
    // ------------------------------------------------------------------------
    TeamArenaBattle();
    // ------------------------------------------------------------------------
    virtual ~TeamArenaBattle();
    // ------------------------------------------------------------------------
    virtual void init() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void reset(bool restart = false) OVERRIDE;
    // ------------------------------------------------------------------------
    void initGameInfo();
    // ------------------------------------------------------------------------
    void setClockModeFromRaceManager();
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
    virtual video::SColor getColor(unsigned int kart_id) const;
    // ------------------------------------------------------------------------
    void handleScoreInServer(int kart_id, int hitter);
    // ------------------------------------------------------------------------
    virtual void setKartScoreFromServer(NetworkString& ns);
    // ------------------------------------------------------------------------
    int getKartScore(int kart_id) const { return m_kart_info.at(kart_id).m_scores; }
    // ------------------------------------------------------------------------
    int getTeamsKartScore(int kart_id);
    // ------------------------------------------------------------------------
    int getTeamScore(KartTeam team) const { return m_teams[getKartTeamIndex(team)].m_scores_teams; }
    // ------------------------------------------------------------------------
    int getTeamInlifePlayer(int team) const { return m_teams[getKartTeamIndex(team)].m_inlife_player; }
    // ------------------------------------------------------------------------
    int getTeamTotalLife(int team) const { return m_teams[getKartTeamIndex(team)].m_total_life; }
    // ------------------------------------------------------------------------
    int getTeamScore(int team) const 
    { 
        if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_ALL_POINTS_PLAYER || 
            RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_PLAYER) {
            return m_teams[getKartTeamIndex(team)].m_total_player_get_score;
        }
        else 
            return m_teams[getKartTeamIndex(team)].m_scores_teams;
    }
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
    virtual void addReservedKart(int kart_id) OVERRIDE
    {
        WorldWithRank::addReservedKart(kart_id);
        //m_scores.at(kart_id) = 0;
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
        if (m_teams[0].m_scores_teams > m_teams[1].m_scores_teams) // m_red_scores > m_blue_scores
        {
            progress.second = (uint32_t)((float)m_teams[0].m_scores_teams /
                (float)RaceManager::get()->getHitCaptureLimit() * 100.0f);
        }
        else
        {
            progress.second = (uint32_t)((float)m_teams[1].m_scores_teams /
                (float)RaceManager::get()->getHitCaptureLimit() * 100.0f);
        }
        return progress;
    }
    // ------------------------------------------------------------------------
    virtual void saveCompleteState(BareNetworkString* bns, STKPeer* peer) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void restoreCompleteState(const BareNetworkString& b) OVERRIDE;
    // ------------------------------------------------------------------------
    irr::core::stringw getWinningTeamsTexte() { return m_winning_text; };
private:
    // ------------------------------------------------------------------------
    std::string getKartTeamsColorName(KartTeam teamColorName); // Inutile
    // ------------------------------------------------------------------------
    void setWinningTeamsTexte(irr::core::stringw winningText) { m_winning_text = winningText; };
    // ------------------------------------------------------------------------
    void playMusic(int8_t numP, int8_t numS);
    // ------------------------------------------------------------------------
    void verifyTeamWin(int team_id);
    // ------------------------------------------------------------------------
    void configureTheifModeValue();
    // ------------------------------------------------------------------------
    void calculateMultiplierPointThiefMode();
    // ------------------------------------------------------------------------
    void calculateTheifPoints(int kart_id, int hitter);
    // ------------------------------------------------------------------------
    void calculatePointsForAllTeamVictoryConditionsPoints(int team_id, int hitter, int team_hitter_id, int points);
    // ------------------------------------------------------------------------
    void calculateAllTeamVictoryConditionsPoints(int player_id, int team_id);
    // ------------------------------------------------------------------------
    int findTeamIdForLosePoints(int kart_id);
    // ------------------------------------------------------------------------
    void kartsRankInfo();
    // ------------------------------------------------------------------------
    irr::core::stringw setWinningTeamNameText();
    // ------------------------------------------------------------------------
    void updateScores(int kart_id, int hitter);
    // ------------------------------------------------------------------------
    void updatePlayerLives(int kart_id, int hitter);
};
#endif // TEAM_ARENA_BATTLE_HPP