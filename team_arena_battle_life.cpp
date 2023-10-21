#include "karts/abstract_kart.hpp"
#include "team_arena_battle_life.hpp"
#include "tracks/track.hpp"
#include <utils/string_utils.hpp>
#include "network/network_config.hpp"
#include "network/network_string.hpp"


TeamArenaBattlelife::TeamArenaBattlelife()
{
    if (RaceManager::get()->hasTimeTarget())
    {
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN,
            RaceManager::get()->getTimeTarget());
    }
    else
    {
        WorldStatus::setClockMode(CLOCK_CHRONO);
    }
}

TeamArenaBattlelife::~TeamArenaBattlelife()
{
}

// ----------------------------------------------------------------------------
void TeamArenaBattlelife::init()
{
    WorldWithRank::init();
    m_display_rank = false;
    m_count_down_reached_zero = false;
    m_use_highscores = false;
    const unsigned int kart_amount = getNumKarts();
    m_kart_info.resize(m_karts.size());
    for (unsigned int i = 0; i < kart_amount; i++)
    {
        m_kart_info[i].m_lifes = RaceManager::get()->getLifeTarget();
    }
    m_teams.resize(4);
    for (unsigned int i = 0; i < 4; i++)
    {
        m_teams[i].m_inlife_player = getTeamNum((KartTeam)i);
    }
}   // init

// ----------------------------------------------------------------------------
void TeamArenaBattlelife::reset(bool restart)
{
    WorldWithRank::reset(restart);
    m_count_down_reached_zero = false;
    if (RaceManager::get()->hasTimeTarget()) {
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN, RaceManager::get()->getTimeTarget());
    }
    else {
        WorldStatus::setClockMode(CLOCK_CHRONO);
    }
    m_teams.clear();
    m_teams.resize(4);
    m_kart_info.clear();
    m_kart_info.resize(m_karts.size());
    const unsigned int kart_amount = getNumKarts();
    for (unsigned int i = 0; i < kart_amount; i++)
    {
        m_kart_info[i].m_lifes = RaceManager::get()->getLifeTarget();
    }

    for (unsigned int i = 0; i < 4; i++)
    {
        m_teams[i].m_inlife_player = getTeamNum((KartTeam)i);
    }


}   // reset

// ----------------------------------------------------------------------------
/** Called when the match time ends.
 */
void TeamArenaBattlelife::countdownReachedZero()
{
    // Prevent negative time in network soccer when finishing
    m_time_ticks = 0;
    m_time = 0.0f;
    m_count_down_reached_zero = true;
}   // countdownReachedZero

// ----------------------------------------------------------------------------
void TeamArenaBattlelife::terminateRace()
{
    const unsigned int kart_amount = getNumKarts();
    for (unsigned int i = 0; i < kart_amount; i++)
    {
        getKart(i)->finishedRace(0.0f, true/*from_server*/);
    }   // i<kart_amount
    WorldWithRank::terminateRace();
}   // terminateRace
// ----------------------------------------------------------------------------
bool TeamArenaBattlelife::getKartFFAResult(int kart_id) const
{
    // the kart(s) which has the top score wins
    
    return getKartLife(kart_id)>0;
}   // getKartFFAResult
// ----------------------------------------------------------------------------
void TeamArenaBattlelife::setKartScoreFromServer(NetworkString& ns)
{
    int kart_id = ns.getUInt8();
    int16_t score = ns.getUInt16();
    m_kart_info.at(kart_id).m_scores = score;
}   // setKartScoreFromServer
// ----------------------------------------------------------------------------
void TeamArenaBattlelife::setKartLifeFromServer(NetworkString& ns)
{
    int kart_id = ns.getUInt8();
    int16_t life = ns.getUInt16();
    m_kart_info.at(kart_id).m_lifes = life;
    
}   // setKartScoreFromServer

// ----------------------------------------------------------------------------
/** Returns the data to display in the race gui.
 */
void TeamArenaBattlelife::getKartsDisplayInfo(
    std::vector<RaceGUIBase::KartIconDisplayInfo>* info)
{
    const unsigned int kart_amount = getNumKarts();
    for (unsigned int i = 0; i < kart_amount; i++)
    {
        RaceGUIBase::KartIconDisplayInfo& rank_info = (*info)[i];
        rank_info.lap = -1;
        rank_info.m_outlined_font = true;
        //if (getKartTeam(i) == KART_TEAM_1)
        //    rank_info.m_color = video::SColor(255, 255, 0, 0);
        //else if (getKartTeams(i) == KART_TEAM_2)
        //    rank_info.m_color = video::SColor(255, 0, 0, 255);
        //else if (getKartTeams(i) == KART_TEAM_3)
        //    rank_info.m_color = video::SColor(255, 0, 255, 0);
        //else if (getKartTeams(i) == KART_TEAM_4)
        //    rank_info.m_color = video::SColor(255, 255, 165, 0);
        rank_info.m_text = getKart(i)->getController()->getName(); // A corriger plus tard 
        if (RaceManager::get()->getKartGlobalPlayerId(i) > -1)
        {
            const core::stringw& flag = StringUtils::getCountryFlag(
                RaceManager::get()->getKartInfo(i).getCountryCode());
            if (!flag.empty())
            {
                rank_info.m_text += L" ";
                rank_info.m_text += flag;
            }
        }
        rank_info.m_text += core::stringw(L" life(") +
            StringUtils::toWString(m_kart_info[i].m_lifes) + L")";
    }
}   // getKartsDisplayInfo

// ----------------------------------------------------------------------------
void TeamArenaBattlelife::updateGraphics(float dt)
{
    World::updateGraphics(dt);
}   // updateGraphics

// ----------------------------------------------------------------------------
/** Called when a kart is hit.
 *  \param kart_id The world kart id of the kart that was hit.
 *  \param hitter The world kart id of the kart who hit(-1 if none).
 */
bool TeamArenaBattlelife::kartHit(int kart_id, int hitter)
{
    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient())
        return false;

    if (isRaceOver())
        return false;
   
    handleScoreInServer(kart_id, hitter);
    int life = m_kart_info[kart_id].m_lifes;
    if (life < 1) {
        m_teams[(int)getKartTeam(kart_id)].m_inlife_player--;
        m_nb_player_inlife--;
        eliminateKart(kart_id, /*notify_of_elimination*/ true);
        m_karts[kart_id]->finishedRace(WorldStatus::getTime());

        // Find a camera of the kart with the most lives ("leader"), and
        // attach all cameras for this kart to the leader.
        // TODO : Ne fonctione pas !!! // William Lussier 2023-10-07 13h43
        int max_lives = 0;
        AbstractKart* leader = NULL;
        for (unsigned int i = 0; i < getNumKarts(); i++)
        {
            AbstractKart* const kart = getKart(i);
            if (kart->isEliminated() || kart->hasFinishedRace() ||
                kart->getWorldKartId() == (unsigned)kart_id) continue;
            if (m_kart_info[i].m_lifes > max_lives)
            {
                leader = kart;
                max_lives = m_kart_info[i].m_lifes;
            }
        }

    }
    return true;
}   // kartHit

// ----------------------------------------------------------------------------
/** Called when the score of kart needs updated.
 *  \param kart_id The world kart id of the kart that was hit.
 *  \param hitter The world kart id of the kart who hit(-1 if none).
 */
void TeamArenaBattlelife::handleScoreInServer(int kart_id, int hitter)
{
    int new_score = 0, new_life = 0;
    if (kart_id == hitter || hitter == -1) {
        new_score = --m_kart_info[kart_id].m_scores;
        new_life = --m_kart_info[kart_id].m_lifes;

        m_teams[(int)getKartTeam(kart_id)].m_scoresTeams--;
    }
    else if (getKartTeam(kart_id)!= getKartTeam(hitter)) {
        new_score = ++m_kart_info[hitter].m_scores;
        new_life = --m_kart_info[kart_id].m_lifes;

        m_teams[(int)getKartTeam(hitter)].m_scoresTeams++;
    }
    

    // TODO : Besoins de modification. Pas besoins de créer deux variable p2 ?? // William Lussier 2023-10-07 11h37
    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isServer())
    {
        //NetworkString p(PROTOCOL_GAME_EVENTS);
        //p.setSynchronous(true);
        //p.addUInt8(GameEventsProtocol::GE_BATTLE_KART_LIFE);
        //if (kart_id == hitter || hitter == -1)
        //    p.addUInt8((uint8_t)kart_id).addUInt16((int16_t)new_life);
        //else
        //    p.addUInt8((uint8_t)hitter).addUInt16((int16_t)new_life);
        //STKHost::get()->sendPacketToAllPeers(&p, true);


        NetworkString p2(PROTOCOL_GAME_EVENTS);
        p2.setSynchronous(true);
        p2.addUInt8(GameEventsProtocol::GE_BATTLE_KART_SCORE);
        if (kart_id == hitter || hitter == -1)
            p2.addUInt8((uint8_t)kart_id).addUInt16((int16_t)new_score);
        else
            p2.addUInt8((uint8_t)hitter).addUInt16((int16_t)new_score);
        STKHost::get()->sendPacketToAllPeers(&p2, true);
    }
}   // handleScoreInServer

int TeamArenaBattlelife::getTeamsKartScore(int kart_id)
{
    return m_teams[(int)getKartTeam(kart_id)].m_scoresTeams;
}


// ----------------------------------------------------------------------------
void TeamArenaBattlelife::update(int ticks)
{
    WorldWithRank::update(ticks);
    WorldWithRank::updateTrack(ticks);
    if (Track::getCurrentTrack()->hasNavMesh())
        updateSectorForKarts();

    std::vector<std::pair<int, int> > ranks;
    for (unsigned i = 0; i < m_kart_info.size(); i++)
    {
        // For eliminated (disconnected or reserved player) make his score
        // int min so always last in rank
        int cur_score = getKart(i)->isEliminated() ?
            std::numeric_limits<int>::min() : m_kart_info[i].m_scores;
        ranks.emplace_back(i, cur_score);
    }
    std::sort(ranks.begin(), ranks.end(),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b)
        {
            return a.second > b.second;
        });
    beginSetKartPositions();
    for (unsigned i = 0; i < ranks.size(); i++)
        setKartPosition(ranks[i].first, i + 1);
    endSetKartPositions();
}   // update


// ----------------------------------------------------------------------------
video::SColor TeamArenaBattlelife::getColor(unsigned int kart_id) const
{
    return GUIEngine::getSkin()->getColor("font::normal");
    /* getKartTeamsColor(kart_id) == KART_TEAM_COLOR_BLUE ? (0, 0, 255, 255) :

     KART_TEAM_COLOR_RED ? (255, 0, 0, 255) :

     KART_TEAM_COLOR_GREEN ? (0, 255, 0, 255) :

     KART_TEAM_COLOR_YELLOW ? (255, 255, 0, 255) :

     KART_TEAM_COLOR_ORANGE ? (255, 165, 0, 255) :

     KART_TEAM_COLOR_PURPLE ? (128, 0, 128, 255) :

     KART_TEAM_COLOR_PINK ? (255, 192, 203, 255) :

     KART_TEAM_COLOR_TURQUOISE ? (0, 206, 209, 255) :

     KART_TEAM_COLOR_DARK_BLUE ? (0, 0, 139, 255) :

     KART_TEAM_COLOR_CYAN ? (0, 255, 255, 255) :

     KART_TEAM_COLOR_DEFAULT ? (255, 182, 193, 255) :

     (255, 182, 193, 255);*/

}   // getColor

// ----------------------------------------------------------------------------
bool TeamArenaBattlelife::isRaceOver()
{

    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient())
        return false;

    const int hit_capture_limit = RaceManager::get()->getHitCaptureLimit();

    if (m_count_down_reached_zero && RaceManager::get()->hasTimeTarget())
        return true;
    int teamDeath = 0;

    for (int i = 0; i < 4; i++) // TODO : Changer le chiffre 4 par la nombre d'équipe qui sont présentement dans le mode de jeu
    {
        if (m_teams[i].m_inlife_player == 0) {
            teamDeath++;
        }
    }

    if (teamDeath > getNumTeams() || (m_count_down_reached_zero && RaceManager::get()->hasTimeTarget())) {
        return true;
    }
    else {
        return false;
    }
}   // isRaceOver

/** Returns the internal identifier for this race. */
const std::string& TeamArenaBattlelife::getIdent() const
{
    return IDENT_TEAM_L;
}   // getIdent

// ----------------------------------------------------------------------------
void TeamArenaBattlelife::saveCompleteState(BareNetworkString* bns, STKPeer* peer)
{
    for (unsigned i = 0; i < m_kart_info.size(); i++)
        bns->addUInt32(m_kart_info[i].m_lifes);
}   // saveCompleteState

// ----------------------------------------------------------------------------
void TeamArenaBattlelife::restoreCompleteState(const BareNetworkString& b)
{
    for (unsigned i = 0; i < m_kart_info.size(); i++)
        m_kart_info[i].m_lifes = b.getUInt32();
}   // restoreCompleteState
// ----------------------------------------------------------------------------
