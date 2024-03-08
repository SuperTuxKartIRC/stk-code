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
}

HotPotato::~HotPotato()
{
}

// ----------------------------------------------------------------------------
void HotPotato::init()
{
    WorldWithRank::init();
    m_display_rank = false;
    m_use_highscores = false;
    initGameInfo();
}   // init

// ----------------------------------------------------------------------------
void HotPotato::reset(bool restart)
{
    WorldWithRank::reset(restart);
    initGameInfo();
}   // reset

// ----------------------------------------------------------------------------
void HotPotato::initGameInfo()
{
    if (RaceManager::get()->hasTimeTarget())
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN, RaceManager::get()->getTimeTarget());
    else
        WorldStatus::setClockMode(CLOCK_CHRONO); // Only this line ??

    m_count_down_reached_zero = false;
    m_kart_info.clear();
    m_kart_info.resize(getNumKarts());

    m_time_set_object = getTime();
    m_nb_player_inlife = getNumKarts();
    m_nb_total_player = getNumKarts();
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
    //int size = World::getwinningteam().size();
    //if (std::find(0, size, i) != size)
    //{
    //}
    // À MODIFIER !!!
    m_karts[0]->getKartModel()->setAnimation(KartModel::AF_WIN_START, true/* play_non_loop*/);
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
        if (m_kart_info[i].m_has_object)
            rank_info.m_color = video::SColor(255, 255, 0, 0); // Better color (red)
        else 
            rank_info.m_color = video::SColor(255, 0, 0, 0); // Black color
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
            StringUtils::toWString(m_kart_info[i].m_number_times_with_object) + L")";
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

    if (hitter != -1) 
    {
        if (m_kart_info.at(kart_id).m_has_object)
            handleScoreInServer(kart_id, hitter); // First Id is the Id of the player with the object 
        else if (m_kart_info.at(hitter).m_has_object)
            handleScoreInServer(hitter, kart_id); // First Id is the Id of the player with the object 
    }
    else 
    {
        //handleScoreInServer(kart_id, hitter);
    }

    return true;
}   // kartHit

// ----------------------------------------------------------------------------
/** Called when the score of kart needs updated.
 *  \param kart_id The world kart id of the kart that was hit.
 *  \param hitter The world kart id of the kart who hit(-1 if none).
 */
void HotPotato::handleScoreInServer(int kart_id, int hitter)
{
    bool playAnimationWin = false;

    if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_KING_HAT_ARENA_BATTLE) {
        playAnimationWin = true;
    }

    if (hitter == -1) 
    {
        // Some logique 
    }

    //if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_HOT_POTATO_ARENA_BATTLE ||
    //    RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_HOT_POTATO_ARENA_BATTLE) {

    //}

    m_kart_info.at(kart_id).m_has_object = false;
    m_kart_info.at(kart_id).m_total_time_with_object += getTime() - m_kart_info.at(kart_id).m_last_start_with_object;
    
    m_kart_info.at(hitter).m_has_object = true;
    m_kart_info.at(hitter).m_last_start_with_object = getTime();
    m_kart_info.at(hitter).m_last_player_touched = kart_id;
    m_kart_info.at(hitter).m_number_times_with_object++;

    if (RaceManager::get()->hasLifeTarget()) 
    {
        if (m_kart_info.at(hitter).m_lives > 0)
            m_kart_info.at(hitter).m_lives--;
        else 
            m_karts[hitter]->finishedRace(WorldStatus::getTime());
    }

    // Autres changements 

    // Have maked a points // Animation 
    if(playAnimationWin)
        getKart(hitter)->getKartModel()->setAnimation(KartModel::AF_WIN_START, true/*play_non_loop*/);
    else 
        getKart(hitter)->getKartModel()->setAnimation(KartModel::AF_LOSE_START, true/*play_non_loop*/);


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

    float time = getTime();
    if ((RaceManager::get()->hasTimeTarget() && m_time_set_object - getTime()  > m_duration_of_object) || 
        (!RaceManager::get()->hasTimeTarget() && getTime() - m_time_set_object > m_duration_of_object)) // getTime() - m_time_set_object > m_duration_of_object
    {
        setObjectDuration();
        setPlayerObject();
    }


    std::vector<std::pair<int, int> > ranks;
    for (unsigned i = 0; i < m_kart_info.size(); i++)
    {
        // For eliminated (disconnected or reserved player) make his score
        // int min so always last in rank
        int cur_score = getKart(i)->isEliminated() ?
            std::numeric_limits<int>::min() : m_kart_info[i].m_number_times_with_object; // Inverse
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
bool HotPotato::isRaceOver()
{

    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient())
        return false;

    //if (!getKartAtPosition(1))
    //    return false;


    // Use in HOT_POTATO_TIME and KING_HAT gamemodes
    if (m_count_down_reached_zero && RaceManager::get()->hasTimeTarget()) {
        //HotPotato::setWinningTeams();
        return true;
    }

    if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_HOT_POTATO_ARENA_BATTLE) 
    {
        const unsigned int kart_amount = getNumKarts(); // m_nb_total_player
        if (m_nb_total_player > kart_amount) 
        {
            m_nb_total_player = (m_nb_total_player - kart_amount);
            m_nb_player_inlife -= (m_nb_total_player - kart_amount);
        }
        //if (m_nb_player_inlife == 1) 
        //    return true;

        if (m_nb_player_inlife <= 3)
            m_have_to_play_speed_music = true;
    }
    // Usefull or not ???
    else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_HOT_POTATO_TIME_ARENA_BATTLE)
    {
        // Do nothing ???
    }
    else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_KING_HAT_ARENA_BATTLE)
    {
        // Check for a number of points 
    }

    // La fin de partie approche 
    if (m_have_to_play_speed_music) 
    {
        if (!m_faster_music_active)
        {
            music_manager->switchToFastMusic();
            m_faster_music_active = true;
        }
    }


    return false;
}   // isRaceOver

// ----------------------------------------------------------------------------
/** Returns the internal identifier for this race. */
const std::string& HotPotato::getIdent() const
{
    if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_HOT_POTATO_ARENA_BATTLE)
        return IDENT_HOTP;
    else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_HOT_POTATO_TIME_ARENA_BATTLE)
        return IDENT_HOTP_T;
    else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_KING_HAT_ARENA_BATTLE)
        return IDENT_KING_H;
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

void HotPotato::setObjectDuration()
{
    m_duration_of_object = 20;
    m_time_set_object = getTime();
}

void HotPotato::setPlayerObject()
{
    if(m_player_id_with_object != -1)
        m_karts[m_player_id_with_object]->finishedRace(WorldStatus::getTime());
    setRandomPlayer();
    m_kart_info[m_player_id_with_object].m_has_object = true;
    setPlayerVisualChangeObject();
}

void HotPotato::setRandomPlayer()
{
    std::srand(static_cast<unsigned>(std::time(0))); // Seed the random number generator

    if (getNumKarts() > 0) {
        std::set<int> selectedIndices; // Utiliser un ensemble pour �viter les doublons

        int index = std::rand() % getNumKarts();
        if (m_last_player_id != index)
        {
            m_last_player_id = index;
            m_player_id_with_object = index;
        }
        
    }
}

void HotPotato::setPlayerVisualChangeObject() 
{
    // Set the dynamite or the hat to the player + set new color ??


    // Set Red color 
    AbstractKart* kart = World::getWorld()->getKart(m_player_id_with_object);
    HandicapLevel hl = kart->getHandicap();
    std::shared_ptr<GE::GERenderInfo> ri;
    ri = std::make_shared<GE::GERenderInfo>(1.0f, false);
    if (NetworkConfig::get()->isNetworking()) {
        //kart->changeKart(kart->getIdent(), hl, ri, RaceManager::get()->getKartInfo(idKart).getKartData());
    }
    else
        kart->changeKart(kart->getIdent(), hl, ri);
}