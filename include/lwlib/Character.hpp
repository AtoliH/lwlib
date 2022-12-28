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
        unsigned int firstStatValue;
        Stat secondStat;
        unsigned int secondStatValue;
        
        ModifierType::Container modifiers;
        
        template <typename... T>
        StoryCard(Stat firstStat, unsigned int firstStatValue, Stat secondStat, unsigned int secondStatValue, T... modifier):
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
            Burn = Attack::Line::StatusBreak::Incineration,
            Blind = Attack::Line::StatusBreak::Flash,
            Paralyze = Attack::Line::StatusBreak::Discharged,
            Freeze = Attack::Line::StatusBreak::Melting,
            Poison = Attack::Line::StatusBreak::Toxic
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
        ModifierType::Container modifiers;
        
        Skill(unsigned char cooldown, ModifierType::Container modifiers): cooldown(cooldown), modifiers(modifiers)
        {
            
        }
        
        Skill(skill c_skill)
        {
            for (char i = 0; i < sizeof(c_skill.modifier_type) / sizeof(c_skill.modifier_type[0]); ++i)
            {
                unsigned int type = c_skill.modifier_type[i];
                int value = c_skill.modifier_value[i];
                unsigned int duration = c_skill.modifier_duration[i];
                modifiers.insert(ModifierType::variantFromC(type, value, duration));
            }
        }
    };
    
    struct LineEffectiveDamage {
        unsigned int bulletDamage;
        float accuracy;
        float critAccuracy;
        float critMultiplier;
        unsigned int bulletCount;
        
        LineEffectiveDamage(unsigned int bulletDamage, float accuracy, float critAccuracy, float critMultiplier, unsigned int bulletCount):
        bulletDamage(bulletDamage), accuracy(accuracy), critAccuracy(critAccuracy), critMultiplier(critMultiplier), bulletCount(bulletCount)
        {
            
        }
    };
    
private:
    std::map<TurnAction::Attack, Attack> attacks;
    
    unsigned int level;
    unsigned int awakening;
    std::set<Element> weakness;
    std::set<Element> resistance;
    std::map<Stat, unsigned int> stats;
    
    std::set<CharacterTag> tags;
    
    std::vector<Barrier> barriers;
    ModifierType::Container modifiers;
    std::map<TurnAction::Skill, Skill> skills;
    std::map<TurnAction::Skill, unsigned char> skillTimer; // Keeps track of skillcooldown
    
    std::map<TurnAction::Attack, StoryCard> storyCards;
    
    float spiritPower;
    
public:
    Character(lw_character c_character);
    
    void useSkill(TurnAction::Skill skillAction);
    
    std::vector<LineEffectiveDamage> attack(const Character enemy, TurnAction::Attack attackChoice = TurnAction::spreadShot, TurnAction::Boost boost = TurnAction::noBoost, TurnAction::Graze graze = TurnAction::noGraze);
    
    unsigned int effectiveStat(Stat stat) const;
    
    template <typename T>
    ModifierType::Variant modifierVariant(T type) const
    {
        auto it = modifiers.find(type);
        if (it == modifiers.end())
            return Modifier(type);
        
        return *it;
    }
    
    template <typename T>
    int modifierValue(T type) const
    {
        return ModifierType::value(modifierVariant(type));
    }
    
    template <typename T, typename U>
    static int substractModifierValue(const Character &x, const Character &y, T a, U b)
    {
        return std::visit([](auto &&a, auto &&b) { return ModifierType::value(a - b); }, x.modifierVariant(a), y.modifierVariant(b));
    }
    
    void equip(TurnAction::Attack attack, StoryCard storyCard)
    {
        storyCards.insert(std::pair(attack, storyCard));
    }
    
    unsigned char getLevel() const { return level; }
        
    bool hasWeakness(Element element) const { return weakness.find(element) != weakness.end(); }
    bool hasResistance(Element element) const { return resistance.find(element) != resistance.end(); }
    bool hasTag(CharacterTag tag) const { return tags.find(tag) != tags.end(); }
    
private:
    void applyModifiers(const ModifierType::Container &newModifiers);
};

#endif /* Character_hpp */
