#include "karts/abstract_kart.hpp"
#include "team_arena_battle.hpp"
#include "tracks/track.hpp"
#include <utils/string_utils.hpp>
#include "network/network_config.hpp"
#include "network/network_string.hpp"


TeamArenaBattle::TeamArenaBattle()
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

TeamArenaBattle::~TeamArenaBattle()
{
}

// ----------------------------------------------------------------------------
void TeamArenaBattle::init()
{
    WorldWithRank::init();
    m_display_rank = false;
    m_count_down_reached_zero = false;
    m_use_highscores = false;
}   // init

// ----------------------------------------------------------------------------
void TeamArenaBattle::reset(bool restart)
{
    WorldWithRank::reset(restart);
    m_count_down_reached_zero = false;
    if (RaceManager::get()->hasTimeTarget())
    {
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN,
            RaceManager::get()->getTimeTarget());
    }
    else
    {
        WorldStatus::setClockMode(CLOCK_CHRONO);
    }
    m_scoresTeams.clear();
    m_scoresTeams.resize(4, 0);
    m_scores.clear();
    m_scores.resize(m_karts.size(), 0);
}   // reset

// ----------------------------------------------------------------------------
/** Called when the match time ends.
 */
void TeamArenaBattle::countdownReachedZero()
{
    // Prevent negative time in network soccer when finishing
    m_time_ticks = 0;
    m_time = 0.0f;
    m_count_down_reached_zero = true;
}   // countdownReachedZero

// ----------------------------------------------------------------------------
void TeamArenaBattle::terminateRace()
{
    const unsigned int kart_amount = getNumKarts();
    for (unsigned int i = 0; i < kart_amount; i++)
    {
        getKart(i)->finishedRace(0.0f, true/*from_server*/);
    }   // i<kart_amount
    WorldWithRank::terminateRace();
}   // terminateRace
// ----------------------------------------------------------------------------
bool TeamArenaBattle::getKartFFAResult(int kart_id) const
{
    // the kart(s) which has the top score wins
    AbstractKart* k = getKartAtPosition(1);
    if (!k)
        return false;
    int top_score = getKartScore(k->getWorldKartId());
    return getKartScore(kart_id) == top_score;
}   // getKartFFAResult
// ----------------------------------------------------------------------------
void TeamArenaBattle::setKartScoreFromServer(NetworkString& ns)
{
    int kart_id = ns.getUInt8();
    int16_t score = ns.getUInt16();
    m_scores.at(kart_id) = score;
}   // setKartScoreFromServer

// ----------------------------------------------------------------------------
/** Returns the data to display in the race gui.
 */
void TeamArenaBattle::getKartsDisplayInfo(
    std::vector<RaceGUIBase::KartIconDisplayInfo>* info)
{
    const unsigned int kart_amount = getNumKarts();
    for (unsigned int i = 0; i < kart_amount; i++)
    {
        RaceGUIBase::KartIconDisplayInfo& rank_info = (*info)[i];
        rank_info.lap = -1;
        rank_info.m_outlined_font = true;
        if (getKartTeams(i) == KART_TEAM_1)
            rank_info.m_color = video::SColor(255, 255, 0, 0);
        else if (getKartTeams(i) == KART_TEAM_2)
            rank_info.m_color = video::SColor(255, 0, 0, 255);
        else if (getKartTeams(i) == KART_TEAM_3)
            rank_info.m_color = video::SColor(255, 0, 255, 0);
        else if (getKartTeams(i) == KART_TEAM_4)
            rank_info.m_color = video::SColor(255, 255, 165, 0);
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
        rank_info.m_text += core::stringw(L" (") +
            StringUtils::toWString(m_scores[i]) + L")";
    }
}   // getKartsDisplayInfo

// ----------------------------------------------------------------------------
void TeamArenaBattle::updateGraphics(float dt)
{
    World::updateGraphics(dt);
}   // updateGraphics

// ----------------------------------------------------------------------------
/** Called when a kart is hit.
 *  \param kart_id The world kart id of the kart that was hit.
 *  \param hitter The world kart id of the kart who hit(-1 if none).
 */
bool TeamArenaBattle::kartHit(int kart_id, int hitter)
{
    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient())
        return false;

    if (isRaceOver())
        return false;

    handleScoreInServer(kart_id, hitter);
    return true;
}   // kartHit

// ----------------------------------------------------------------------------
/** Called when the score of kart needs updated.
 *  \param kart_id The world kart id of the kart that was hit.
 *  \param hitter The world kart id of the kart who hit(-1 if none).
 */
void TeamArenaBattle::handleScoreInServer(int kart_id, int hitter)
{
    int new_score = 0;
    if (kart_id == hitter || hitter == -1) {
       
    }
    
        //new_score = --m_scores[kart_id];
    else {
        ++m_scores[hitter];
        switch (getKartTeamsColor(hitter))
        {
        case KART_TEAM_COLOR_RED:
            new_score = ++m_scoresTeams[0];
            break;
        case KART_TEAM_COLOR_BLUE:
            new_score = ++m_scoresTeams[1];
            break;
        case KART_TEAM_COLOR_GREEN:
            new_score = ++m_scoresTeams[2];
            break;
        case KART_TEAM_COLOR_ORANGE:
            new_score = ++m_scoresTeams[3];
            break;

        default:
            break;
        }
    }
        //new_score = ++m_scores[hitter];

    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isServer())
    {
        NetworkString p(PROTOCOL_GAME_EVENTS);
        p.setSynchronous(true);
        p.addUInt8(GameEventsProtocol::GE_BATTLE_KART_SCORE);
        if (kart_id == hitter || hitter == -1)
            p.addUInt8((uint8_t)kart_id).addUInt16((int16_t)new_score);
        else
            p.addUInt8((uint8_t)hitter).addUInt16((int16_t)new_score);
        STKHost::get()->sendPacketToAllPeers(&p, true);
    }
}   // handleScoreInServer

int TeamArenaBattle::getTeamsKartScore(int kart_id)
{
    switch (getKartTeams(kart_id))
    {
    case KART_TEAM_1:
        return m_scoresTeams[0];
        break;
    case KART_TEAM_2:
        return m_scoresTeams[1];
        break;
    case KART_TEAM_3:
        return m_scoresTeams[2];
        break;
    case KART_TEAM_4:
        return m_scoresTeams[3];
        break;

    default:
        return 0;
        break;
    }
    return 0;
}


// ----------------------------------------------------------------------------
void TeamArenaBattle::update(int ticks)
{
    WorldWithRank::update(ticks);
    WorldWithRank::updateTrack(ticks);
    if (Track::getCurrentTrack()->hasNavMesh())
        updateSectorForKarts();

    std::vector<std::pair<int, int> > ranks;
    for (unsigned i = 0; i < m_scores.size(); i++)
    {
        // For eliminated (disconnected or reserved player) make his score
        // int min so always last in rank
        int cur_score = getKart(i)->isEliminated() ?
            std::numeric_limits<int>::min() : m_scores[i];
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
video::SColor TeamArenaBattle::getColor(unsigned int kart_id) const
{
    //return GUIEngine::getSkin()->getColor("font::normal");
    return getKartTeamsColor(kart_id) == KART_TEAM_COLOR_BLUE ? (0, 0, 255, 255) :

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

        (255, 182, 193, 255);

}   // getColor

// ----------------------------------------------------------------------------
bool TeamArenaBattle::isRaceOver()
{

    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient())
        return false;

    //if (!getKartAtPosition(1))
    //    return false;

    //const int top_id = getKartAtPosition(1)->getWorldKartId();
    const int hit_capture_limit = RaceManager::get()->getHitCaptureLimit();
    
    if(m_count_down_reached_zero && RaceManager::get()->hasTimeTarget())
        return true;

    else if (hit_capture_limit > 0 && (m_scoresTeams[0] >= hit_capture_limit || m_scoresTeams[1] >= hit_capture_limit || m_scoresTeams[2] >= hit_capture_limit || m_scoresTeams[3] >= hit_capture_limit))
        return true;

    //else if(getCurrentNumKarts() == 1 || getCurrentNumPlayers() == 0) // TODO : dois aussi regarder si les joueurs encore en vie font partie de la même équipe
    //    return true;

    //return (m_count_down_reached_zero && RaceManager::get()->hasTimeTarget()) ||
    //       (hit_capture_limit != 0 && m_scores[top_id] >= hit_capture_limit)  || 
    //       (m_scoresTeams[0] > 4 || m_scoresTeams[1] > 4 || m_scoresTeams[2] > 4 || m_scoresTeams[3] > 4 );

    return false;
}   // isRaceOver

// ----------------------------------------------------------------------------
//bool TeamArenaBattle::kartHit(int kart_id, int hitter)
//{
//    return true;
//}   // kartHit

//-----------------------------------------------------------------------------
//unsigned int TeamArenaBattle::getRescuePositionIndex(AbstractKart* kart)
//{
//    return m_kart_position_map.at(kart->getWorldKartId());
//}   // getRescuePositionIndex

// ----------------------------------------------------------------------------
/** Returns the internal identifier for this race.
 */
const std::string& TeamArenaBattle::getIdent() const
{
    return IDENT_TEAM_L;
}   // getIdent

// ----------------------------------------------------------------------------
void TeamArenaBattle::saveCompleteState(BareNetworkString* bns, STKPeer* peer)
{
    // TODO : Vérifier que ça fonctionne
    for (unsigned i = 0; i < m_scoresTeams.size(); i++)
        bns->addUInt32(m_scoresTeams[i]);
}   // saveCompleteState

// ----------------------------------------------------------------------------
void TeamArenaBattle::restoreCompleteState(const BareNetworkString& b)
{
    // TODO : Vérifier que ça fonctionne
    for (unsigned i = 0; i < m_scoresTeams.size(); i++)
        m_scoresTeams[i] = b.getUInt32();
}   // restoreCompleteState
// ----------------------------------------------------------------------------
