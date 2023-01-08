//
//  Character.hpp
//  lwcalculator
//
//  Created by Fire937 on 6/16/22.
//

#ifndef Character_hpp
#define Character_hpp

#include <map>
#include <set>
#include <cmath>
#include <random>

#include "lwlib/TurnAction.hpp"
#include "lwlib/Attack.hpp"

class Character
{
public:
    enum Stat {
        Health = STAT_HEALTH,
        Agility = STAT_AGILITY,
        YangAttack = STAT_YANG_ATK,
        YangDefense = STAT_YANG_DEF,
        YinAttack = STAT_YIN_ATK,
        YinDefense = STAT_YIN_DEF
    };
    
    struct StoryCard {
        Stat firstStat;
        int firstStatValue;
        Stat secondStat;
        int secondStatValue;
        
        ModifierContainer modifiers;
        
        template <typename... T>
        StoryCard(Stat firstStat, int firstStatValue, Stat secondStat, int secondStatValue, std::pair<T, Modifier<T>> ...modifier):
        firstStat(firstStat), firstStatValue(firstStatValue), secondStat(secondStat), secondStatValue(secondStatValue)
        {
            ([&]{
                modifiers.insert(modifier);
            }(), ...);
        }
    };
    
    class Barrier {
    public:
        enum Status {
            Broken = BARRIER_BROKEN,
            Healthy = BARRIER_HEALTHY,
            Burn = Attack::Line::Incineration,
            Blind = Attack::Line::Flash,
            Paralyze = Attack::Line::Discharged,
            Freeze = Attack::Line::Melting,
            Poison = Attack::Line::Toxic
        };
        
    private:
        Status status;
        unsigned char duration;
    
    public:
        Barrier(Status status, unsigned char duration): status(status), duration(duration)
        {
            
        }
    };
    
    struct Skill {
        unsigned char cooldown;
        ModifierContainer modifiers;
        
        Skill(unsigned char cooldown, const ModifierContainer &modifiers): cooldown(cooldown), modifiers(modifiers)
        {
            
        }
    };
    
    struct LineEffectiveDamage {
        int bulletDamage;
        float accuracy;
        float critAccuracy;
        float critMultiplier;
        int bulletCount;
        
        LineEffectiveDamage(int bulletDamage, float accuracy, float critAccuracy, float critMultiplier, int bulletCount):
        bulletDamage(bulletDamage), accuracy(accuracy), critAccuracy(critAccuracy), critMultiplier(critMultiplier), bulletCount(bulletCount)
        {
            
        }
    };
    
private:
    // Immutable base specs
    int level;
    int awakening;
    // todo Some of these types are not right, for example this should be unordered_map
    std::map<Stat, int> stats;
    /**
     * Weakness or resistance to an element
     * The incoming damage is multiplied by the value of the temperament
     * for the element of the incoming bullet
     */
    std::map<Element, float> temperament;

    std::map<TurnAction::Attack, Attack> attacks;
    std::map<TurnAction::Skill, Skill> skills;

    std::set<CharacterTag> tags;

    // Equipment
    std::map<TurnAction::Attack, StoryCard> storyCards;

    // State variables
    float spiritPower;
    std::vector<Barrier> barriers;

    ModifierContainer modifiers;
    std::map<TurnAction::Skill, unsigned char> skillTimer; // Keeps track of skillcooldown

    int currentHealth;

public:
    Character(int level, int awakening, const std::map<Stat, int> &stats,
              const std::map<Element, float> &temperament,
              const std::map<TurnAction::Attack, Attack> &attacks, const std::map<TurnAction::Skill, Skill> &skills,
              const std::set<CharacterTag> &tags);

    void useSkill(TurnAction::Skill skillAction);

    /**
     * Simulate an attack using pseudo RNG
     * @param enemy
     * @param attackChoice
     * @param boost
     * @param graze
     * @return
     */
    std::vector<LineEffectiveDamage> attack(const Character enemy, TurnAction::Attack attackChoice = TurnAction::spreadShot, TurnAction::Boost boost = TurnAction::noBoost, TurnAction::Graze graze = TurnAction::noGraze);

    /**
     * Stat calculated from character's base stats and story cards stats, no modifier applied
     * @param stat
     * @return
     */
    int effectiveStat(Stat stat) const;

    unsigned char getLevel() const { return level; }

    float getMultiplier(Element element) const { return temperament.at(element); }
    bool hasTag(CharacterTag tag) const { return tags.find(tag) != tags.end(); }

    template <typename T>
    const Modifier<T>& getModifier(T type) const { return modifiers[type]; }

    // Public methods to update the character's specs manually
    /**
     * Equip the character with the specified story card
     * @param attack The slot on which the story card should be attached
     * @param storyCard The story card to equip
     */
    void equip(TurnAction::Attack attack, StoryCard storyCard)
    {
        storyCards.insert(std::make_pair(attack, storyCard));
    }

    void setHealth(int health) {
        currentHealth = std::clamp(health, 0, stats[Health]);
    }

    void addHealth(int health) {
        setHealth(currentHealth + health);
    }
private:
    /**
     * Apply the given modifiers to the character using rng to
     * eventually determine if the modifier will be applied or not.
     * Applies buffs to character and activates eventual effects (Absorb etc)
     * todo Pass an enemy (enemies) character to this function for effects such as Yang ATK Down to work
     * @param newModifiers
     */
    void applyModifiers(const ModifierContainer &newModifiers);

    float computeTotalAttack(const Attack::Line &line) const {
        // todo Check if modifier exists before accessing it (otherwise will throw if doesn't exist)
        float baseAttack = line.isYang()? effectiveStat(YangAttack) : effectiveStat(YinAttack);
        int attackModifier = line.isYang()? modifiers[ModifierType::YangAttack].getValue() : modifiers[ModifierType::YinAttack].getValue();
        float attackMultiplier = std::pow(1 + std::abs(attackModifier) * 0.3f, (0 < attackModifier) - (attackModifier < 0));

        // Tags damage (hard, slicing etc)
        float baseHard = line.isYang()? effectiveStat(YangDefense) : effectiveStat(YinDefense);
        float hard = line.hasTag(Attack::Line::Hard) ? line.getTag(Attack::Line::Hard).value : 0;
        float slicing = line.hasTag(Attack::Line::Slicing) ? line.getTag(Attack::Line::Slicing).value : 0;
        float specialAttack = hard * baseHard + slicing * effectiveStat(Agility);
        float totalAttack = (baseAttack + specialAttack) * attackMultiplier;

        return totalAttack;
    }

    float computeShotBuffs(const Attack::Line &line) const {
        // Bullet buff (element and bullet type)
        int elementBuff = modifiers[ModifierType::fromElement(line.bulletElement)].getValue();
        int bulletBuff = modifiers[ModifierType::fromBullet(line.bulletType)].getValue();
        float shotBuffs = 1 + (elementBuff + bulletBuff)/100.0f;

        return shotBuffs;
    }

    float computeTotalDefense(const Character &enemy, const Attack::Line &line) const {
        // Enemy defense
        float baseDefense = line.isYang() ? enemy.effectiveStat(YangDefense) : enemy.effectiveStat(YinDefense);
        int defenseModifier = line.isYang() ? enemy.getModifier(ModifierType::YangDefense).getValue() : enemy.getModifier(ModifierType::YinDefense).getValue();
        float defenseMultiplier = line.hasTag(Attack::Line::Piercing) ? 1.0 : std::pow(1 + std::abs(defenseModifier) * 0.3f, (0 < defenseModifier) - (defenseModifier < 0));
        float totalDefense = baseDefense * defenseMultiplier;

        return totalDefense;
    }

    float computeAccuracy(const Character &enemy, const Attack::Line &line) const {
        // Accuracy
        int accuracyModifier = (modifiers[ModifierType::Accuracy] - enemy.getModifier(ModifierType::Evasion)).getValue();
        float accuracyMultiplier = std::pow(1 + 0.2f * std::abs(accuracyModifier), (0 < accuracyModifier) - (accuracyModifier < 0));
        float accuracy = std::min(1.0f, line.accuracy * accuracyMultiplier);

        return accuracy;
    }

    float computeCriticalMultiplier(const Character &enemy) const {
        // Critical (multiplier)
        int criticalAttackModifier = modifiers[ModifierType::CritAttack].getValue();
        int criticalDefenseModifier = enemy.getModifier(ModifierType::CritDefense).getValue();
        float criticalAttackBase = std::pow(1 + std::abs(criticalAttackModifier) * 0.3f, (0 < criticalAttackModifier) - (criticalAttackModifier < 0));
        float criticalDefenseBase = std::pow(1 + std::abs(criticalDefenseModifier) * 0.3f, (0 < criticalDefenseModifier) - (criticalDefenseModifier < 0));
        float criticalMultiplier = 1 + criticalAttackBase / criticalDefenseBase;

        return criticalMultiplier;
    }

    float computeCriticalAccuracy(const Character &enemy, const Attack::Line &line) const {
        // Critical (accuracy)
        int criticalAccuracyModifier = (modifiers[ModifierType::CritAccuracy] - enemy.getModifier(ModifierType::CritEvasion)).getValue();
        float criticalAccuracyMultiplier = std::pow(1 + 0.2f * std::abs(criticalAccuracyModifier), (0 < criticalAccuracyModifier) - (criticalAccuracyModifier < 0));
        bool doesKillerConnect = std::any_of(line.killers.begin(), line.killers.end(), [&enemy](const auto &tag){
            return enemy.hasTag(tag);
        });
        float criticalAccuracy = std::min(1.0f, (doesKillerConnect? 1.0f : line.critAccuracy) * criticalAccuracyMultiplier);

        return criticalAccuracy;
    }

    LineEffectiveDamage
    computeEffectiveLineDamage(const Character &enemy, const Attack::Line &line, float danmakuMultiplier) const {
        //---------- Bullet Damage ----------//
        float totalAttack = computeTotalAttack(line);

        float shotPower = line.power;
        float shotBuffs = computeShotBuffs(line);

        // Enemy specs (level, resistance solo/all, temperament and trait)
        float defenseLevel = enemy.getLevel();
        float targettingResistance = 1;
        float traitResistance = 1;
        float temperamentMultiplier = enemy.getMultiplier(line.bulletElement);
        float totalDefense = computeTotalDefense(enemy, line);

        float accuracy = computeAccuracy(enemy, line);
        float critMultiplier = computeCriticalMultiplier(enemy);
        float critAccuracy = computeCriticalAccuracy(enemy, line);

        // TODO: Barrier breaks
        float brokenMultiplier = 1.0f;

        // Aggregate everything
        // todo Some base damage is independent of the line/bullet
        float hitBulletDamageBase = (totalAttack * level * shotPower * shotBuffs * danmakuMultiplier * temperamentMultiplier) / (totalDefense * traitResistance * targettingResistance * defenseLevel);

        return LineEffectiveDamage(
                hitBulletDamageBase,
                accuracy,
                critAccuracy,
                critMultiplier,
                line.bulletCount);
    }

    bool bernouli(float p) {
        static std::default_random_engine rng;
        static std::uniform_real_distribution<double> dist(0.0, 1.0);
        return p > dist(rng);
    }

    void attackRecap() {
//        if (i == 0)
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
    }
};

#endif /* Character_hpp */
