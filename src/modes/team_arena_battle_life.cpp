#include "team_arena_battle_life.hpp"

#include "audio/music_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include <utils/string_utils.hpp>


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
    const unsigned int kart_amount = getNumKarts();
    m_display_rank = false;
    m_count_down_reached_zero = false;
    m_use_highscores = false;
    m_kart_info.resize(kart_amount);
    m_teams.resize(4);
    for (unsigned int i = 0; i < 4; i++)
    {
        m_teams[i].m_inlife_player = getTeamNum((KartTeam)i);
        m_teams[i].m_total_life = getTeamNum((KartTeam)i) * RaceManager::get()->getLifeTarget();
        m_teams[i].m_total_player = getTeamNum((KartTeam)i);
    }
}   // init

// ----------------------------------------------------------------------------
void TeamArenaBattlelife::reset(bool restart)
{
    //int life = RaceManager::get()->getLifeTarget() == NULL : 1 ? 5;
    WorldWithRank::reset(restart);
    m_count_down_reached_zero = false;
    if (RaceManager::get()->hasTimeTarget()) {
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN, RaceManager::get()->getTimeTarget());
    }
    else {
        WorldStatus::setClockMode(CLOCK_CHRONO);
    }
    const unsigned int kart_amount = getNumKarts();
    m_teams.clear();
    m_teams.resize(4);
    m_kart_info.clear();
    m_kart_info.resize(kart_amount);
    for (unsigned int i = 0; i < 4; i++)
    {
        m_teams[i].m_inlife_player = getTeamNum((KartTeam)i);
        m_teams[i].m_total_life = getTeamNum((KartTeam)i) * (RaceManager::get()->getLifeTarget());
        m_teams[i].m_total_player = getTeamNum((KartTeam)i);
    }

    if (NetworkConfig::get()->isNetworking())
        RaceManager::get()->setHitCaptureLimit(3);

    for (unsigned int i = 0; i < kart_amount; i++) {
        if (NetworkConfig::get()->isNetworking())
            m_kart_info[i].m_lifes = 3;
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
        irr::video::ITexture* heart_icon = irr_driver->getTexture(FileManager::GUI_ICON, "heart.png");
        //draw2DImage(heart_icon, indicator_pos, source_rect, NULL, NULL, true);
        rank_info.m_text += core::stringw(L" (") + StringUtils::toWString(m_kart_info[i].m_lifes) + L")";
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
        new_life = --m_kart_info[kart_id].m_lifes;

        m_teams[(int)getKartTeam(kart_id)].m_total_life--;
    }
    else if (getKartTeam(kart_id)!= getKartTeam(hitter)) {
        new_life = --m_kart_info[kart_id].m_lifes;

        m_teams[(int)getKartTeam(kart_id)].m_total_life--;
    }
    

    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isServer())
    {
        NetworkString p2(PROTOCOL_GAME_EVENTS);
        p2.setSynchronous(true);
        p2.addUInt8(GameEventsProtocol::GE_BATTLE_KART_SCORE);
        if (kart_id == hitter || hitter == -1)
            p2.addUInt8((uint8_t)kart_id).addUInt16((int16_t)new_life);
        else
            p2.addUInt8((uint8_t)hitter).addUInt16((int16_t)new_life);
        STKHost::get()->sendPacketToAllPeers(&p2, true);
    }
}   // handleScoreInServer

// ----------------------------------------------------------------------------
void TeamArenaBattlelife::update(int ticks)
{ // TODO : Besoins de vérifier si tout ce qui se trouve dans cette méthode est utile // William Lussier 2023-10-27 8h52
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
            std::numeric_limits<int>::min() : m_kart_info[i].m_lifes;
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

    if (m_count_down_reached_zero && RaceManager::get()->hasTimeTarget()) {
        TeamArenaBattlelife::setWinningTeams();
        return true;
    }
    int teamDeath = 0;

    for (int i = 0; i < 4; i++) // TODO : Changer le chiffre 4 par la nombre d'équipe qui sont présentement dans le mode de jeu
    {
        if (m_teams[i].m_total_player >0 && m_teams[i].m_inlife_player == 0) {
            teamDeath++;
        }
        if (teamDeath >= getNumTeams() - 1) {
            for (int i = 0; i < 4; i++)
            {
                if(m_teams[i].m_inlife_player != 0)
                    World::setWinningTeam(i);
            }
            return true;
        }
        else if (getNumTeams() >= 3 && getNumTeams() - teamDeath <= 2) {
            if (!m_faster_music_active)
            {
                music_manager->switchToFastMusic();
                m_faster_music_active = true;
            }
        }
    }
    return false;
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

void TeamArenaBattlelife::setWinningTeams()
{
    // Find the highest score
    int maxScore = 0;
    for (const auto& team : m_teams) {
        if (team.m_total_life > maxScore) {
            maxScore = team.m_total_life;
        }
    }

    // Find the indices of the occurrences of the largest value
    std::vector<int> indices;
    for (int i = 0; i < m_teams.size(); ++i) {
        if (m_teams[i].m_total_life == maxScore) {
            indices.push_back(i);
        }
    }
    if (indices.size() > 0)
        World::setWinningTeam(indices);
}