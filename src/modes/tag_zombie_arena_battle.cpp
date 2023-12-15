#include "tag_zombie_arena_battle.hpp"

#include "audio/music_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "items/item_manager.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "tracks/track.hpp"
#include <utils/string_utils.hpp>
#include "world_status.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
#include "karts/max_speed.hpp"
#include <karts/kart.hpp>

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
    AbstractKart* m_kart;
    WorldWithRank::init();
    m_display_rank = false;
    m_count_down_reached_zero = false;
    m_use_highscores = false;
    initGameInfo();

    for (size_t i = 0; i < m_total_player; i++) {
        // Besoins de changement // William Lussier 2023-11-11 16h27
        if (getKartTeam(i) == m_tag_zombie_team)
        {
            changeKart(i);
            m_kart = World::getKart(i);
        }
        changeKartTeam(i, m_player_team); // Update the team for the kart
    }
    initializeClassPowerMap();
}   // init

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::reset(bool restart)
{
    WorldWithRank::reset(restart);
    AbstractKart* m_kart;

    m_count_down_reached_zero = false;
    m_is_game_terminated = false;
    if (RaceManager::get()->hasTimeTarget())
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN, RaceManager::get()->getTimeTarget());
    else
        WorldStatus::setClockMode(CLOCK_CHRONO);

    for (size_t i = 0; i < m_total_player; i++) {
        if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TAG_ZOMBIE_SURVIROR_ARENA_BATTLE) 
        {
            if (getKartTeam(i) == m_player_team) 
            {
                changeKartTeam(i, m_tag_zombie_team);
                m_kart_info[i].m_converted_time = getTime();
                m_kart_info[i].m_is_start_zombie = true;
                if (NetworkConfig::get()->isNetworking())
                    getKartAtPosition(i + 1)->setKartTeam(m_tag_zombie_team);
                changeKart(i);
            }
        }
        else if (getKartTeam(i) == m_tag_zombie_team) 
        {
            changeKartTeam(i, m_player_team);
            if (NetworkConfig::get()->isNetworking())
                getKartAtPosition(i + 1)->setKartTeam(m_player_team);
            changeKart(i);
        }
        m_kart_info[i].m_type = getRandomClassType();
    }

    initGameInfo();

    if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TAG_ZOMBIE_SURVIROR_ARENA_BATTLE) {
        m_team_info[ZNBT].m_inlife_player = m_total_player;
        m_nb_not_zombie_player = m_nb_tags_zombie;
    }

}   // reset

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::initGameInfo()
{
    m_nb_not_zombie_player = getTeamNum(m_player_team);
    m_total_player = getNumKarts();
    m_nb_tags_zombie = m_nb_tags_zombie > m_total_player ? m_total_player - 1 : m_nb_tags_zombie; // TODO : Besoins de changement (>= ??) ?? // William Lussier 2023-11-16


    m_team_info.clear();
    m_kart_info.clear();

    m_team_info.resize(3);
    m_team_info[PNBT].m_inlife_player = getTeamNum(m_player_team);
    m_team_info[ZNBT].m_inlife_player;

    m_kart_info.resize(m_total_player);

    if (!NetworkConfig::get()->isNetworking())
    {
        m_delay = (RaceManager::get()->getTimeTarget() > 120.f) ? 15.0f : 5.0f;
    }
    else
    {
        m_delay = 10.0f;
    }
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
    uint8_t kart_id = ns.getUInt8();
    int8_t zombie_id = ns.getInt8();

    if (zombie_id == -2)
        setSurvivor(kart_id);
    else
        setZombie(kart_id, zombie_id);
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
        if (m_kart_info[i].m_nb_player_converted > 0)
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

    if (kart_id != hitter && hitter != -1 && getKartTeam(kart_id) != getKartTeam(hitter) && getKartTeam(hitter) != m_player_team) 
        handleScoreInServer(kart_id, hitter);

    return true;
}   // kartHit

// ----------------------------------------------------------------------------
/** Called when the score of kart needs updated.
 *  \param kart_id The world kart id of the kart that was hit.
 *  \param hitter The world kart id of the kart who hit(-1 if none).
 */
void TagZombieArenaBattle::handleScoreInServer(int kart_id, int hitter)
{
    m_kart_info[hitter].m_nb_player_converted++;
    setZombie(kart_id, hitter);
}   // handleScoreInServer

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::update(int ticks)
{
    if (!NetworkConfig::get()->isNetworking() || (NetworkConfig::get()->isNetworking() && NetworkConfig::get()->isServer()))
        setZombieStart();

    if (m_convert_player)
        changeKart(m_id_player_converted);

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

    if ((m_count_down_reached_zero && RaceManager::get()->hasTimeTarget()) || 
        (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TAG_ZOMBIE_LAST_SURVIROR_ARENA_BATTLE && m_nb_not_zombie_player == 1) ||
        m_nb_not_zombie_player == 0)
    {
        m_is_game_terminated = true;
    }

    if (m_is_game_terminated) {
        if (m_nb_not_zombie_player == 1) {
            World::setWinningTeam(m_player_team);
            for (int i = 0; i < m_kart_info.size(); i++)
            {
                if (m_kart_info[i].m_converted_time == -1) {
                    AbstractKart* kart = World::getWorld()->getKart(i);
                    setWinningTeamsTexte(_("The survivor (%s) Win!", kart->getName()));
                    break;
                }
            }
        }
        else if (m_nb_not_zombie_player == 0) {
            World::setWinningTeam(m_tag_zombie_team);
            setWinningTeamsTexte(_("The zombie team Wins!"));
        }
        else {
            World::setWinningTeam(m_player_team);
            setWinningTeamsTexte(_("The survivor team Wins!"));
        }
        calculatePoints();
        return true;
    }

    playMusic(2,2);

    return false;
}   // isRaceOver

// ----------------------------------------------------------------------------
/** Returns the internal identifier for this race.
 */
const std::string& TagZombieArenaBattle::getIdent() const
{
    if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TAG_ZOMBIE_ARENA_BATTLE)
        return IDENT_TAG_Z;
    else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TAG_ZOMBIE_SURVIROR_ARENA_BATTLE)
        return IDENT_TAG_Z_S;
    else
        return IDENT_TAG_Z_LS;
}   // getIdent

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::saveCompleteState(BareNetworkString* bns, STKPeer* peer)
{
    // TODO : V�rifier que �a fonctionne
    for (unsigned i = 0; i < m_team_info.size(); i++)
        bns->addUInt32(m_team_info[i].m_inlife_player);
}   // saveCompleteState

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::restoreCompleteState(const BareNetworkString& b)
{
    // TODO : V�rifier que �a fonctionne
    for (unsigned i = 0; i < m_team_info.size(); i++)
        m_team_info[i].m_inlife_player = b.getUInt32();
}   // restoreCompleteState

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::setZombie(int kartId, int zombieId)
{
    changeKartTeam(kartId, m_tag_zombie_team); // Update the team for the kart

    if (NetworkConfig::get()->isNetworking()) 
        getKartAtPosition(kartId + 1)->setKartTeam(m_tag_zombie_team);

    m_nb_not_zombie_player--;
    m_kart_info[kartId].m_converted_time = getTime();
    m_kart_info[kartId].m_zombie_id_convert = zombieId;
    if (m_nb_not_zombie_player == 0)
        m_kart_info[kartId].m_points_result = -1;
    m_team_info[PNBT].m_inlife_player--; // m_player_team
    m_team_info[ZNBT].m_inlife_player++; // m_tag_zombie_team

    // Changement ???
    m_convert_player = true;
    m_id_player_converted = kartId;

    setZombieTexte("", kartId, zombieId);

    bool test = NetworkConfig::get()->isServer();
    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isServer())
    {
        NetworkString p(PROTOCOL_GAME_EVENTS);
        p.setSynchronous(true);
        p.addUInt8(GameEventsProtocol::GE_BATTLE_KART_SCORE);
        p.addUInt8((uint8_t)kartId).addInt24((int8_t)zombieId);
        STKHost::get()->sendPacketToAllPeers(&p, true);
    }

    /*for (size_t i = 0; i < m_kart_info.size(); ++i)
    {
        int random = rand() % 50 + 1;
        if (random > 0 && random <= 12)
            m_kart_info[i].m_type = VITESSE;
        else if (random > 12 && random <= 24)
            m_kart_info[i].m_type = FORCE;
        else if (random > 24 && random <= 36)
            m_kart_info[i].m_type = DEFENSE;
        else if (random > 36 && random <= 48)
            m_kart_info[i].m_type = CONTROLE;
        else if (random > 48 && random <= 50)
            m_kart_info[i].m_type = JACK_OF_ALL_TRADE;
    }*/
}

// ----------------------------------------------------------------------------
void TagZombieArenaBattle::setSurvivor(int kartId)
{
    changeKartTeam(kartId, m_player_team); // Update the team for the kart

    if (NetworkConfig::get()->isNetworking()) {
        getKartAtPosition(kartId + 1)->setKartTeam(m_player_team);
    }

    m_team_info[PNBT].m_inlife_player++; // m_player_team
    m_team_info[ZNBT].m_inlife_player--; // m_tag_zombie_team

    m_kart_info[kartId].m_converted_time = -1;
    m_kart_info[kartId].m_type = POWERFULL_SURVIROR;

    // Changement de la type de classe 
    //m_kart_info[kartId].m_type = ;
    m_iPower = kartId;

    //int *collectible_type = (int*)PowerupManager::POWERUP_ZIPPER;
    //int *amount = (int*)10;
    //getItem(collectible_type, amount);

    // Changement ???
    m_convert_player = true;
    m_id_player_converted = kartId;

    setZombieTexte("", kartId, -2);

    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isServer())
    {
        NetworkString p(PROTOCOL_GAME_EVENTS);
        p.setSynchronous(true);
        p.addUInt8(GameEventsProtocol::GE_BATTLE_KART_SCORE);
        p.addUInt8((uint8_t)kartId).addInt24((int8_t)-2);
        STKHost::get()->sendPacketToAllPeers(&p, true);
    }
}

// ----------------------------------------------------------------------------
bool TagZombieArenaBattle::setZombieStart()
{
    float tempsTotal = RaceManager::get()->getTimeTarget();
    if (m_delay != -1 && ((tempsTotal > 0 && getTime() <= (tempsTotal - m_delay)) ||
        (tempsTotal == 0 && getTime() >= m_delay)))
    {
        generateUniqueRandomNumbers();
        for (int8_t i = 0; i < m_nb_tags_zombie; i++) {
            if (m_tag_zombie_list_rand.size() <= m_nb_tags_zombie) {
                if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TAG_ZOMBIE_SURVIROR_ARENA_BATTLE)
                    setSurvivor(m_tag_zombie_list_rand[i]);
                else
                    setZombie(m_tag_zombie_list_rand[i], -1);
                m_kart_info[m_tag_zombie_list_rand[i]].m_is_start_zombie = true;
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
{ // Pourrais planter si le nombre de joueur total (m_total_player) change en ligne 
    std::srand(static_cast<unsigned>(std::time(0))); // Seed the random number generator

    m_tag_zombie_list_rand.clear();
    m_tag_zombie_list_rand.reserve(m_nb_tags_zombie);

    if (m_total_player > 0) {
        std::set<int> selectedIndices; // Utiliser un ensemble pour �viter les doublons

        while (selectedIndices.size() < m_nb_tags_zombie) {
            int index = std::rand() % m_total_player;

            // V�rifier si l'index n'a pas d�j� �t� s�lectionn�
            if (selectedIndices.find(index) == selectedIndices.end()) {
                // L'index n'a pas �t� s�lectionn�, l'ajouter � la liste
                selectedIndices.insert(index);
                m_tag_zombie_list_rand.push_back(index);
            }
        }
    }
}

void TagZombieArenaBattle::playMusic(int8_t numP, int8_t numS)
{
    if (m_total_player > numP && m_nb_not_zombie_player <= numS) 
    {
        if (!m_faster_music_active)
        {
            music_manager->switchToFastMusic();
            m_faster_music_active = true;
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
    //KartModel* kartModel = kart->getKartModel();
    std::shared_ptr<GE::GERenderInfo> ri;
    if (getKartTeam(idKart) == m_tag_zombie_team) {
        ri = std::make_shared<GE::GERenderInfo>(0.33f, false);
        if (NetworkConfig::get()->isNetworking()) {
            //kart->changeKart(kart->getIdent(), hl, ri, RaceManager::get()->getKartInfo(idKart).getKartData());
        }
        else
            kart->changeKart(kart->getIdent(), hl, ri);
    }
    else {
        ri = std::make_shared<GE::GERenderInfo>(1.0f, false);
        if (NetworkConfig::get()->isNetworking()) {
            //kart->changeKart(kart->getIdent(), hl, ri, RaceManager::get()->getKartInfo(idKart).getKartData());
        }
        else
            kart->changeKart(kart->getIdent(), hl, ri);
    }
    m_convert_player = false;
}

// ----------------------------------------------------------------------------
/** Function to calculate the points at the end of the game
 */
void TagZombieArenaBattle::calculatePoints() 
{
    if (m_has_count_points) 
    {
        return;
    }
    m_has_count_points = true;

    for (size_t i = 0; i < m_kart_info.size(); ++i) 
    {
        int points = 0;
        // +5 points pour chaque joueur survivant (�quipe rouge)
        if (getKartTeam(i) == m_player_team && !getKart(i)->isEliminated()) 
        {
            points += 5;
        }
        // calcule des points de l'�quipe zombie (�quipe verte)
        if (getKartTeam(i) == m_tag_zombie_team) 
        {
            // +3 points pour chaque joueur qui est un zombie d'origine
            if (m_kart_info[i].m_is_start_zombie && RaceManager::get()->getMinorMode() != RaceManager::MINOR_MODE_TAG_ZOMBIE_SURVIROR_ARENA_BATTLE) 
            {
                points += 3;
            }
            if (m_kart_info[i].m_points_result != -1 && !getKart(i)->isEliminated()) 
            {
                // +1 point pour chaque joueur transform� en zombie (sauf le dernier) si l'�quipe zombie gagne 
                if(getWinningTeam().at(0) == m_tag_zombie_team && m_kart_info[i].m_is_start_zombie != true)
                    points += 1;
                // +1 point pour chaque joueur que le zombie aura transform� en zombie
                if (m_kart_info[i].m_nb_player_converted > 0) 
                {
                    points += std::min(m_kart_info[i].m_nb_player_converted, 5);
                }
            }
        }

        if (points > 5) 
        {
            points = 5;
        }

        m_kart_info[i].m_points_result = points;
    }
}

void TagZombieArenaBattle::setZombieTexte(irr::core::stringw winningText, int kartId, int zombieId)
{
    core::stringw msg;
    irr::video::SColor color;

    // I18N: Show when a player gets transformed in zombie
    if (winningText == "") {
        AbstractKart* kart = getKart(kartId);
        const core::stringw& player = kart->getController()->getName();
        if (zombieId == -2) {
            kart = getKart(kartId);
            const core::stringw& zombie = kart->getController()->getName();
            msg = _("%s has been tansformed in survivor!", player);
            color = video::SColor(255, 255, 0, 0);
        }
        else if (zombieId != -1) {
            kart = getKart(zombieId);
            const core::stringw& zombie = kart->getController()->getName();
            msg = _("%s has been tansformed in zombie by %s!", player, zombie);
            color = video::SColor(255, 0, 255, 0);
        }
        else {
            msg = _("%s has been tansformed in zombie!", player);
            color = video::SColor(255, 255, 0, 0);
        }
    }
    if (m_race_gui && !msg.empty())
        m_race_gui->addMessage(msg, NULL, 2.5f, color);
}

//-----------------------------------------------------------------------------
void TagZombieArenaBattle::getDefaultCollectibles(int* collectible_type, int* amount)
{
    // in tag mode, give zippers
    if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TAG_ZOMBIE_ARENA_BATTLE && m_delay != -1)
    {
        *collectible_type = PowerupManager::POWERUP_ZIPPER;
        *amount = 1; // 1 zipper au d�bart de la partie jusqu'au premier powerup
    }
    else World::getDefaultCollectibles(collectible_type, amount);

    //set types for everyone


}   // getDefaultCollectibles

bool TagZombieArenaBattle::haveBonusBoxes()
{
    // In tag mode, if the bonus boxes is deactivated, the rechargeable powers are activated 
    return RaceManager::get()->haveBonusBoxes();
}   // haveBonusBoxes

bool TagZombieArenaBattle::timerPower()
{
    float tempsRestant = getTime();
    float tempsTotal = RaceManager::get()->getTimeTarget();

    if (tempsRestant <= (tempsTotal - m_delayItem))
    {
        m_iPower = m_kart_info.size() - 1;
        m_delayItem += 30;
        return true;
    }

    if (m_iPower > 0)
    {
        m_iPower--;
        return true;
    }
    else return World::timerPower();
}


void TagZombieArenaBattle::initializeClassPowerMap() {
    // La performance de ce code devra �tre v�rifier 
    // Initialisez la carte avec les pouvoirs disponibles pour chaque ClassesTypes
    // Exemple pour la ClassesTypes VITESSE avec deux pouvoirs possibles

    // ClassesTypes - VITESSE
    m_class_power_map[VITESSE].push_back({ PowerupManager::POWERUP_ZIPPER, 3 });
    //m_class_power_map[VITESSE].push_back({ PowerupManager::POWERUP_PARACHUTE, 1 });

    // ClassesTypes - FORCE
    m_class_power_map[FORCE].push_back({ PowerupManager::POWERUP_BOWLING, 4 });
    m_class_power_map[FORCE].push_back({ PowerupManager::POWERUP_CAKE, 1 });

    // ClassesTypes - DEFENSE
    //m_class_power_map[DEFENSE].push_back({ PowerupManager::POWERUP_ANVIL, 2 });
    m_class_power_map[DEFENSE].push_back({ PowerupManager::POWERUP_BUBBLEGUM, 4 });

    // ClassesTypes - DEFORCE
    //m_class_power_map[DEFORCE].push_back({ PowerupManager::POWERUP_ANVIL, 2 });
    //m_class_power_map[DEFORCE].push_back({ PowerupManager::POWERUP_SWATTER, 4 });

    // ClassesTypes - CONTROLE
    m_class_power_map[CONTROLE].push_back({ PowerupManager::POWERUP_PARACHUTE, 3 });
    m_class_power_map[CONTROLE].push_back({ PowerupManager::POWERUP_PLUNGER, 2 });

    // ClassesTypes - SPEED_CONTROLLER
    //m_class_power_map[SPEED_CONTROLLER].push_back({ PowerupManager::POWERUP_SWITCH, 3 });
    //m_class_power_map[SPEED_CONTROLLER].push_back({ PowerupManager::POWERUP_PLUNGER, 5 });

    // ClassesTypes - FUNNY
    m_class_power_map[FUNNY].push_back({ PowerupManager::POWERUP_SWITCH, 5 });
    //m_class_power_map[FUNNY].push_back({ PowerupManager::POWERUP_PLUNGER, 2 });

    // ClassesTypes - JACK_OF_ALL_TRADE
    m_class_power_map[JACK_OF_ALL_TRADE].push_back({ PowerupManager::POWERUP_BOWLING, 1 });
    m_class_power_map[JACK_OF_ALL_TRADE].push_back({ PowerupManager::POWERUP_ZIPPER, 1 });
    m_class_power_map[JACK_OF_ALL_TRADE].push_back({ PowerupManager::POWERUP_PARACHUTE, 1 });
    m_class_power_map[JACK_OF_ALL_TRADE].push_back({ PowerupManager::POWERUP_ANVIL, 1 });
    m_class_power_map[JACK_OF_ALL_TRADE].push_back({ PowerupManager::POWERUP_BUBBLEGUM, 1 });
    m_class_power_map[JACK_OF_ALL_TRADE].push_back({ PowerupManager::POWERUP_PLUNGER, 1 });
    m_class_power_map[JACK_OF_ALL_TRADE].push_back({ PowerupManager::POWERUP_SWITCH, 1 });
    m_class_power_map[JACK_OF_ALL_TRADE].push_back({ PowerupManager::POWERUP_CAKE, 1 });

    m_class_power_map[POWERFULL_SURVIROR].push_back({ PowerupManager::POWERUP_CAKE, 1 });
    m_class_power_map[POWERFULL_SURVIROR].push_back({ PowerupManager::POWERUP_ZIPPER, 1 });
    m_class_power_map[POWERFULL_SURVIROR].push_back({ PowerupManager::POWERUP_ANVIL, 1 });
    m_class_power_map[POWERFULL_SURVIROR].push_back({ PowerupManager::POWERUP_SWITCH, 1 });
    m_class_power_map[POWERFULL_SURVIROR].push_back({ PowerupManager::POWERUP_BUBBLEGUM, 1 });

    // Ajoutez des entr�es similaires pour d'autres ClassesTypes et leurs pouvoirs
}

int TagZombieArenaBattle::getRandomPowerForClass(ClassesTypes classType) {
    // S�lectionnez un pouvoir al�atoire en fonction de la ClassesTypes
    const auto& powerInfoList = m_class_power_map[classType];
    int totalWeight = 0;
    for (const auto& powerInfo : powerInfoList) {
        totalWeight += powerInfo.weight;
    }

    int randomWeight = rand() % totalWeight;
    int cumulativeWeight = 0;
    for (const auto& powerInfo : powerInfoList) {
        cumulativeWeight += powerInfo.weight;
        if (randomWeight < cumulativeWeight) {
            return powerInfo.powerType;
        }
    }

    // En cas d'erreur, retournez le premier pouvoir
    return powerInfoList.front().powerType;
}


// � v�rifier // Le code dois �tre v�rifier 

void TagZombieArenaBattle::distributePower(int8_t powerIndex, int* collectible_type, int* amount) {
	int powerType = getRandomPowerForClass(m_kart_info[m_iPower].m_type);
	*collectible_type = powerType;
    if(m_kart_info[m_iPower].m_type == POWERFULL_SURVIROR)
        *amount = 10;
    else 
	    *amount = 1;
}

TagZombieArenaBattle::ClassesTypes TagZombieArenaBattle::getRandomClassType() {
    // Liste des ClassesTypes avec leur poids respectif
    std::vector<std::pair<ClassesTypes, int>> classWeights = {
        {CONTROLE, 7},         // Poids de 5
        {FORCE, 7},             // Poids de 4
        {DEFENSE, 7},           // Poids de 3
        //{SPEED_CONTROLLER, 7},  // Poids de 3
        {VITESSE, 5},           // Poids de 2
        {FUNNY, 3},           // Poids de 1
        {JACK_OF_ALL_TRADE, 1}  // Poids de 1
    };

    // Calcul de la somme des poids
    int totalWeight = 0;
    for (const auto& classWeight : classWeights) {
        totalWeight += classWeight.second;
    }

    // S�lection d'un nombre al�atoire entre 0 et totalWeight
    int randomWeight = rand() % totalWeight;

    // S�lection de la classe en fonction du poids
    int cumulativeWeight = 0;
    for (const auto& classWeight : classWeights) {
        cumulativeWeight += classWeight.second;
        if (randomWeight < cumulativeWeight) {
            return classWeight.first;
        }
    }

    // En cas d'erreur, retournez la premi�re classe
    return classWeights.front().first;
}

void TagZombieArenaBattle::getItem(int* collectible_type, int* amount) {
    int8_t powerIndex = m_kart_info[m_iPower].m_type;
    distributePower(powerIndex, collectible_type, amount);
}