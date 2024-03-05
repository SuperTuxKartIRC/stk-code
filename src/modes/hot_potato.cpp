#include "hot_potato.hpp"

#include "audio/music_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "tracks/track.hpp"
#include <utils/string_utils.hpp>
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include <vector>
#include <algorithm> 

HotPotato::HotPotato()
{
    if (RaceManager::get()->hasTimeTarget())
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN, RaceManager::get()->getTimeTarget());
    else
        WorldStatus::setClockMode(CLOCK_CHRONO); // Only this line ??
}

HotPotato::~HotPotato()
{
}

// ----------------------------------------------------------------------------
void HotPotato::init()
{
    WorldWithRank::init();
    m_display_rank = false;
    m_count_down_reached_zero = false;
    m_use_highscores = false;
    initGameInfo();
}   // init

// ----------------------------------------------------------------------------
void HotPotato::reset(bool restart)
{
    WorldWithRank::reset(restart);
    m_count_down_reached_zero = false;
    if (RaceManager::get()->hasTimeTarget())
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN, RaceManager::get()->getTimeTarget());
    else
        WorldStatus::setClockMode(CLOCK_CHRONO);
    initGameInfo();
}   // reset

// ----------------------------------------------------------------------------
void HotPotato::initGameInfo()
{
    m_kart_info.clear();
    m_kart_info.resize(getNumKarts());
}

// ----------------------------------------------------------------------------
/** Called when the match time ends.
 */
void HotPotato::countdownReachedZero()
{
    // Prevent negative time in network team arena battle when finishing
    m_time_ticks = 0;
    m_time = 0.0f;
    m_count_down_reached_zero = true;
}   // countdownReachedZero

// ----------------------------------------------------------------------------
void HotPotato::terminateRace()
{
    //const unsigned int kart_amount = getNumKarts();
    //int size = World::getWinningTeam().size();
    //for ( int i = 0; i < kart_amount; i++)
    //{
    //    getKart(i)->finishedRace(0.0f, true/*from_server*/);
    //    //if (std::find(0, size, i) != size)
    //    //{
    //    //    m_karts[i]->getKartModel()->setAnimation(KartModel::AF_WIN_START, true/* play_non_loop*/);
    //    //}
    //}   // i<kart_amount
    WorldWithRank::terminateRace();
}   // terminateRace

// ----------------------------------------------------------------------------
/** Returns the data to display in the race gui.
 */
void HotPotato::getKartsDisplayInfo(
    std::vector<RaceGUIBase::KartIconDisplayInfo>* info)
{
    const unsigned int kart_amount = getNumKarts();
    for (unsigned int i = 0; i < kart_amount; i++)
    {
        RaceGUIBase::KartIconDisplayInfo& rank_info = (*info)[i];
        rank_info.lap = -1;
        rank_info.m_outlined_font = true;
        if (getKartTeam(i) == KART_TEAM_RED)
            rank_info.m_color = video::SColor(255, 255, 0, 0);
        else if (getKartTeam(i) == KART_TEAM_BLUE)
            rank_info.m_color = video::SColor(255, 0, 0, 255);
        else if (getKartTeam(i) == KART_TEAM_GREEN)
            rank_info.m_color = video::SColor(255, 0, 255, 0);
        else if (getKartTeam(i) == KART_TEAM_ORANGE)
            rank_info.m_color = video::SColor(255, 255, 165, 0);
        rank_info.m_text = getKart(i)->getController()->getName();
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
            StringUtils::toWString(m_kart_info[i].m_scores) + L")";
    }
}   // getKartsDisplayInfo

// ----------------------------------------------------------------------------
void HotPotato::updateGraphics(float dt)
{
    World::updateGraphics(dt);
}   // updateGraphics

// ----------------------------------------------------------------------------
/** Called when a kart is hit.
 *  \param kart_id The world kart id of the kart that was hit.
 *  \param hitter The world kart id of the kart who hit(-1 if none).
 */
bool HotPotato::kartHit(int kart_id, int hitter)
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
void HotPotato::handleScoreInServer(int kart_id, int hitter)
{
    //int team;
    //if (kart_id == hitter || hitter == -1) {
    //    m_kart_info[kart_id].m_scores--;
    //    team = (int)getKartTeam(kart_id);
    //    m_teams[team].m_scores_teams--;

    //    if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_ALL_POINTS_PLAYER ||
    //        RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_PLAYER) {
    //        if (m_kart_info[kart_id].get_score == true && m_kart_info[kart_id].m_scores < hit_capture_limit) {
    //            m_kart_info[kart_id].get_score = false;
    //            m_teams[team].m_total_player_get_score--;
    //        }
    //    }
    //    getKart(kart_id)->getKartModel()->setAnimation(KartModel::AF_WIN_START, true/*play_non_loop*/);
    //}
    //else {
    //    m_kart_info[hitter].m_scores++;
    //    team = (int)getKartTeam(hitter);
    //    m_teams[team].m_scores_teams++;

    //    if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_ALL_POINTS_PLAYER ||
    //        RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_PLAYER) {
    //        if (m_kart_info[hitter].get_score == false && m_kart_info[hitter].m_scores >= hit_capture_limit) {
    //            m_kart_info[hitter].get_score = true;
    //            m_teams[team].m_total_player_get_score++;
    //        }
    //    }
    //    getKart(hitter)->getKartModel()->setAnimation(KartModel::AF_WIN_START, true/*play_non_loop*/);
    //}


    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isServer())
    {
        NetworkString p(PROTOCOL_GAME_EVENTS);
        p.setSynchronous(true);
        p.addUInt8(GameEventsProtocol::GE_BATTLE_KART_SCORE);
        p.addUInt8((uint8_t)kart_id).addUInt8((int8_t)hitter);
        STKHost::get()->sendPacketToAllPeers(&p, true);
    }
}   // handleScoreInServer

// ----------------------------------------------------------------------------
void HotPotato::setKartScoreFromServer(NetworkString& ns)
{
    uint8_t kart_id = ns.getUInt8();
    int8_t hitter = ns.getUInt8();
    handleScoreInServer(kart_id, hitter);
}   // setKartScoreFromServer

// ----------------------------------------------------------------------------
void HotPotato::update(int ticks)
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
video::SColor HotPotato::getColor(unsigned int kart_id) const
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
bool HotPotato::isRaceOver()
{

    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient())
        return false;

    //if (!getKartAtPosition(1))
    //    return false;


    if (m_count_down_reached_zero && RaceManager::get()->hasTimeTarget()) {

        //HotPotato::setWinningTeams();
        return true;
    }

    //else if (hit_capture_limit > 0) {
    //    for (int i = 0; i < 4; i++) // TODO : La valeur du 4 dois être changer par le nombre d'équipe // William Lussier 2023-10-09 8h37
    //    {
    //        if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_TEAM) {
    //            if (m_teams[i].m_scores_teams >= hit_capture_limit) {
    //                // Set the winning team
    //                World::setWinningTeam(i);
    //                return true;
    //            }
    //            else if (hit_capture_limit > 1 && m_teams[i].m_scores_teams >= hit_capture_limit - 1) {
    //                if (!m_faster_music_active)
    //                {
    //                    music_manager->switchToFastMusic();
    //                    m_faster_music_active = true;
    //                }
    //            }
    //        }
    //        else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_ALL_POINTS_PLAYER) {
    //            if (m_teams[i].m_total_player > 0 && m_teams[i].m_total_player_get_score == m_teams[i].m_total_player) {
    //                // Set the winning team
    //                World::setWinningTeam(i);
    //                return true;
    //            }
    //            else if (m_teams[i].m_total_player > 1 && m_teams[i].m_total_player_get_score == m_teams[i].m_total_player - 1) {
    //                if (!m_faster_music_active)
    //                {
    //                    music_manager->switchToFastMusic();
    //                    m_faster_music_active = true;
    //                }
    //            }
    //        }
    //        else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_PLAYER) {
    //            if (m_teams[i].m_total_player > 0 && m_teams[i].m_total_player_get_score == 1) {
    //                // Set the winning team
    //                World::setWinningTeam(i);
    //                return true;
    //            }
    //        }
    //        // TODO : dois aussi regarder si les joueurs encore en vie font partie de la même équipe
    //    }
    //}

    return false;
}   // isRaceOver

// ----------------------------------------------------------------------------
/** Returns the internal identifier for this race. */
const std::string& HotPotato::getIdent() const
{
    if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_TEAM)
        return IDENT_TEAM_PT;
    else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_PLAYER)
        return IDENT_TEAM_PP;
    else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_ALL_POINTS_PLAYER)
        return IDENT_TEAM_APP;
}   // getIdent

// ----------------------------------------------------------------------------
void HotPotato::saveCompleteState(BareNetworkString* bns, STKPeer* peer)
{
    // TODO : Vérifier que ça fonctionne // William Lussier 2023-10-07 12h00
    //for (unsigned i = 0; i < m_teams.size(); i++)
    //    bns->addUInt32(m_teams[i].m_scores_teams);
}   // saveCompleteState

// ----------------------------------------------------------------------------
void HotPotato::restoreCompleteState(const BareNetworkString& b)
{
    // TODO : Vérifier que ça fonctionne // William Lussier 2023-10-07 12h00
    //for (unsigned i = 0; i < m_teams.size(); i++)
    //    m_teams[i].m_scores_teams = b.getUInt32();
}   // restoreCompleteState

bool HotPotato::hasWin(int kartId)
{
    // Obtenez la valeur de l'équipe du kartId
    int kartTeam = getKartTeam(kartId);

    // Recherchez la valeur dans la liste m_winning_teams
    auto it = std::find(m_winning_teams.begin(), m_winning_teams.end(), kartTeam);

    // Vérifiez si la valeur a été trouvée
    if (it != m_winning_teams.end())
        return true;
    else
        return false;
}
