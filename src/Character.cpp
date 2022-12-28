//
//  Character.cpp
//  lwcalculator
//
//  Created by Fire937 on 6/16/22.
//

#include "lwlib/Character.hpp"

Character::Character(lw_character c_character) :
        attacks({
                        {TurnAction::firstSpellCard, c_character.spell[TurnAction::firstSpellCard]},
                        {TurnAction::secondSpellCard, c_character.spell[TurnAction::secondSpellCard]},
                        {TurnAction::thirdSpellCard, c_character.spell[TurnAction::thirdSpellCard]},
                        {TurnAction::fourthSpellCard, c_character.spell[TurnAction::fourthSpellCard]},
                        {TurnAction::lastWord, c_character.spell[TurnAction::lastWord]},
                        {TurnAction::spreadShot, c_character.spell[TurnAction::spreadShot]},
                        {TurnAction::focusShot, c_character.spell[TurnAction::focusShot]}
                }),
        level(c_character.level),
        awakening(5),
        spiritPower(1),
        stats({
                      {Stat::Health, c_character.stats[Stat::Health]},
                      {Stat::Agility, c_character.stats[Stat::Agility]},
                      {Stat::YangAttack, c_character.stats[Stat::YangAttack]},
                      {Stat::YangDefense, c_character.stats[Stat::YangDefense]},
                      {Stat::YinAttack, c_character.stats[Stat::YinAttack]},
                      {Stat::YinDefense, c_character.stats[Stat::YinDefense]}
              }),
        skills({
                       {TurnAction::firstSkill, c_character.skills[0]},
                       {TurnAction::secondSkill, c_character.skills[1]},
                       {TurnAction::thirdSkill, c_character.skills[2]}
               })
{
    for (char i = 0; i < sizeof(c_character.weakness)*8; ++i)
    {
        decltype(c_character.weakness) flag = 1 << i;
        if (c_character.weakness & flag)
            weakness.insert(static_cast<Element>(flag));
    }

    for (char i = 0; i < sizeof(c_character.resistance)*8; ++i)
    {
        decltype(c_character.resistance) flag = 1 << i;
        if (c_character.resistance & flag)
            resistance.insert(static_cast<Element>(flag));
    }

    for (char i = 0; i < sizeof(c_character.races)*8; ++i)
    {
        decltype(c_character.races) flag = 1 << i;
        if (c_character.races & flag)
            tags.insert(static_cast<CharacterTag>(flag));
    }

    decltype(c_character.barriers) bitmask = c_character.barriers;
    for (char i = 0; i < sizeof(c_character.barrier_status_duration); ++i)
    {
        if (bitmask)
            barriers.push_back(Barrier(static_cast<Barrier::Status>(bitmask & FIRST_BARRIER_MASK), c_character.barrier_status_duration[i]));
        bitmask <<= BARRIER_NUMBER_OF_STATES;
    }

    for (char i = 0; i < NUMBER_OF_UNIQUE_MODIFIERS; ++i)
    {
        int value = c_character.modifier_value[i];
        unsigned int duration = c_character.modifier_duration[i];
        modifiers.insert(ModifierType::variantFromC(i, value, duration));
    }
}

void Character::useSkill(TurnAction::Skill skillAction) {
    auto skill = skills.at(skillAction); // Can throw if the skill has not been set, responsability of the class to always have all three skills set, we'll see how we handle skills that are not learned yet
    applyModifiers(skill.modifiers);
}

std::vector<Character::LineEffectiveDamage>
Character::attack(const Character enemy, TurnAction::Attack attackChoice, TurnAction::Boost boost,
                  TurnAction::Graze graze) {
    auto it = storyCards.find(attackChoice);
    if (it != storyCards.end())
        applyModifiers(it->second.modifiers);

    Attack attack = attacks.at(attackChoice); // Again will throw if there is no attack
    applyModifiers(attack.preModifiers);

    // TODO: Passive buff

    unsigned char danmakuLevel = TurnAction::isShot(attackChoice)? std::max(1, (int)spiritPower) : awakening-1;
    float danmakuMultiplier = attack.danmakuMultiplier[danmakuLevel];

    std::vector<LineEffectiveDamage> led;
    auto lines = attack.lines;
    for (unsigned int i = 0; i < attack.boostLayout[boost]; ++i)
    {
        auto line = lines[i];

        // line buff activates when bullet hits (if no bullet hits, no buff)
        // Bullets usually follow this pattern:
        // frames: 1 2 3 4 5 6 7 8 9 ... (as many frames as there are bullets in line 1)
        // line 1: x x x x x x x x x (an x is a line bullet hit for the frame)
        // line 2: x x x             (this is if line 2 has 3 bullets)
        // line 3:       x x x x     (this is if line 3 has 4 bullets)
        // line 4: x x x         x x (5 bullets, line 4 wraps around)
        // line 5:       x x         (2 bullets)
        // line 6:           x x     (2 bullets)
        // TODO: Line buff

        float baseAttack = line.isYang()? effectiveStat(YangAttack) : effectiveStat(YinAttack);
        int attackModifier = line.isYang()? modifierValue(ModifierType::YangAttack) : modifierValue(ModifierType::YinAttack);
        float attackMultiplier = std::pow(1 + std::abs(attackModifier) * 0.3f, (0 < attackModifier) - (attackModifier < 0));

        float baseHard = line.isYang()? effectiveStat(YangDefense) : effectiveStat(YinDefense);
        float specialAttack = line.hard * baseHard + line.slicing * effectiveStat(Agility);
        float totalAttack = (baseAttack + specialAttack) * attackMultiplier;

        int elementBuff = modifierValue(ModifierType::fromElement(line.bulletElement));
        int bulletBuff = modifierValue(ModifierType::fromBullet(line.bulletType));
        float shotBuffs = 1 + (elementBuff + bulletBuff)/100.0f;
        float shotPower = line.power;

        float defenseLevel = enemy.getLevel();
        float targettingResistance = 1;
        float traitResistance = 1;

        float baseDefense = line.isYang() ? enemy.effectiveStat(YangDefense) : enemy.effectiveStat(YinDefense);
        int defenseModifier = line.isYang() ? enemy.modifierValue(ModifierType::YangDefense) : enemy.modifierValue(ModifierType::YinDefense);
        float defenseMultiplier = line.isPiercing? 1.0 : std::pow(1 + std::abs(defenseModifier) * 0.3f, (0 < defenseModifier) - (defenseModifier < 0));
        float totalDefense = baseDefense * defenseMultiplier;

        int accuracyModifier = substractModifierValue(*this, enemy, ModifierType::Accuracy, ModifierType::Evasion);
        float accuracyMultiplier = std::pow(1 + 0.2f * std::abs(accuracyModifier), (0 < accuracyModifier) - (accuracyModifier < 0));
        float accuracy = std::min(1.0f, line.accuracy * accuracyMultiplier);

        float weaknessMultiplier = enemy.hasWeakness(line.bulletElement) ? 2 : 1;
        float resistanceMultiplier = enemy.hasResistance(line.bulletElement) ? 0.5f : 1;

        int critAttackModifier = modifierValue(ModifierType::CritAttack);
        int critDefenseModifier = enemy.modifierValue(ModifierType::CritDefense);
        float critAttackBase = std::pow(1 + std::abs(critAttackModifier) * 0.3f, (0 < critAttackModifier) - (critAttackModifier < 0));
        float critDefenseBase = std::pow(1 + std::abs(critDefenseModifier) * 0.3f, (0 < critDefenseModifier) - (critDefenseModifier < 0));
        float critMultiplier = 1 + critAttackBase / critDefenseBase;

        int critAccuracyModifier = substractModifierValue(*this, enemy, ModifierType::CritAccuracy, ModifierType::CritEvasion);
        float critAccuracyMultiplier = std::pow(1 + 0.2f * std::abs(critAccuracyModifier), (0 < critAccuracyModifier) - (critAccuracyModifier < 0));
        bool doesKillerConnect = std::any_of(line.killers.begin(), line.killers.end(), [&enemy](const auto &tag){
            return enemy.hasTag(tag);
        });
        float critAccuracy = std::min(1.0f, (doesKillerConnect? 1.0f : line.critAccuracy) * critAccuracyMultiplier);

        // TODO: Barrier breaks
        float brokenMultiplier = 1.0f;

        float hitBulletDamageBase = (totalAttack * level * shotPower * shotBuffs * danmakuMultiplier * weaknessMultiplier * resistanceMultiplier) / (totalDefense * traitResistance * targettingResistance * defenseLevel);

//            if (i == 0)
//            {
//                std::cout
//                    << "total attack: " << totalAttack
//                    << " level: " << level
//                    << " shot pow: " << shotPower
//                    << " shot buff: " << shotBuffs
//                    << " danmaku: " << danmakuMultiplier
//                    << " weakness: " << weaknessMultiplier
//                    << " resistance: " << resistanceMultiplier
//                    << " total defense: " << totalDefense
//                    << " trait: " << traitResistance
//                    << " targetting: " << targettingResistance
//                    << " def level: " << defenseLevel
//                    << " crit acc: " << critAccuracy
//                    << " crit mul: " << critMultiplier
//                    << " bullet count: " << line.bulletCount
//                    << " accuracy: " << accuracy
//                << std::endl;
//            }

        led.push_back(LineEffectiveDamage(hitBulletDamageBase, accuracy, critAccuracy, critMultiplier, line.bulletCount));
    }

    applyModifiers(attack.postModifiers);

    for (auto modifier : ModifierType::Container(modifiers))
    {
        modifiers.erase(modifier);
        std::visit([](auto &modifier) { modifier--; }, modifier);
        if (ModifierType::duration(modifier) > 0)
            modifiers.insert(modifier);
    }

    return led;
}

unsigned int Character::effectiveStat(Character::Stat stat) const {
    unsigned int effectiveStat = stats.at(stat);
    for (const auto& [_, storyCard] : storyCards)
    {
        if (storyCard.firstStat == stat) effectiveStat += storyCard.firstStatValue;
        else if (storyCard.secondStat == stat) effectiveStat += storyCard.secondStatValue;
    }

    return effectiveStat;
}

void Character::applyModifiers(const ModifierType::Container &newModifiers) {
    for(auto &modifier : newModifiers)
    {
        auto it = modifiers.find(modifier);
        if (it != modifiers.end())
        {
            auto newModifier = std::visit([](auto &&a, auto &&b) -> ModifierType::Variant { return a + b; }, *it, modifier);
            modifiers.erase(modifier);
            modifiers.insert(newModifier);
        }
        else
            modifiers.insert(modifier);
    }
}
