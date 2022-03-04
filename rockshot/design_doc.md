# Rocket Platform Fighters Design

level format:
min/max extents of flat, 2D platforms aligned to the XZ plane, and the platform's height on Y axis

platforms


entity bottom point is established
entity positive distance along Y from nearest platform beneath entity is measured
  if entity is above a platform, entity falls with gravity
  if entity is within epsilon along Y from the platform, entity moves to platform's height
  if entity is below platform, nobody cares, keep falling
  if entity passes some minimum Y, respawn at some maximum Y
spacebar for jump
can move around while falling through the air
there is a terminal velocity

basic collision detection with other entities (players/enemies and bullets)
Entities of the same type do not collide!


Three basic entity types:

## Platform
Axis aligned hexahedrons

Behavior: None

## Fighter
The player is a fighter
Enemies are fighters
Fighters are equipped with a rocket launcher

Behavior:
* Accept a series of inputs (move left, jump, fire weapon) to perform actions
  * in this way, a fighter can either be controlled by the player, or by AI

## Rocket
Rockets are fired by players

Behavior:
* Fly in straight lines (rays)
* Explode if they collide with a platform or fighter, or after flying a set distance

## Bullet
Fighters can also fire bullets? hitscan


Steps:
1. Platform (static) entity creation method/function
1. Fighter-platform collision
1. Fighter (dynamic) entity creation function
1. Fighter input handling (movement, jumping, firing weapon)
1. Rocket-platform collision
1. Rocket-fighter collision
1. Broad phase collision detection
