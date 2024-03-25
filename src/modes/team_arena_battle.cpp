#include "team_arena_battle.hpp"

#include "audio/music_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "tracks/track.hpp"
#include <utils/string_utils.hpp>
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include <vector>
#include <algorithm> 
#include <utils/translation.hpp>

TeamArenaBattle::TeamArenaBattle()
{
    setClockModeFromRaceManager();
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
    initGameInfo();
}   // init

// ----------------------------------------------------------------------------
void TeamArenaBattle::reset(bool restart)
{
    WorldWithRank::reset(restart);
    m_count_down_reached_zero = false;
    setClockModeFromRaceManager();
    initGameInfo();

    for (unsigned int i = 0; i < getNumKarts(); i++) {
        m_kart_info[i].m_lives = RaceManager::get()->getLifeTarget();
    }
}   // reset

// ----------------------------------------------------------------------------
void TeamArenaBattle::initGameInfo()
{
    m_teams.clear();
    m_teams.resize(getNumTeams());
    m_kart_info.clear();
    m_kart_info.resize(getNumKarts());
    for (unsigned int i = 0; i < getNumTeams(); i++)
    {
        m_teams[i].m_inlife_player = getTeamNum((KartTeam)i);
        m_teams[i].m_total_life = getTeamNum((KartTeam)i) * (RaceManager::get()->getLifeTarget());
        m_teams[i].m_total_player = getTeamNum((KartTeam)i);
    }
    m_winning_team = -1;
    m_team_death = 0;

    m_hasThiefMode = true; // 
    m_hasAllTeamVictoryConditions = true; // 

    configureTheifModeValue();
}

// ----------------------------------------------------------------------------
void TeamArenaBattle::setClockModeFromRaceManager()
{
    if (RaceManager::get()->hasTimeTarget())
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN, RaceManager::get()->getTimeTarget());
    else
        WorldStatus::setClockMode(CLOCK_CHRONO);
}

// ----------------------------------------------------------------------------
/** Called when the match time ends.
 */
void TeamArenaBattle::countdownReachedZero()
{
    // Prevent negative time in network team arena battle when finishing
    m_time_ticks = 0;
    m_time = 0.0f;
    m_count_down_reached_zero = true;
}   // countdownReachedZero

// ----------------------------------------------------------------------------
void TeamArenaBattle::terminateRace()
{
    //const unsigned int kart_amount = getNumKarts();
    //int size = World::getWinningTeam().size();
    //for ( int i = 0; i < kart_amount; i++)
    //{
    //    getKart(i)->finishedRace(0.0f, true/*from_server*/);
    //    if (std::find(0, size, i) != size)
    //    {
    //        m_karts[i]->getKartModel()->setAnimation(KartModel::AF_WIN_START, true/* play_non_loop*/);
    //    }
    //}   // i<kart_amount
    WorldWithRank::terminateRace();
}   // terminateRace

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
        if (RaceManager::get()->isTabLifeMode()) {
            //rank_info.m_color = video::SColor(255, 255, 0, 0);
            rank_info.m_text += core::stringw(L" (") + StringUtils::toWString(m_kart_info[i].m_lives) + L")";
            //rank_info.m_color = video::SColor(255, 0, 0, 255); // Black or blue 
            rank_info.m_text += core::stringw(L" (") + StringUtils::toWString(m_kart_info[i].m_scores) + L")";

        }
        else
            rank_info.m_text += core::stringw(L" (") + StringUtils::toWString(m_kart_info[i].m_scores) + L")";
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
    // Player and Team points
    updateScores(kart_id, hitter);

    // Player lives
    updatePlayerLives(kart_id, hitter);

    getKart(kart_id)->getKartModel()->setAnimation(KartModel::AF_LOSE_START, true/*play_non_loop*/);

    if (hitter != -1) 
    {
        if (m_hasAllTeamVictoryConditions && !RaceManager::get()->isTabLifeMode())
            calculateAllTeamVictoryConditionsPoints(hitter, getKartIdTeamIndex(hitter));
        else 
            verifyTeamWin(getKartIdTeamIndex(hitter));
    }

    kartsRankInfo();

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
void TeamArenaBattle::setKartScoreFromServer(NetworkString& ns)
{
    uint8_t kart_id = ns.getUInt8();
    int8_t hitter = ns.getUInt8();
    handleScoreInServer(kart_id, hitter);
}   // setKartScoreFromServer

// ----------------------------------------------------------------------------
int TeamArenaBattle::getTeamsKartScore(int kart_id)
{
    if (RaceManager::get()->isTabAPPMode())
        return m_teams[getKartIdTeamIndex(kart_id)].m_total_player_get_score;
    else if (RaceManager::get()->isTabLifeMode())
        return m_teams[getKartIdTeamIndex(kart_id)].m_total_life;
    else 
        return m_teams[getKartIdTeamIndex(kart_id)].m_scores_teams;
}

// ----------------------------------------------------------------------------
void TeamArenaBattle::update(int ticks)
{
    WorldWithRank::update(ticks);
    WorldWithRank::updateTrack(ticks);
    if (Track::getCurrentTrack()->hasNavMesh())
        updateSectorForKarts();

    // kartsRankInfo() ??

}   // update


// ----------------------------------------------------------------------------
video::SColor TeamArenaBattle::getColor(unsigned int kart_id) const
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
bool TeamArenaBattle::isRaceOver()
{
    if (NetworkConfig::get()->isNetworking() && NetworkConfig::get()->isClient())
        return false;

    if (m_count_down_reached_zero && RaceManager::get()->hasTimeTarget()) {
        if(m_winning_team != -2)
            TeamArenaBattle::setWinningTeams();
        return true;
    }

    if (m_winning_team != -1) 
        return true;

    // Manque la condition pour jouer la musique // Les conditions 

    return false;
}

// ----------------------------------------------------------------------------
/** Returns the internal identifier for this race. */
const std::string& TeamArenaBattle::getIdent() const
{
    if(RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_TEAM)
        return IDENT_TEAM_PT;
    else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_PLAYER)
        return IDENT_TEAM_PP;
    else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_ALL_POINTS_PLAYER)
        return IDENT_TEAM_APP;
    else 
        return IDENT_TEAM_L;
}   // getIdent

// ----------------------------------------------------------------------------
void TeamArenaBattle::saveCompleteState(BareNetworkString* bns, STKPeer* peer)
{
    // TODO : Vérifier que ça fonctionne // William Lussier 2023-10-07 12h00
    for (unsigned i = 0; i < m_teams.size(); i++)
        bns->addUInt32(m_teams[i].m_scores_teams);
}   // saveCompleteState

// ----------------------------------------------------------------------------
void TeamArenaBattle::restoreCompleteState(const BareNetworkString& b)
{
    // TODO : Vérifier que ça fonctionne // William Lussier 2023-10-07 12h00
    for (unsigned i = 0; i < m_teams.size(); i++)
        m_teams[i].m_scores_teams = b.getUInt32();
}   // restoreCompleteState

// ----------------------------------------------------------------------------
void TeamArenaBattle::setWinningTeams()
{
    // Find the highest score
    int maxScore = 0;
    for (const auto& team : m_teams) {
        if (RaceManager::get()->isTabAPPMode() || RaceManager::get()->isTabPPMode()) {
            if (team.m_total_player_get_score > maxScore) {
                maxScore = team.m_total_player_get_score;
            }
        }
        if (RaceManager::get()->isTabLifeMode()) {
            if (team.m_scores_teams > maxScore) {
                maxScore = team.m_scores_teams;
            }
        }
        else { // highest Team score
            if (team.m_scores_teams > maxScore) {
                maxScore = team.m_scores_teams;
            }
        }
    }

    // Find the indices of the occurrences of the largest value
    std::vector<int> indices;
    if (maxScore > 0) {
        for (int i = 0; i < m_teams.size(); ++i) {

            if ((RaceManager::get()->isTabTPMode() && m_teams[i].m_scores_teams == maxScore) ||
                (RaceManager::get()->isTabAPPMode() && m_teams[i].m_total_player_get_score == maxScore) ||
                (RaceManager::get()->isTabPPMode() && m_teams[i].m_total_player_get_score == maxScore) ||
                (RaceManager::get()->isTabLifeMode() && m_teams[i].m_scores_teams == maxScore)) {
                indices.push_back(i);
            }
        }
    }

    if (indices.size() > 0)
        World::setWinningTeam(indices);

    if(indices.size() >= 2)
        setWinningTeamsTexte(_("It's a tie between Teams "));
    else if (indices.size() == 0)
        setWinningTeamsTexte(_("No team Won!"));
    else if (indices.size() >= 5)
        setWinningTeamsTexte(_("It's a draw between Teams "));
    else 
        setWinningTeamsTexte(_("The %s team Wins!", setWinningTeamNameText()));

    m_winning_team = -2;
}

bool TeamArenaBattle::hasWin(int kartId)
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

// ------------------------------------------------------------------------
void TeamArenaBattle::playMusic(int8_t numP, int8_t numS)
{
    //if (getNumKarts() > numP) // && m_nb_not_zombie_player <= numS
    //{
    //    if (!m_faster_music_active)
    //    {
    //        music_manager->switchToFastMusic();
    //        m_faster_music_active = true;
    //    }
    //}

    if (!m_faster_music_active) 
    {
        if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_TEAM) {
            if (1 == 2) { // hit_capture_limit > 1 && m_teams[i].m_scores_teams >= hit_capture_limit - 1
                music_manager->switchToFastMusic();
                m_faster_music_active = true;
            }
        }
        else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_ALL_POINTS_PLAYER) {
            if (1 == 2) // m_teams[i].m_total_player > 1 && m_teams[i].m_total_player_get_score == m_teams[i].m_total_player - 1
            { 
                music_manager->switchToFastMusic();
                m_faster_music_active = true;
            }
        }
        else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_PLAYER) {
            if (1 == 2) { // Play the fast music // The condition need to change 
                music_manager->switchToFastMusic();
                m_faster_music_active = true;
            }
        }
        else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_LIFE) {
            if (getNumTeams() >= 3 && getNumTeams() - m_team_death <= 2) { // Play the fast music // The condition need to change 
                music_manager->switchToFastMusic();
                m_faster_music_active = true;
            }
        }
    }
    
}

// ------------------------------------------------------------------------
void TeamArenaBattle::verifyTeamWin(int team_id)
{
    if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_TEAM) 
    {
        if (m_teams[team_id].m_scores_teams >= hit_capture_limit)
            m_winning_team = team_id;
    }
    else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_ALL_POINTS_PLAYER) 
    {
        if (m_teams[team_id].m_total_player > 0 && m_teams[team_id].m_total_player_get_score >= m_teams[team_id].m_total_player)
            m_winning_team = team_id;
    }
    else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_POINTS_PLAYER) 
    {
        if (m_teams[team_id].m_total_player > 0 && m_teams[team_id].m_total_player_get_score == 1)
            m_winning_team = team_id;
    }
    else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TEAM_ARENA_BATTLE_LIFE) 
    {
        if (m_team_death == getNumTeams() -1 && m_teams[team_id].m_inlife_player > 0)
            m_winning_team = team_id;
    }

    if (m_winning_team != -1) 
    {
        setWinningTeamsTexte(_("The %s team Wins!", setWinningTeamNameText()));
        World::setWinningTeam(m_winning_team);
    }
}

// ------------------------------------------------------------------------
std::string TeamArenaBattle::getKartTeamsColorName(KartTeam teamColorName)
{ 
    return teamColorName == KART_TEAM_BLUE ? "blue" :
           teamColorName == KART_TEAM_RED ? "red" :
           teamColorName == KART_TEAM_GREEN ? "green" :
           teamColorName == KART_TEAM_ORANGE ? "orange" :
           teamColorName == KART_TEAM_YELLOW ? "yellow" :
           teamColorName == KART_TEAM_PURPLE ? "purple" :
           teamColorName == KART_TEAM_PINK ? "pink" :
           teamColorName == KART_TEAM_TURQUOISE ? "turquoise" :
           teamColorName == KART_TEAM_DARK_BLUE ? "dark_blue" :
           teamColorName == KART_TEAM_CYAN ? "cyan" :
           teamColorName == KART_TEAM_DEFAULT ? "pinky" :
           "pinky";
}

// ------------------------------------------------------------------------
void TeamArenaBattle::configureTheifModeValue() 
{
    // Initialiser le générateur de nombres aléatoires
    srand(time(nullptr));

    // Générer un nombre aléatoire entre 0 et 1
    int randomNumber = rand() % 2; // 0 ou 1
    m_hasThiefMode = randomNumber;

    RaceManager::get()->setThiefMode(m_hasThiefMode);

    RaceManager::get()->setThiefMode(true); // Pour tester 
    RaceManager::get()->setSpecialVictoryMode(true); // Pour tester 

    if (!m_hasThiefMode)
        return;

    int m_nb_point_thief = 1;
    int m_nb_point_player_lose = 1;

    bool hasMultiplierPointThiefMode = true; // Not here 
    int multiplierPointThiefNb = 1; // Usefull or not 
}

// ------------------------------------------------------------------------
void TeamArenaBattle::calculateTheifPoints(int kart_id, int hitter)
{
    int nb_thief_point;
    // À vérifier 

    // Lives  
    if (RaceManager::get()->isTABLifeMode()) 
    {
        if (m_kart_info[kart_id].m_lives > m_nb_point_thief)
            nb_thief_point = m_nb_point_thief;
        else
            nb_thief_point = m_kart_info[kart_id].m_lives;

        m_kart_info[kart_id].m_lives -= m_nb_point_thief;
        m_teams[getKartIdTeamIndex(kart_id)].m_total_life -= m_nb_point_thief;

        m_kart_info[hitter].m_lives += m_nb_point_thief;
        m_teams[getKartIdTeamIndex(hitter)].m_total_life += m_nb_point_thief;
    }
    else // Points 
    {
        if (m_kart_info[kart_id].m_scores > m_nb_point_thief)
            nb_thief_point = m_nb_point_thief;
        else
            nb_thief_point = m_kart_info[kart_id].m_scores;

        m_kart_info[kart_id].m_scores -= m_nb_point_thief;
        m_teams[getKartIdTeamIndex(kart_id)].m_scores_teams -= m_nb_point_thief;

        m_kart_info[hitter].m_scores += m_nb_point_thief;
        m_teams[getKartIdTeamIndex(hitter)].m_scores_teams += m_nb_point_thief;
    }
}

// ------------------------------------------------------------------------
void TeamArenaBattle::calculateAllTeamVictoryConditionsPoints(int player_id, int team_id)
{
    // À vérifier 
    // hasAllTeamVictoryConditions

    if (RaceManager::get()->isTabTPMode()) 
    {
        if (m_teams[team_id].m_opposing_team_touches_win_nb == getNumTeams() - 1)
            m_winning_team = team_id;
    }
    if (RaceManager::get()->isTabAPPMode()) 
    {
        if (m_teams[team_id].m_opposing_team_touches_win_nb == getTeamNum(KartTeam(player_id))) // 
            m_winning_team = team_id;
    }
    if (RaceManager::get()->isTabPPMode()) 
    {
        if (m_teams[team_id].m_opposing_team_touches_win_nb == 1 )
            m_winning_team = team_id;
    }

    if (m_winning_team != -1)
    {
        setWinningTeamsTexte(_("The %s team Wins!", setWinningTeamNameText()));
        World::setWinningTeam(m_winning_team);
    }
}

// ------------------------------------------------------------------------
void TeamArenaBattle::calculatePointsForAllTeamVictoryConditionsPoints(int team_kart_id, int hitter, int team_hitter_id, int points)
{
    // We have to find a team_hitter_id
    if (team_kart_id == -1) {
        team_hitter_id = findTeamIdForLosePoints(hitter);
        if (team_hitter_id == -1)
            return;
    }

    if (team_kart_id != -1)
    {
        m_teams[getKartIdTeamIndex(hitter)].m_opposing_team_touches[team_kart_id] += points;
        m_kart_info[hitter].m_opposing_team_touches[team_kart_id] += points;

        // Point du joueur  
        if (m_kart_info[hitter].m_opposing_team_touches[team_kart_id] == RaceManager::get()->getHitCaptureLimit() &&
            m_kart_info[hitter].m_opposing_team_touches_v[team_kart_id] == false)
        {
            m_kart_info[hitter].m_opposing_team_touches_win_nb += 1;
            m_kart_info[hitter].m_opposing_team_touches_v[team_kart_id] == true;
        }
        else if (m_kart_info[hitter].m_opposing_team_touches[team_hitter_id] < RaceManager::get()->getHitCaptureLimit() &&
                 m_kart_info[hitter].m_opposing_team_touches_v[team_hitter_id] == true)
        {
            m_kart_info[hitter].m_opposing_team_touches_win_nb -= 1;
            m_kart_info[hitter].m_opposing_team_touches_v[team_kart_id] == false;
        }


        // Point de l'équipe 
        if (RaceManager::get()->isTabAPPMode() || RaceManager::get()->isTabPPMode())
        {
            if (m_kart_info[hitter].m_opposing_team_touches_win_nb == getNumTeams() - 1 &&
                m_kart_info[hitter].m_opposing_team_touches_v[team_kart_id] == false)
            {
                m_teams[team_hitter_id].m_opposing_team_touches_win_nb += 1;
                m_teams[team_hitter_id].m_opposing_team_touches_v[team_kart_id] == true;
            }
            else if (m_kart_info[hitter].m_opposing_team_touches_win_nb != getNumTeams() - 1 &&
                     m_kart_info[hitter].m_opposing_team_touches_v[team_kart_id] == true)
            {
                m_teams[team_hitter_id].m_opposing_team_touches_win_nb -= 1;
                m_teams[team_hitter_id].m_opposing_team_touches_v[team_kart_id] == false;
            }
        }
        else if (m_teams[team_hitter_id].m_opposing_team_touches[team_kart_id] == RaceManager::get()->getHitCaptureLimit() &&
                m_teams[team_hitter_id].m_opposing_team_touches_v[team_kart_id] == false)
        {
            m_teams[team_hitter_id].m_opposing_team_touches_win_nb += 1;
            m_teams[team_hitter_id].m_opposing_team_touches_v[team_kart_id] == true;
        }
        else if (m_teams[team_hitter_id].m_opposing_team_touches[team_kart_id] < RaceManager::get()->getHitCaptureLimit() &&
                 m_teams[team_hitter_id].m_opposing_team_touches_v[team_kart_id] == true)
        {
            m_teams[team_hitter_id].m_opposing_team_touches_win_nb -= 1;
            m_teams[team_hitter_id].m_opposing_team_touches_v[team_kart_id] == false;
        }
    }
}

// ------------------------------------------------------------------------
void TeamArenaBattle::calculateMultiplierPointThiefMode()
{
    // Inutile pour le moment 

}

// ------------------------------------------------------------------------
int TeamArenaBattle::findTeamIdForLosePoints(int kart_id) 
{   // We have to find a team_hitter_id
    for (size_t i = 0; i < m_kart_info[kart_id].m_opposing_team_touches.size(); i++)
    {
        if (m_kart_info[kart_id].m_opposing_team_touches[i] >= 1) {
            return m_kart_info[kart_id].m_opposing_team_touches[i];
        }
    }
    return -1;
}

// ------------------------------------------------------------------------
void TeamArenaBattle::kartsRankInfo() 
{
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
}

// ------------------------------------------------------------------------
irr::core::stringw TeamArenaBattle::setWinningTeamNameText()
{
    std::string str = getKartTeamsColorName(KartTeam(m_winning_team)); // RaceGUIBase::
    irr::core::stringw teamColor = irr::core::stringw(str.c_str());
    teamColor = _(teamColor.c_str());
    return teamColor;
}

// ------------------------------------------------------------------------
void TeamArenaBattle::updateScores(int kart_id, int hitter) 
{
    if (hitter != -1)
    {
        m_teams[getKartIdTeamIndex(hitter)].m_scores_teams++;
        m_kart_info[hitter].m_scores++;

        if (RaceManager::get()->isTabAPPMode() || RaceManager::get()->isTabPPMode()) 
        {
            if (m_kart_info[hitter].m_scores > m_teams[getKartIdTeamIndex(hitter)].m_scores_teams)
                m_teams[getKartIdTeamIndex(hitter)].m_scores_teams = m_kart_info[hitter].m_scores;
            if (m_kart_info[hitter].m_scores >= hit_capture_limit && m_kart_info[hitter].m_has_score == false) 
            {
                m_teams[getKartIdTeamIndex(hitter)].m_total_player_get_score++;
                m_kart_info[hitter].m_has_score = true;
            }
        }

        // À la bonne place ou pas
        if (m_hasThiefMode)
            calculateTheifPoints(kart_id, hitter);

        calculatePointsForAllTeamVictoryConditionsPoints(getKartIdTeamIndex(kart_id), hitter, getKartIdTeamIndex(hitter), 1);

        getKart(hitter)->getKartModel()->setAnimation(KartModel::AF_WIN_START, true/*play_non_loop*/);
    }
    else if (m_teams[getKartIdTeamIndex(kart_id)].m_scores_teams > 0)
    {
        m_teams[getKartIdTeamIndex(kart_id)].m_scores_teams--;
        m_kart_info[kart_id].m_scores--;

        if (m_kart_info[kart_id].m_scores < hit_capture_limit && m_kart_info[kart_id].m_has_score == true) 
        { // Mal gérer 
            m_teams[getKartIdTeamIndex(kart_id)].m_total_player_get_score--;
            m_kart_info[kart_id].m_has_score = false;
        }

        // player 
        calculatePointsForAllTeamVictoryConditionsPoints(-1, kart_id, getKartIdTeamIndex(kart_id), -1);
    }
}

// ------------------------------------------------------------------------
void TeamArenaBattle::updatePlayerLives(int kart_id, int hitter) 
{
    if (RaceManager::get()->isTabLifeMode() && m_kart_info[kart_id].m_lives > 0)
    {
        m_kart_info[kart_id].m_lives--;
        m_teams[getKartIdTeamIndex(kart_id)].m_total_life--;

        if (m_kart_info[kart_id].m_lives == 0)
            m_teams[getKartIdTeamIndex(kart_id)].m_inlife_player--;
        if (m_teams[getKartIdTeamIndex(kart_id)].m_inlife_player == 0)
            m_team_death++;

        if (m_kart_info[kart_id].m_lives == 0) 
        {
            eliminateKart(kart_id, /*notify_of_elimination*/ true);
            m_karts[kart_id]->finishedRace(WorldStatus::getTime());
        }
    }
}
