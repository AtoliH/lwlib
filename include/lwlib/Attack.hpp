//
//  Attack.hpp
//  lwcalculator
//
//  Created by Fire937 on 6/16/22.
//

#ifndef Attack_hpp
#define Attack_hpp

#include <array>
#include <vector>
#include <type_traits>
#include <map>
#include <variant>
#include <set>

#include "lwlib/internal/macro.h"
#include "lwlib/Modifier.hpp"
#include "lwlib/TurnAction.hpp"

enum class CharacterTag {
    Gensokyo = RACE_GENSOKYO,
    Human = RACE_HUMAN,
    Magician = RACE_MAGICIAN,
    Youkai = RACE_YOUKAI,
    IncidentResolver = RACE_INCIDENT_RESOLVER,
    Tengu = RACE_TENGU
};

enum class Element {
    None = ELEMENT_NONE,
    Sun = ELEMENT_SUN,
    Moon = ELEMENT_MOON,
    Fire = ELEMENT_FIRE,
    Water = ELEMENT_WATER,
    Wood = ELEMENT_WOOD,
    Metal = ELEMENT_METAL,
    Earth = ELEMENT_EARTH,
    Star = ELEMENT_STAR
};

enum class BulletType {
    Ofuda = BULLET_TYPE_OFUDA,
    Normal = BULLET_TYPE_NORMAL,
    Energy = BULLET_TYPE_ENERGY,
    Body = BULLET_TYPE_BODY,
    Liquid = BULLET_TYPE_LIQUID,
    Light = BULLET_TYPE_LIGHT,
    Laser = BULLET_TYPE_LASER,
    Sharp = BULLET_TYPE_SHARP,
    Heavy = BULLET_TYPE_HEAVY,
    Missile = BULLET_TYPE_MISSILE,
    Slash = BULLET_TYPE_SLASH
};

struct Attack
{
    struct Line
    {
    public:
        enum YinYang {
            Yin = BULLET_ATTRIBUTE_YIN,
            Yang = BULLET_ATTRIBUTE_YANG
        };

        enum Tag {
            Hard,
            Slicing,

            // Special
            Piercing,
            Specular,
            Elastic,
            Precise,
            Explosive,

            // Status Break
            Melting = BARRIER_FREEZE,
            Toxic = BARRIER_POISON,
            Discharged = BARRIER_PARALYZE,
            Incineration = BARRIER_BURN,
            Flash = BARRIER_BLIND,
        };

        /**
         * Used to attach more detail to a tag such as its slicing power,
         * its poisoning rate, or the numbers of turns the effect will last
         * Not all tags have these details and some tags can have only part
         * of these details
         */
        struct TagInfo {
            float value;
            float rate;
            float duration;
        };
        
        float power;
        float accuracy;
        float critAccuracy;
        int bulletCount;
        YinYang bulletYinYang;
        BulletType bulletType;
        Element bulletElement;

        /**
         * Bullet tags such as hard, piercing, elastic, etc.
         * Doesn't include killers and modifiers.
         */
        std::map<Tag, TagInfo> tags;
        /**
         * Killers such as Gensokyo-Killer, Human-Killer, etc.
         */
        std::set<CharacterTag> killers;
        /**
         * Modifiers such as Yang ATK UP/DOWN, Element X UP, etc.
         * Also contains effects such as Recover Health, Barrier Generation, ...
         */
        ModifierContainer modifiers;

        Line(float power, float accuracy, float critAccuracy, int bulletCount, YinYang bulletYinYang,
             BulletType bulletType, Element bulletElement, const std::map<Tag, TagInfo> &tags,
             const std::set<CharacterTag> &killers, const ModifierContainer &modifiers);

        /**
         * Get tag info if tag exists, throws otherwise
         * @return
         */
        TagInfo getTag(Tag tag) const {
            return tags.at(tag);
        }

        bool hasTag(Tag tag) const {
            return tags.find(tag) != tags.end();
        }

        bool hasKiller(std::set<CharacterTag> tags) {
            return setIntersect(killers.begin(), killers.end(), tags.begin(), tags.end());
        }

        bool isYang() const {
            return bulletYinYang == Yang;
        }
        
        bool isYin() const {
            return bulletYinYang == Yin;
        }

    private:
        template <class I1, class I2>
        static bool setIntersect(I1 first1, I1 last1, I2 first2, I2 last2) {
            while (first1 != last1 && first2 != last2) {
                if (*first1 < *first2)
                    ++first1;
                else if (*first2 < *first1)
                    ++first2;
                else
                    return true;
            }
            return false;
        }
    };
    
    std::array<Line, 6> lines;
    /**
     * Danmaku Level Modifier is a multiplier to a hit which is based off the level of the shot/spellcard.
     * Spell Card level is raised by Awakening a unit, Basic Shot level rises with total Spirit Power
     * Ranges from lv.0 to lv.5, multiplier is the same for 0 and 1?? will have to look into this
     * Level 5 usually around 133% (to verify...)
     * From observation (very very roughly):
     * - spell: 100 105 115 120 125
     * - shot:  105 110 115 120 125
     */
    std::array<unsigned short, 5> danmakuMultiplier;
    /**
     * The boost pattern of the spell (e.g. 1-1-3, 2-2-1, ...)
     * The amount of boost needed to unleash the number of lines mapped
     * Line 0 is always used so it's ignored, count starts at line 1
     * e.g. for 3-1-1
     * Boost Once -> 3 mapped -> unleash first 3 lines (ignoring line 0)
     * Boost Twice -> 3 + 1 = 4 mapped -> unleash first 4 lines (ignoring line 0)
     * Boost Thrice -> 3 + 1 + 1 = 5 mapped -> unleash first 5 lines (i.e. all lines)
     * No Boost is always 1 (we only launch the first line at 0 boost power)
     * Boost Thrice is always 6 (we always launch all 6 lines at max boost power)
     */
    std::map<TurnAction::Boost, int> boostLayout;
    
    ModifierContainer preModifiers;
    ModifierContainer postModifiers;
    
    Attack(const std::array<Line, 6> &lines, const std::array<unsigned short, 5> &danmakuMultiplier, const std::map<TurnAction::Boost, int> &boostLayout, const ModifierContainer &preModifiers = {}, const ModifierContainer &postModifiers = {}):
    lines(lines), danmakuMultiplier(danmakuMultiplier), boostLayout(boostLayout), preModifiers(preModifiers), postModifiers(postModifiers)
    {
        
    }

};

#endif /* Attack_hpp */
