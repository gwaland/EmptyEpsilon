#include "beamweapon.h"
#include "multiplayer_server.h"
#include "components/beamweapon.h"
#include "components/collision.h"
#include "components/docking.h"
#include "components/reactor.h"
#include "components/warpdrive.h"
#include "spaceObjects/spaceObject.h"
#include "spaceObjects/spaceship.h"
#include "spaceObjects/beamEffect.h"
#include "ecs/query.h"

void BeamWeaponSystem::update(float delta)
{
    if (!game_server) return;
    if (delta <= 0.0f) return;

    for(auto [entity, beamsys, transform, reactor, docking_port, obj] : sp::ecs::Query<BeamWeaponSys, sp::Transform, sp::ecs::optional<Reactor>, sp::ecs::optional<DockingPort>, SpaceObject*>()) {
        if (!beamsys.target) continue;
        P<SpaceShip> ship = P<SpaceObject>(obj);
        auto warp = entity.getComponent<WarpDrive>();

        for(auto& mount : beamsys.mounts) {
            if (mount.cooldown > 0.0f)
                mount.cooldown -= delta * beamsys.getSystemEffectiveness();

            P<SpaceObject> target = *beamsys.target.getComponent<SpaceObject*>();

            // Check on beam weapons only if we are on the server, have a target, and
            // not paused, and if the beams are cooled down or have a turret arc.
            if (mount.range > 0.0f && target && obj->isEnemy(target) && delta > 0.0f && (!warp || warp->current == 0.0f) && (!docking_port || docking_port->state == DockingPort::State::NotDocking))
            {
                // Get the angle to the target.
                auto diff = target->getPosition() - (transform.getPosition() + rotateVec2(glm::vec2(mount.position.x, mount.position.y), transform.getRotation()));
                float distance = glm::length(diff) - target->getRadius() / 2.0f;

                // We also only care if the target is within no more than its
                // range * 1.3, which is when we want to start rotating the turret.
                // TODO: Add a manual aim override similar to weapon tubes.
                if (distance < mount.range * 1.3f)
                {
                    float angle = vec2ToAngle(diff);
                    float angle_diff = angleDifference(mount.direction + transform.getRotation(), angle);

                    if (mount.turret_arc > 0)
                    {
                        // Get the target's angle relative to the turret's direction.
                        float turret_angle_diff = angleDifference(mount.turret_direction + transform.getRotation(), angle);

                        // If the turret can rotate ...
                        if (mount.turret_rotation_rate > 0)
                        {
                            // ... and if the target is within the turret's arc ...
                            if (fabsf(turret_angle_diff) < mount.turret_arc / 2.0f)
                            {
                                // ... rotate the turret's beam toward the target.
                                if (fabsf(angle_diff) > 0)
                                {
                                    mount.direction += (angle_diff / fabsf(angle_diff)) * std::min(mount.turret_rotation_rate * beamsys.getSystemEffectiveness(), fabsf(angle_diff));
                                }
                            // If the target is outside of the turret's arc ...
                            } else {
                                // ... rotate the turret's beam toward the turret's
                                // direction to reset it.
                                float reset_angle_diff = angleDifference(mount.direction, mount.turret_direction);

                                if (fabsf(reset_angle_diff) > 0)
                                {
                                    mount.direction += (reset_angle_diff / fabsf(reset_angle_diff)) * std::min(mount.turret_rotation_rate * beamsys.getSystemEffectiveness(), fabsf(reset_angle_diff));
                                }
                            }
                        }
                    }

                    // If the target is in the beam's arc and range, the beam has cooled
                    // down, and the beam can consume enough energy to fire ...
                    if (distance < mount.range && mount.cooldown <= 0.0f && fabsf(angle_diff) < mount.arc / 2.0f && (!reactor || reactor->use_energy(mount.energy_per_beam_fire)))
                    {
                        // ... add heat to the beam and zap the target.
                        beamsys.addHeat(mount.heat_per_beam_fire);

                        //When we fire a beam, and we hit an enemy, check if we are not scanned yet, if we are not, and we hit something that we know is an enemy or friendly,
                        //  we now know if this ship is an enemy or friend.
                        if (ship)
                            ship->didAnOffensiveAction();

                        mount.cooldown = mount.cycle_time; // Reset time of weapon

                        auto hit_location = target->getPosition() - glm::normalize(target->getPosition() - transform.getPosition()) * target->getRadius();
                        P<BeamEffect> effect = new BeamEffect();
                        effect->setSource(obj, mount.position);
                        effect->setTarget(target, hit_location);
                        effect->beam_texture = mount.texture;
                        effect->beam_fire_sound = "sfx/laser_fire.wav";
                        effect->beam_fire_sound_power = mount.damage / 6.0f;

                        DamageInfo info(obj, mount.damage_type, hit_location);
                        info.frequency = beamsys.frequency;
                        info.system_target = beamsys.system_target;
                        target->takeDamage(mount.damage, info);
                    }
                }
            // If the beam is turreted and can move, but doesn't have a target, reset it
            // if necessary.
            } else if (mount.range > 0.0f && mount.turret_arc > 0.0f && mount.direction != mount.turret_direction && mount.turret_rotation_rate > 0) {
                float reset_angle_diff = angleDifference(mount.direction, mount.turret_direction);

                if (fabsf(reset_angle_diff) > 0)
                {
                    mount.direction += (reset_angle_diff / fabsf(reset_angle_diff)) * std::min(mount.turret_rotation_rate * beamsys.getSystemEffectiveness(), fabsf(reset_angle_diff));
                }
            }
        }
    }
}