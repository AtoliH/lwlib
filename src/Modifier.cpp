//
//  Modifier.cpp
//  lwcalculator
//
//  Created by Fire937 on 6/18/22.
//

#include "lwlib/Modifier.hpp"
#include "lwlib/Attack.hpp"

#include <variant>

ModifierType::Element ModifierType::fromElement(::Element element)
{
    switch (element)
    {
        case ::Element::None: return ElementNone;
        case ::Element::Sun: return ElementSun;
        case ::Element::Moon: return ElementMoon;
        case ::Element::Fire: return ElementFire;
        case ::Element::Water: return ElementWater;
        case ::Element::Wood: return ElementWood;
        case ::Element::Metal: return ElementMetal;
        case ::Element::Earth: return ElementEarth;
        case ::Element::Star: return ElementStar;
            
        default: assert(!"Element doesn't exist"); return ModifierType::ElementNone;
    }
}

ModifierType::Bullet ModifierType::fromBullet(::BulletType bullet)
{
    switch (bullet)
    {
        case ::BulletType::Ofuda: return BulletOfuda;
        case ::BulletType::Normal: return BulletNormal;
        case ::BulletType::Energy: return BulletEnergy;
        case ::BulletType::Body: return BulletBody;
        case ::BulletType::Liquid: return BulletLiquid;
        case ::BulletType::Light: return BulletLight;
        case ::BulletType::Laser: return BulletLaser;
        case ::BulletType::Sharp: return BulletSharp;
        case ::BulletType::Heavy: return BulletHeavy;
        case ::BulletType::Missile: return BulletMissile;
        case ::BulletType::Slash: return BulletSlash;
        
        default: assert(!"Bullet type doesn't exist"); return BulletOfuda;
    }
}
