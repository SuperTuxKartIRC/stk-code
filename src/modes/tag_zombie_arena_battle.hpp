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
#ifndef TAG_ZOMBIE_ARENA_BATTLE_HPP
#define TAG_ZOMBIE_ARENA_BATTLE_HPP

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
#include <utility>
#include <ge_render_info.hpp>
#include <algorithm>
#include <string>


class TagZombieArenaBattle : public WorldWithRank {

private:
    std::map<int, int> m_swatter_reset_kart_ticks;
    bool m_count_down_reached_zero = false;
    const int8_t PNBT = 0, ZNBT = 2;
    irr::core::stringw m_winning_text;
    enum ClassesTypes {
        VITESSE,
        FORCE,
        DEFENSE,
        DEFORCE,
        CONTROLE,
        SPEED_CONTROLLER,
        JACK_OF_ALL_TRADE,
    };

    /** Profiling usage */
    int m_total_rescue = 0;
    int m_total_hit = 0;

    struct PowerInfo {
        int powerType;
        int weight;
    };

    struct BattleInfo
    {
        int  m_lifes;
        int m_nb_player_converted; // int8_t
        float m_converted_time = -1;
        bool  m_is_start_zombie;
        int8_t m_zombie_id_convert;
        int m_points_result;
        int m_nb_rescues;
        ClassesTypes m_type;
    };

    struct TeamInfo
    {
        int  m_scores_teams;
        int8_t  m_inlife_player;
    };

protected:
    int8_t m_total_player = getNumKarts();
    int8_t m_nb_not_zombie_player;
    int8_t m_nb_tags_zombie = NetworkConfig::get()->isNetworking() ? RaceManager::get()->getTagTarget() : RaceManager::get()->getNumberOfGreenAIKarts();
    float m_delay = 0;
    float m_delayItem = 15;

    KartTeam m_tag_zombie_team = KART_TEAM_GREEN;
    KartTeam m_player_team = KART_TEAM_RED;

    std::vector<TeamInfo> m_team_info; // TeamList
    std::vector<BattleInfo> m_kart_info; // KartList
    std::vector<int8_t> m_tag_zombie_list_rand; // List of random zombie at start of the game
    std::map<ClassesTypes, std::vector<PowerInfo>> m_class_power_map;

    bool m_convert_player = false;
    int m_id_player_converted;
    int m_iPower;
    
public:
    // ------------------------------------------------------------------------
    TagZombieArenaBattle();
    // ------------------------------------------------------------------------
    virtual ~TagZombieArenaBattle();
    // ------------------------------------------------------------------------
    virtual void init() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void reset(bool restart = false) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void getKartsDisplayInfo(std::vector<RaceGUIBase::KartIconDisplayInfo>* info) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual video::SColor getColor(unsigned int kart_id) const;
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
    virtual bool isRaceOver() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool kartHit(int kart_id, int hitter = -1) OVERRIDE;
    // ------------------------------------------------------------------------
    void increaseRescueCount() { m_total_rescue++; }
    // ------------------------------------------------------------------------
    virtual void countdownReachedZero() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void terminateRace() OVERRIDE;

private:
    // ------------------------------------------------------------------------
    virtual void initGameInfo();
    // ------------------------------------------------------------------------
    void handleScoreInServer(int kart_id, int hitter);
    // ------------------------------------------------------------------------
    void setKartLifeFromServer(NetworkString& ns);
    // ------------------------------------------------------------------------
    void setWinningTeams();
    // ------------------------------------------------------------------------
    virtual void setZombie(int kartId, int zombieId);
    // ------------------------------------------------------------------------
    void TagZombieArenaBattle::setSurvivor(int kartId);
    // ------------------------------------------------------------------------
    virtual bool setZombieStart();
    // ------------------------------------------------------------------------
    void changeKart(int idKart);
    // ------------------------------------------------------------------------
    // Function to generate unique random numbers and store them in m_tag_zombie_list_rand
    virtual void generateUniqueRandomNumbers();
    // ------------------------------------------------------------------------
    void calculatePoints();
    // ------------------------------------------------------------------------
    void resetKartForSwatterHit(int kart_id, int at_world_ticks)
    {
        m_swatter_reset_kart_ticks[kart_id] = at_world_ticks;
    }
    // ------------------------------------------------------------------------
    virtual void saveCompleteState(BareNetworkString* bns, STKPeer* peer) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void restoreCompleteState(const BareNetworkString& b) OVERRIDE;

public:
    // ------------------------------------------------------------------------
    int getKartNbConvertedPlayer(int kart_id) const { return m_kart_info.at(kart_id).m_nb_player_converted; }
    // ------------------------------------------------------------------------
    int getKartConvertedTime(int kart_id) const { return m_kart_info.at(kart_id).m_converted_time; }
    // ------------------------------------------------------------------------
    int getKartConverteZombie(int kart_id) const { return m_kart_info.at(kart_id).m_zombie_id_convert; }
    // ------------------------------------------------------------------------
    int getKartPointsResult(int kart_id) const { return m_kart_info.at(kart_id).m_points_result; }
    // ------------------------------------------------------------------------
    int getKartNbRescues(int kart_id) const { return m_kart_info.at(kart_id).m_nb_rescues; }
    // ------------------------------------------------------------------------
    void setKartNbRescuues(int kart_id) { m_kart_info.at(kart_id).m_nb_rescues++; }
    // ------------------------------------------------------------------------
    int getTeamInlifePlayer(int team) const { return m_team_info[(int)team].m_inlife_player; }
    // ------------------------------------------------------------------------
    void setKartsInfoFromServer(NetworkString& ns);

    // Function set() / get() the none zombie and the zombie team with the KartTeam
    // ------------------------------------------------------------------------
    void setTagZombieTeam(KartTeam team) { m_tag_zombie_team = team; };
    // ------------------------------------------------------------------------
    KartTeam getTagZombieTeam() const { return m_tag_zombie_team; }
    // ------------------------------------------------------------------------
    void setTagPlayerTeam(KartTeam team) { m_player_team = team; };
    // ------------------------------------------------------------------------
    KartTeam getTagPlayerTeam() const { return m_player_team; }
    // ------------------------------------------------------------------------
    void setWinningTeamsTexte(irr::core::stringw winningText) { m_winning_text = winningText; };
    // ------------------------------------------------------------------------
    irr::core::stringw getWinningTeamsTexte() { return m_winning_text; };
    // ------------------------------------------------------------------------
    virtual void setZombieTexte(irr::core::stringw winningText, int kartId, int zombieId);


    int getRandomPowerForClass(ClassesTypes classType);
    void distributePower(int8_t powerIndex, int* collectible_type, int* amount);
    ClassesTypes getRandomClassType();
    void initializeClassPowerMap();

    // overriding World methods
    virtual void getDefaultCollectibles(int* collectible_type, int* amount) OVERRIDE;
    virtual bool haveBonusBoxes() OVERRIDE;
    virtual bool timerPower() OVERRIDE;
    virtual void getItem(int* collectible_type, int* amount) OVERRIDE;
};

#endif // TAG_ARENA_BATTLE_HPP