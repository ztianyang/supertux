//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "badguy/mrtree.hpp"

#include <math.h>

#include "audio/sound_manager.hpp"
#include "badguy/poisonivy.hpp"
#include "badguy/stumpy.hpp"
#include "math/random.hpp"
#include "math/util.hpp"
#include "object/player.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"

static const float TREE_SPEED = 100;

static const float POISONIVY_WIDTH = 32;
static const float POISONIVY_HEIGHT = 32;
static const float POISONIVY_Y_OFFSET = 24;

MrTree::MrTree(const ReaderMapping& reader)
  : WalkingBadguy(reader, "images/creatures/mr_tree/mr_tree.sprite","left","right", LAYER_OBJECTS,
                  "images/objects/lightmap_light/lightmap_light-large.sprite")
{
  walk_speed = TREE_SPEED;
  max_drop_height = 16;
  SoundManager::current()->preload("sounds/mr_tree.ogg");
}

bool
MrTree::is_freezable() const
{
  return true;
}

bool
MrTree::collision_squished(GameObject& object)
{
  if (m_frozen)
    return WalkingBadguy::collision_squished(object);

  auto player = dynamic_cast<Player*>(&object);
  if (player && (player->m_does_buttjump || player->is_invincible())) {
    player->bounce(*this);
    kill_fall();
    return true;
  }

  // replace with Stumpy
  Vector stumpy_pos = get_pos();
  stumpy_pos.x += 20;
  stumpy_pos.y += 25;
  auto& stumpy = Sector::get().add<Stumpy>(stumpy_pos, m_dir);
  remove_me();

  // give Feedback
  SoundManager::current()->play("sounds/mr_tree.ogg", get_pos());
  if (player) player->bounce(*this);

  // spawn some particles
  // TODO: provide convenience function in MovingSprite or MovingObject?
  for (int px = static_cast<int>(stumpy.get_bbox().get_left()); px < static_cast<int>(stumpy.get_bbox().get_right()); px+=10) {
    Vector ppos = Vector(static_cast<float>(px),
                         static_cast<float>(stumpy.get_bbox().get_top()) - 5.0f);
    float angle = graphicsRandom.randf(-math::PI_2, math::PI_2);
    float velocity = graphicsRandom.randf(45, 90);
    float vx = sinf(angle)*velocity;
    float vy = -cosf(angle)*velocity;
    Vector pspeed = Vector(vx, vy);
    Vector paccel = Vector(0, Sector::get().get_gravity()*10);
    Sector::get().add<SpriteParticle>("images/particles/leaf.sprite",
                                           "default",
                                           ppos, ANCHOR_MIDDLE,
                                           pspeed, paccel,
                                           LAYER_OBJECTS-1);
  }

  if (!m_frozen) { //Frozen Mr.Trees don't spawn any PoisonIvys.
    // spawn PoisonIvy
    Vector leaf1_pos(stumpy_pos.x - POISONIVY_WIDTH - 1, stumpy_pos.y - POISONIVY_Y_OFFSET);
    Rectf leaf1_bbox(leaf1_pos.x, leaf1_pos.y, leaf1_pos.x + POISONIVY_WIDTH, leaf1_pos.y + POISONIVY_HEIGHT);
    if (Sector::get().is_free_of_movingstatics(leaf1_bbox, this)) {
      auto& leaf1 = Sector::get().add<PoisonIvy>(leaf1_bbox.p1(), Direction::LEFT);
      leaf1.m_countMe = false;
    }

    // spawn PoisonIvy
    Vector leaf2_pos(stumpy_pos.x + m_sprite->get_current_hitbox_width() + 1, stumpy_pos.y - POISONIVY_Y_OFFSET);
    Rectf leaf2_bbox(leaf2_pos.x, leaf2_pos.y, leaf2_pos.x + POISONIVY_WIDTH, leaf2_pos.y + POISONIVY_HEIGHT);
    if (Sector::get().is_free_of_movingstatics(leaf2_bbox, this)) {
      auto& leaf2 = Sector::get().add<PoisonIvy>(leaf2_bbox.p1(), Direction::RIGHT);
      leaf2.m_countMe = false;
    }
  }
  return true;
}

/* EOF */
