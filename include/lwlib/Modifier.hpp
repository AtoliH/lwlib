//
//  Modifier.hpp
//  lwcalculator
//
//  Created by Fire937 on 6/18/22.
//

#ifndef Modifier_hpp
#define Modifier_hpp

#include <set>
#include <variant>
#include <algorithm>

#include "lwlib/internal/macro.h"

template <typename T>
class Modifier;

enum class Element;
enum class BulletType;

struct ModifierType {
    enum Element {
        ElementNone = MOD_ELEMENT_NONE,
        ElementSun = MOD_ELEMENT_SUN,
        ElementMoon = MOD_ELEMENT_MOON,
        ElementFire = MOD_ELEMENT_FIRE,
        ElementWater = MOD_ELEMENT_WATER,
        ElementWood = MOD_ELEMENT_WOOD,
        ElementMetal = MOD_ELEMENT_METAL,
        ElementEarth = MOD_ELEMENT_EARTH,
        ElementStar = MOD_ELEMENT_STAR
    };
    
    enum Bullet {
        BulletOfuda = MOD_BULLET_TYPE_OFUDA,
        BulletNormal = MOD_BULLET_TYPE_NORMAL,
        BulletEnergy = MOD_BULLET_TYPE_ENERGY,
        BulletBody = MOD_BULLET_TYPE_BODY,
        BulletLiquid = MOD_BULLET_TYPE_LIQUID,
        BulletLight = MOD_BULLET_TYPE_LIGHT,
        BulletLaser = MOD_BULLET_TYPE_LASER,
        BulletSharp = MOD_BULLET_TYPE_SHARP,
        BulletHeavy = MOD_BULLET_TYPE_HEAVY,
        BulletMissile = MOD_BULLET_TYPE_MISSILE,
        BulletSlash = MOD_BULLET_TYPE_SLASH
    };
    
    enum Stat {
        YangAttack = MOD_YANG_ATK,
        YangDefense = MOD_YANG_DEF,
        YinAttack = MOD_YIN_ATK,
        YinDefense = MOD_YIN_DEF,
        Evasion = MOD_EVASION,
        Accuracy = MOD_ACCURACY,
        CritAttack = MOD_CRIT_ATK,
        CritAccuracy = MOD_CRIT_ACC,
        Agility = MOD_AGILITY,
        CritDefense = MOD_CRIT_DEF,
        CritEvasion = MOD_CRIT_EVA
    };
    
    typedef std::variant<Modifier<Stat>, Modifier<Bullet>, Modifier<Element>> Variant;
    
    struct variantComparison { bool operator () (const Variant &a, const Variant &b) const; };
    typedef std::set<ModifierType::Variant, variantComparison> Container;
    
    static Variant variantFromC(unsigned int type, int value, unsigned int duration);
    
    static int value(Variant variant);
    
    static int duration(Variant variant);
    
    static ModifierType::Element fromElement(::Element element);
    
    static ModifierType::Bullet fromBullet(::BulletType bullet);
};

template <typename T>
class Modifier
{
private:
    T type;
    int value;
    int duration;
        
public:
    Modifier(T type, int value = 0, int duration = 0): type(type), value(value), duration(duration)
    {
        
    }
    
    template <typename U>
    Modifier<T>& operator+=(const Modifier<U>& rhs)
    {
        if constexpr(!std::is_same_v<T,U>)
            assert(!"Can't do arithmetic operations on modifiers of different types");
        
        if constexpr(std::is_same_v<T, ModifierType::Stat>)
        {
            value = std::clamp(value + rhs.getValue(), -10, 10);
            duration = std::max(duration, rhs.getDuration());
        }
        else
        {
            value += rhs.getValue();
        }
        
        return *this;
    }
    
    template <typename U>
    friend Modifier<T> operator+(Modifier<T> lhs, const Modifier<U>& rhs)
    {
        lhs += rhs;
        return lhs;
    }
    
    Modifier<T> operator-() const
    {
        return Modifier(type, -value, duration);
    }
    
    template <typename U>
    Modifier<T>& operator-=(const Modifier<U>& rhs)
    {
        return *this += (-rhs);
    }
    
    template <typename U>
    friend Modifier<T> operator-(Modifier<T> lhs, const Modifier<U>& rhs)
    {
        lhs -= rhs;
        return lhs;
    }
    
    Modifier<T>& operator--()
    {
        duration--;
        return *this;
    }
    
    Modifier<T>& operator--(int)
    {
        Modifier<T> &old = *this;
        operator--();
        return old;
    }
    
    T getType() const { return type; }
    int getValue() const { return value; }
    int getDuration() const { return duration; }
    
    template <typename U>
    friend bool operator<(const Modifier<T>& l, const Modifier<U>& r)
    {
        // Element < Bullet < Stat
        if constexpr(std::is_same_v<T,U>)
            return l.type < r.getType();
        else if constexpr(std::is_same_v<T, ModifierType::Stat>)
            return false;
        else if constexpr(std::is_same_v<T, ModifierType::Element>)
            return true;
        else if constexpr(std::is_same_v<U, ModifierType::Stat>)
            return true;
        else if constexpr(std::is_same_v<U, ModifierType::Element>)
            return false;
        else
            static_assert(std::is_same_v<T,U>, "Unhandled modifier comparison case");
    }
    
    friend ModifierType;
};

#endif /* Modifier_hpp */
