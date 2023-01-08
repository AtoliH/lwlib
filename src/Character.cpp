//
//  Character.cpp
//  lwcalculator
//
//  Created by Fire937 on 6/16/22.
//

#include "lwlib/Character.hpp"

#include <random>

void Character::useSkill(TurnAction::Skill skillAction) {
    auto skill = skills.at(skillAction); // Can throw if the skill has not been set, responsability of the class to always have all three skills set, we'll see how we handle skills that are not learned yet
    applyModifiers(skill.modifiers);
}


std::vector<Character::LineEffectiveDamage>
Character::attack(const Character enemy, TurnAction::Attack attackChoice, TurnAction::Boost boost,
                  TurnAction::Graze graze) {
    std::vector<LineEffectiveDamage> led;

    // Apply story cards modifiers
    auto it = storyCards.find(attackChoice);
    if (it != storyCards.end())
        applyModifiers(it->second.modifiers);

    Attack attack = attacks.at(attackChoice); // Again will throw if there is no attack

    // Apply pre spell modifiers
    applyModifiers(attack.preModifiers);

    // TODO: Passive buff

    unsigned char danmakuLevel = TurnAction::isShot(attackChoice)? std::max(1, (int)spiritPower) : awakening-1;
    float danmakuMultiplier = attack.danmakuMultiplier[danmakuLevel];

    const auto &lines = attack.lines;

    // line buff activates when bullet hits (if no bullet hits, no buff)
    // same for barrier breaks
    // Bullets usually follow this pattern:
    // frames: 1 2 3 4 5 6 7 8 9 ... (as many frames as there are bullets in line 1)
    // line 1: x x x x x x x x x (an x is a line bullet hit for the frame)
    // line 2: x x x             (this is if line 2 has 3 bullets)
    // line 3:       x x x x     (this is if line 3 has 4 bullets)
    // line 4: x x x         x x (5 bullets, line 4 wraps around)
    // line 5:       x x         (2 bullets)
    // line 6:           x x     (2 bullets)
    // TODO: Line buff
    // Which lines have a bullet in each frame
    // First line has a bullet in each frame and determines how many frames there will be
    const int framesCount = lines[0].bulletCount;
    int frameId = 0;
    std::vector<std::vector<int>> frames(framesCount, {0});
    for (int i = 1; i < attack.boostLayout[boost]; i++) {
        const auto &line = lines[i];
        for (int j = 0; j < line.bulletCount; j++) {
            frames[frameId % framesCount].push_back(i);
            frameId++;
        }
    }

    // For each bullet compute effects and damage
    // todo bullet effects and hit probability rng to get effective bullet damage
    std::vector<std::vector<float>> simulatedLineDamage(attack.boostLayout[boost]);
    for (const auto &frame : frames) {
        for (const auto &lineId : frame) {
            // Maybe we want for all effects/modifiers to be applied before
            // the bullet damage is calculated (or maybe after)
            // Then we'll make a queue of lineDamage for which
            // the Bernouli test passed and calculated damage for those
            // All the other bullets will be set to 0 anyway
            const auto &line = lines[lineId];

            // Actually compute the bullet damage
            const auto &lineDamage = computeEffectiveLineDamage(enemy, line, danmakuMultiplier);
            // If bullet connects, damage (+ effects if first of the line to connect)
            if (bernouli(lineDamage.accuracy)) {
                applyModifiers(line.modifiers);

                float simulatedCriticalMultiplier =
                        bernouli(lineDamage.critAccuracy) ? lineDamage.critMultiplier : 1;
                simulatedLineDamage[lineId].push_back(lineDamage.bulletDamage * simulatedCriticalMultiplier);
            } else {
                // Bullet misses, 0 damage
                simulatedLineDamage[lineId].push_back(0);
            }
        }
    }

    // Apply post spell modifiers
    applyModifiers(attack.postModifiers);

    // Decrement duration of each active buff
    modifiers.forEachBuff([this](auto &modifierPair) {
        int duration = modifierPair.second.decrementDuration();
        // If duration is zero it means the modifier has expired, so we delete it
        if (duration == 0)
            this->modifiers.erase(modifierPair.first);
    });

    return led;
}


int Character::effectiveStat(Character::Stat stat) const {
    int effectiveStat = stats.at(stat);
    for (const auto& [_, storyCard] : storyCards)
    {
        if (storyCard.firstStat == stat) effectiveStat += storyCard.firstStatValue;
        else if (storyCard.secondStat == stat) effectiveStat += storyCard.secondStatValue;
    }

    return effectiveStat;
}

void Character::applyModifiers(const ModifierContainer &newModifiers) {
    // todo Apply rate/add (update modifier then do stuff with value/duration)
    // same for buffs and effects
    newModifiers.forEachBuff([&](const auto &modifierPair) {
        modifiers[modifierPair.first] += modifierPair.second;
    });
    newModifiers.forEachEffect([&](const auto &modifierPair) {
        switch (modifierPair.first) {
            case ModifierType::Absorb:
                // Recover X% of HP
                addHealth(modifierPair.second.getValue() * stats[Health]);
                break;
            default:
                // Ignore
                std::cerr << "Warning: Effect (" << modifierPair.first << ") is not handled." << std::endl;
        }
    });
}

Character::Character(int level, int awakening, const std::map<Stat, int> &stats,
                     const std::map<Element, float> &temperament,
                     const std::map<TurnAction::Attack, Attack> &attacks,
                     const std::map<TurnAction::Skill, Skill> &skills, const std::set<CharacterTag> &tags) : level(
        level), awakening(awakening), stats(stats), temperament(temperament), attacks(attacks),
                                                                                                             skills(skills),
                                                                                                             tags(tags),
                                                                                                             currentHealth(stats.at(Health)){
    // todo Initialize state (barriers, skill cooldowns, spirit power, ...)
}
