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

#include "lwlib/internal/types.h"
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
        
        enum Special {
            Piercing,
            Specular,
            Elastic,
            Precise,
            Explosive
        };
        
        enum StatusBreak {
            Melting = BARRIER_FREEZE,
            Toxic = BARRIER_POISON,
            Discharged = BARRIER_PARALYZE,
            Incineration = BARRIER_BURN,
            Flash = BARRIER_BLIND
        };
        
        float power;
        float accuracy;
        float critAccuracy;
        YinYang bulletYinYang;
        BulletType bulletType;
        Element bulletElement;
        unsigned int bulletCount;
        
        float hard;
        float slicing;
        
        bool isPiercing = false;
        bool isSpecular = false;
        bool isElastic = false;
        bool isPrecise = false;
        bool isExplosive = false;
        
        std::set<StatusBreak> statusBreaks;
        std::set<CharacterTag> killers;
        
        template <typename... Ts, std::enable_if_t<(sizeof...(Ts) > 0)>* = nullptr>
        Line(float power, float accuracy, float critAccuracy, YinYang bulletYinYang, BulletType bulletType, Element bulletElement, unsigned int bulletCount, float hard, float slicing, Ts... tags):
        Line(power, accuracy, critAccuracy, bulletYinYang, bulletType, bulletElement, bulletCount, hard, slicing)
        {
            
            ([&]{
                if constexpr(std::is_same_v<decltype(tags), StatusBreak>)
                    statusBreaks.insert(tags);
                else if constexpr(std::is_same_v<decltype(tags), CharacterTag>)
                    killers.insert(tags);
                else if constexpr(std::is_same_v<decltype(tags), Special>)
                {
                    switch (tags) {
                        case Piercing:
                            isPiercing = true;
                            break;
                        case Specular:
                            isSpecular = true;
                            break;
                        case Elastic:
                            isElastic = true;
                            break;
                        case Precise:
                            isPrecise = true;
                        case Explosive:
                            isExplosive = true;
                            break;
                        default:
                            break;
                    }
                }
            } (), ...);
        }
        
        Line(float power, float accuracy, float critAccuracy, YinYang bulletYinYang, BulletType bulletType, Element bulletElement, unsigned int bulletCount, float hard = 0.0, float slicing = 0.0):
        power(power), accuracy(accuracy), critAccuracy(critAccuracy), bulletYinYang(bulletYinYang), bulletType(bulletType), bulletElement(bulletElement), bulletCount(bulletCount), hard(hard), slicing(slicing)
        {
        }
        
        Line(attack_line c_line):
        power(c_line.pow),
        accuracy(c_line.hit),
        critAccuracy(c_line.crit),
        bulletYinYang(static_cast<YinYang>(c_line.attribute)),
        bulletType(static_cast<BulletType>(c_line.bullet)),
        bulletElement(static_cast<Element>(c_line.element)),
        bulletCount(c_line.bullet_count),
        hard(c_line.hard),
        slicing(c_line.slicing),
        isPiercing(c_line.piercing),
        isSpecular(c_line.specular),
        isElastic(c_line.elastic),
        isPrecise(c_line.precise),
        isExplosive(c_line.explosive)
        {
            for (char i = 0; i < sizeof(c_line.status_breaks)*8; ++i)
            {
                decltype(c_line.status_breaks) flag = 1 << i;
                if (c_line.status_breaks & flag)
                    statusBreaks.insert(static_cast<StatusBreak>(flag));
            }
            
            for (char i = 0; i < sizeof(c_line.killers); ++i)
            {
                decltype(c_line.killers) flag = 1 << i;
                if (c_line.killers & flag)
                    killers.insert(static_cast<CharacterTag>(flag));
            }
        }
        
        bool isYang() const {
            return bulletYinYang == Yang;
        }
        
        bool isYin() const {
            return bulletYinYang == Yin;
        }
    };
    
    std::array<Line, 6> lines;
    std::array<unsigned short, 5> danmakuMultiplier;
    std::map<TurnAction::Boost, unsigned char> boostLayout;
    
    ModifierType::Container preModifiers;
    ModifierType::Container postModifiers;
    
    Attack(std::array<Line, 6> lines, std::array<unsigned short, 5> danmakuMultiplier, std::map<TurnAction::Boost, unsigned char> boostLayout, ModifierType::Container preModifiers = {}, ModifierType::Container postModifiers = {}):
    lines(lines), danmakuMultiplier(danmakuMultiplier), boostLayout(boostLayout), preModifiers(preModifiers), postModifiers(postModifiers)
    {
        
    }
    
    Attack(attack c_attack):
    lines({ c_attack.line[0], c_attack.line[1], c_attack.line[2], c_attack.line[3], c_attack.line[4], c_attack.line[5] }),
    danmakuMultiplier({
        static_cast<unsigned short>(c_attack.danmaku_modifier[0]),
        static_cast<unsigned short>(c_attack.danmaku_modifier[1]),
        static_cast<unsigned short>(c_attack.danmaku_modifier[2]),
        static_cast<unsigned short>(c_attack.danmaku_modifier[3]),
        static_cast<unsigned short>(c_attack.danmaku_modifier[4])
    }),
    boostLayout({
        std::pair(TurnAction::noBoost, static_cast<unsigned char>(c_attack.boost_layout[0])),
        std::pair(TurnAction::boostOnce, static_cast<unsigned char>(c_attack.boost_layout[1])),
        std::pair(TurnAction::boostTwice, static_cast<unsigned char>(c_attack.boost_layout[2])),
        std::pair(TurnAction::boostThrice, static_cast<unsigned char>(c_attack.boost_layout[3]))
    })
    {
        char c_array_size = sizeof(c_attack.pre_modifier_type) / sizeof(c_attack.pre_modifier_type[0]);
        for (char i = 0; i < c_array_size; ++i)
        {
            unsigned int type = c_attack.pre_modifier_type[i];
            int value = c_attack.pre_modifier_value[i];
            unsigned int duration = c_attack.pre_modifier_duration[i];
            if (value != 0 && duration != 0)
            {
                preModifiers.insert(ModifierType::variantFromC(type, value, duration));
            }
            
            type = c_attack.post_modifier_type[i];
            value = c_attack.post_modifier_value[i];
            duration = c_attack.post_modifier_duration[i];
            if (value != 0 && duration != 0)
                postModifiers.insert(ModifierType::variantFromC(type, value, duration));
        }
    }
};

#endif /* Attack_hpp */
