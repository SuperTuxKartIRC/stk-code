//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2015 Joerg Henrichs
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

#include "items/volleyball.hpp"

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "graphics/hit_sfx.hpp"
#include "graphics/material.hpp"
#include "io/xml_node.hpp"
#include "karts/kart.hpp"
#include "modes/linear_world.hpp"

#include "utils/log.hpp" //TODO: remove after debugging is done

float Volleyball::m_st_max_distance;   // maximum distance for a Volleyball ball to be attracted
float Volleyball::m_st_max_distance_squared;
float Volleyball::m_st_force_to_target;

// -----------------------------------------------------------------------------
Volleyball::Volleyball(Kart* kart)
    : Flyable(kart, PowerupManager::POWERUP_VOLLEYBALL, 50.0f /* mass */)
{
    m_has_hit_kart = false;
    m_roll_sfx = SFXManager::get()->createSoundSource("bowling_roll");
    fixSFXSplitscreen(m_roll_sfx);
    m_roll_sfx->play();
    m_roll_sfx->setLoop(true);
}   // Volleyball

// ----------------------------------------------------------------------------
/** Destructor, removes any playing sfx.
 */
Volleyball::~Volleyball()
{
    // This will stop the sfx and delete the object.
    removeRollSfx();
}   // ~Volleyball

// -----------------------------------------------------------------------------
/** Initialises this object with data from the power.xml file.
 *  \param node XML Node
 *  \param bowling The volleyball ball mesh
 */
void Volleyball::init(const XMLNode& node, scene::IMesh* volleyball)
{
    Flyable::init(node, volleyball, PowerupManager::POWERUP_VOLLEYBALL);
    m_st_max_distance = 10.0f;
    m_st_max_distance_squared = 20.0f * 20.0f;
    m_st_force_to_target = 10.0f;

    node.get("max-distance", &m_st_max_distance);
    m_st_max_distance_squared = m_st_max_distance * m_st_max_distance;

    node.get("force-to-target", &m_st_force_to_target);
}   // init

// ----------------------------------------------------------------------------
/** Updates the bowling ball ineach frame. If this function returns true, the
 *  object will be removed by the projectile manager.
 *  \param dt Time step size.
 *  \returns True of this object should be removed.
 */
bool Volleyball::updateAndDelete(int ticks)
{
    bool can_be_deleted = Flyable::updateAndDelete(ticks);
    if (can_be_deleted)
    {
        removeRollSfx();
        return true;
    }

    // Bowling balls lose energy (e.g. when hitting the track), so increase
    // the speed if the ball is too slow, but only if it's not too high (if
    // the ball is too high, it is 'pushed down', which can reduce the
    // speed, which causes the speed to increase, which in turn causes
    // the ball to fly higher and higher.
    //btTransform trans = getTrans();
    float hat = (getXYZ() - getHitPoint()).length();
    if (hat - 0.5f * m_extend.getY() < 0.01f)
    {
        const Material* material = getMaterial();
        if (!material || material->isDriveReset())
        {
            hit(NULL);
            removeRollSfx();
            return true;
        }
    }
    btVector3 v = m_body->getLinearVelocity();

    if (v.length2() < 0.1)
    {
        hit(NULL);
        removeRollSfx();
        return true;
    }

    if (m_roll_sfx && m_roll_sfx->getStatus() == SFXBase::SFX_PLAYING)
        m_roll_sfx->setPosition(getXYZ());

    return false;
}   // updateAndDelete

// -----------------------------------------------------------------------------
/** Callback from the physics in case that a kart or physical object is hit.
 *  The volleyball ball triggers an explosion when hit.
 *  \param kart The kart hit (NULL if no kart was hit).
 *  \param object The object that was hit (NULL if none).
 *  \returns True if there was actually a hit (i.e. not owner, and target is
 *           not immune), false otherwise.
 */
bool Volleyball::hit(Kart* kart, PhysicalObject* obj)
{
    bool was_real_hit = Flyable::hit(kart, obj);
    if (was_real_hit)
    {
        if (kart && kart->isShielded())
        {
            kart->decreaseShieldTime();
            return true;
        }
        else
        {
            m_has_hit_kart = kart != NULL;
            explode(kart, obj, /*hit_secondary*/false);
        }
    }
    return was_real_hit;
}   // hit

// ----------------------------------------------------------------------------
void Volleyball::removeRollSfx()
{
    if (m_roll_sfx)
    {
        m_roll_sfx->deleteSFX();
        m_roll_sfx = NULL;
    }
}   // removeRollSfx

// ----------------------------------------------------------------------------
/** Returns the hit effect object to use when this objects hits something.
 *  \returns The hit effect object, or NULL if no hit effect should be played.
 */
HitEffect* Volleyball::getHitEffect() const
{
    if (GUIEngine::isNoGraphics())
        return NULL;
    if (m_deleted_once)
        return NULL;
    if (m_has_hit_kart)
        return new HitSFX(getXYZ(), "strike");
    else
        return new HitSFX(getXYZ(), "crash");
}   // getHitEffect

// ----------------------------------------------------------------------------
void Volleyball::onFireFlyable()
{
    Flyable::onFireFlyable();

    m_has_hit_kart = false;
    float y_offset = 0.5f * m_owner->getKartLength() + m_extend.getZ() * 0.5f;

    // if the kart is looking backwards, release from the back
    if (m_owner->getControls().getLookBack())
    {
        y_offset = -y_offset;
        m_speed = -m_speed * 2;
    }
    else
    {
        float min_speed = m_speed * 4.0f;
        /* make it go faster when throwing forward
           so the player doesn't catch up with the ball
           and explode by touching it */
        m_speed = m_owner->getSpeed() + m_speed;
        if (m_speed < min_speed) m_speed = min_speed;
    }

    const Vec3& normal = m_owner->getNormal();
    createPhysics(y_offset, btVector3(0.0f, 0.0f, m_speed * 2),
        new btSphereShape(0.5f * m_extend.getY()),
        0.4f /*restitution*/,
        -70.0f * normal /*gravity*/,
        true /*rotates*/);
    // Even if the ball is fired backwards, m_speed must be positive,
    // otherwise the ball can start to vibrate when energy is added.
    m_speed = fabsf(m_speed);
    // Do not adjust the up velociy depending on height above terrain, since
    // this would disable gravity.
    setAdjustUpVelocity(false);

    // should not live forever, auto-destruct after 20 seconds
    m_max_lifespan = stk_config->time2Ticks(20);
}   // onFireFlyable
