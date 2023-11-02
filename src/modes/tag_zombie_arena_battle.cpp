#include "tag_zombie_arena_battle.hpp"

#include "audio/music_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "items/item_manager.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "tracks/track.hpp"
#include <utils/string_utils.hpp>
#include "world_status.hpp"

TagZombieArenaBattle::TagZombieArenaBattle()
{
    if (RaceManager::get()->hasTimeTarget())
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN, RaceManager::get()->getTimeTarget());
    else
        WorldStatus::setClockMode(CLOCK_CHRONO);
}
TagZombieArenaBattle::~TagZombieArenaBattle()
{
}

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::init()
{
    WorldWithRank::init();
    m_display_rank = false;
    m_count_down_reached_zero = false;
    m_use_highscores = false;
    initGameInfo();
}   // init

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::reset(bool restart)
{
    WorldWithRank::reset(restart);
    m_count_down_reached_zero = false;
    if (RaceManager::get()->hasTimeTarget())
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN, RaceManager::get()->getTimeTarget());
    else
        WorldStatus::setClockMode(CLOCK_CHRONO);

    for (size_t i = 0; i < m_total_player; i++)
        changeKartTeam(i, m_player_team); // Update the team for the kart

    initGameInfo();

}   // reset

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::initGameInfo()
{
    m_nb_not_zombie_player = getTeamNum(m_player_team);
    m_total_player = getNumKarts();

    m_team_info.clear();
    m_kart_info.clear();

    m_team_info.resize(2);
    m_team_info[PNBT].m_inlife_player = getTeamNum(m_player_team);
    m_team_info[ZNBT].m_inlife_player = getTeamNum(m_player_team);

    m_kart_info.resize(m_total_player);

}   // initGameInfo

// ----------------------------------------------------------------------------
/** Called when the match time ends.
 */
void TagZombieArenaBattle::countdownReachedZero()
{
    // Prevent negative time in network soccer when finishing
    m_time_ticks = 0;
    m_time = 0.0f;
    m_count_down_reached_zero = true;
}   // countdownReachedZero

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::terminateRace()
{
    const unsigned int kart_amount = getNumKarts();
    for (unsigned int i = 0; i < kart_amount; i++)
    {
        getKart(i)->finishedRace(0.0f, true/*from_server*/);
    }   // i<kart_amount
    WorldWithRank::terminateRace();
}   // terminateRace

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::setKartsInfoFromServer(NetworkString& ns)
{
    int8_t kart_id = ns.getUInt8();
    uint8_t zombie_id = ns.getUInt8();

    if(zombie_id != -1)
        m_kart_info.at(zombie_id).m_nb_player_converted++;
    setZombie(kart_id, zombie_id);


    change = true;
    idplayer = kart_id;
}   // setKartScoreFromServer

// ----------------------------------------------------------------------------
/** Returns the data to display in the race gui.
 */
void TagZombieArenaBattle::getKartsDisplayInfo(
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
        else if (getKartTeam(i) == KART_TEAM_GREEN)
            rank_info.m_color = video::SColor(255, 0, 255, 0);
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
        if(m_kart_info[i].m_nb_player_converted > 0)
            rank_info.m_text += core::stringw(L" (") + StringUtils::toWString(m_kart_info[i].m_nb_player_converted) + L")";
    }
}   // getKartsDisplayInfo

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::updateGraphics(float dt)
{
    World::updateGraphics(dt);
}   // updateGraphics

// ----------------------------------------------------------------------------
/** Called when a kart is hit.
 *  \param kart_id The world kart id of the kart that was hit.
 *  \param hitter The world kart id of the kart who hit(-1 if none).
 */
bool TagZombieArenaBattle::kartHit(int kart_id, int hitter)
{
    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient())
        return false;

    if (isRaceOver())
        return false;

    // Pour tester et pouvoir voir la valeur // William Lussier 2023-10-27 11h46
    // Enlever la variable plus tard (directement dans le if)
    int team = (int)getKartTeam(kart_id); 
    int team2 = hitter != -1 ? (int)getKartTeam(hitter) : team;

    if (team != team2) {
        if (team2 == m_tag_zombie_team) {
            handleScoreInServer(kart_id, hitter);
        }
        else if (team == m_tag_zombie_team) {
            handleScoreInServer(hitter, kart_id);
        }
    }

    return true;
}   // kartHit

// ----------------------------------------------------------------------------
/** Called when the score of kart needs updated.
 *  \param kart_id The world kart id of the kart that was hit.
 *  \param hitter The world kart id of the kart who hit(-1 if none).
 */
void TagZombieArenaBattle::handleScoreInServer(int kart_id, int hitter)
{
    if (kart_id != hitter && hitter != -1) {
    
        setZombie(kart_id, hitter);
        if (NetworkConfig::get()->isNetworking() &&
            NetworkConfig::get()->isServer())
        {
            NetworkString p(PROTOCOL_GAME_EVENTS);
            p.setSynchronous(true);
            p.addUInt8(GameEventsProtocol::GE_BATTLE_KART_SCORE);
            p.addUInt8((uint8_t)kart_id).addUInt8((uint8_t)hitter);
            STKHost::get()->sendPacketToAllPeers(&p, true);
        }
    }
}   // handleScoreInServer

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::update(int ticks)
{
    if (!NetworkConfig::get()->isNetworking() || (NetworkConfig::get()->isNetworking() && NetworkConfig::get()->isServer())) {
        setZombieStart();
    }

    if (change) {
        if (changewait == 1) {
            //changeKart(idplayer);
            change = false;
            changewait = 0;
        }
        changewait = 1;
    }
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
            std::numeric_limits<int>::min() : m_kart_info[i].m_nb_player_converted;
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
video::SColor TagZombieArenaBattle::getColor(unsigned int kart_id) const
{
    return GUIEngine::getSkin()->getColor("font::normal");
}   // getColor

// ----------------------------------------------------------------------------
bool TagZombieArenaBattle::isRaceOver()
{
    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient())
        return false;

    if (m_nb_not_zombie_player <= 0) {
        World::setWinningTeam(m_tag_zombie_team);
        return true;
    }
    if (m_count_down_reached_zero && RaceManager::get()->hasTimeTarget()) {
        if (m_nb_not_zombie_player <= 0) {
            World::setWinningTeam(m_tag_zombie_team);
        }
        else {
            World::setWinningTeam(m_player_team);
        }
        return true;
    }
    if (m_total_player > 2 && m_nb_not_zombie_player <= m_total_player - 2) {
        if (!m_faster_music_active)
        {
            music_manager->switchToFastMusic();
            m_faster_music_active = true;
        }
    }
    return false;
}   // isRaceOver

// ----------------------------------------------------------------------------
/** Returns the internal identifier for this race.
 */
const std::string& TagZombieArenaBattle::getIdent() const
{
    return IDENT_TAG_Z;
}   // getIdent

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::saveCompleteState(BareNetworkString* bns, STKPeer* peer)
{
    // TODO : Vérifier que ça fonctionne
    for (unsigned i = 0; i < m_team_info.size(); i++)
        bns->addUInt32(m_team_info[i].m_inlife_player);
}   // saveCompleteState

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::restoreCompleteState(const BareNetworkString& b)
{
    // TODO : Vérifier que ça fonctionne
    for (unsigned i = 0; i < m_team_info.size(); i++)
        m_team_info[i].m_inlife_player = b.getUInt32();
}   // restoreCompleteState

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::setZombie(int kartId, int zombieId)
{
    changeKartTeam(kartId, m_tag_zombie_team); // Update the team for the kart
    m_nb_not_zombie_player--;
    m_kart_info[kartId].m_convertedTime = (double)getTime();
    m_kart_info[kartId].m_zombie_id_convert = zombieId;

    m_team_info[PNBT].m_inlife_player--; // m_player_team
    m_team_info[ZNBT].m_inlife_player++; // m_tag_zombie_team

    // Changement ???
    change = true;
    idplayer = kartId;

    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isServer())
    {
        NetworkString p(PROTOCOL_GAME_EVENTS);
        p.setSynchronous(true);
        p.addUInt8(GameEventsProtocol::GE_BATTLE_KART_SCORE);
        p.addUInt8((uint8_t)kartId).addUInt8((uint8_t)zombieId);
        STKHost::get()->sendPacketToAllPeers(&p, true);
    }
}

// ----------------------------------------------------------------------------
/** Generate unique random numbers for the start list of tag zombie (m_tag_zombie_list_rand)
 */
bool TagZombieArenaBattle::setZombieStart()
{  // TODO : Après avoir finit de tester : enlever les variables de types float 
    std::srand(static_cast<unsigned>(std::time(0))); // Seed the random number generator
    m_tag_zombie_list_rand.clear();
    m_tag_zombie_list_rand.reserve(m_nb_tags_zombie);

    float tempsRestant = getTime();
    float tempsTotal = RaceManager::get()->getTimeTarget();
    generateUniqueRandomNumbers();
    if (m_delay != -1 && getTime() <= (tempsTotal - m_delay))
    {
        for (int i = 0; i < m_nb_tags_zombie; i++) {
            if (m_tag_zombie_list_rand.size() <= m_nb_tags_zombie) {
                setZombie(m_tag_zombie_list_rand[i], -1);
                m_kart_info[i].m_is_start_zombie = true;
            }
        }
        m_delay = -1;
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
/** Function to generate unique random numbers and store them in m_tag_zombie_list_rand
 */
void TagZombieArenaBattle::generateUniqueRandomNumbers()
{
    m_nb_tags_zombie = m_nb_tags_zombie >= m_total_player ? m_total_player - 1 :  m_nb_tags_zombie ;

    if (m_total_player > 0) {
        for (int i = 0; i < m_nb_tags_zombie; i++) {
            int index = std::rand() % m_total_player;
            m_tag_zombie_list_rand.push_back(index);
        }
    }
}

// ----------------------------------------------------------------------------
/** Function to change the color of a kart depending of the team
 */
void TagZombieArenaBattle::changeKart(int idKart) 
{
    AbstractKart* kart = World::getWorld()->getKart(idKart);
    HandicapLevel hl = kart->getHandicap();
    KartModel* kartModel = kart->getKartModel();

    if (kartModel != nullptr) {

        std::shared_ptr<GE::GERenderInfo> ri;
        std::string idString = std::to_string(idKart);
        if (getKartTeam(idKart) == m_tag_zombie_team) {
            ri = std::make_shared<GE::GERenderInfo>(0.33f, false);
            kart->changeKartMore(idString, hl, ri);
        }
        else {
            ri = std::make_shared<GE::GERenderInfo>(1.0f, false);
            kart->changeKartMore(idString, hl, ri);
        }

    }
    else {
        // Handle the case when kartModel is null
    }
}