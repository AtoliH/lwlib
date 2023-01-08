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
#include <map>

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

    enum Effect {
        Absorb
    };
    
    static ModifierType::Element fromElement(::Element element);
    
    static ModifierType::Bullet fromBullet(::BulletType bullet);
};

/**
 * Represents a modifier of any type or even an effect
 * @tparam T
 */
template <typename T>
class Modifier
{
private:
    int value;
    int duration;

    /**
     * rate and add are used for skills to increase the value
     * of the modifier by add with a certain rate
     * Alternatively, rate can be used for bullet modifiers
     * when the line has a random chance to apply a modifier
     * e.g. 50% chance for Yang ATK UP
     * In this case the value has to be set to 0 and
     * add will contain the value of the modifier
     */
    float rate;
    int add;

public:
    Modifier(int value = 0, int duration = 0, float rate = 0, int add = 0):
    value(value), duration(duration), rate(rate), add(add)
    {
        
    }
    
    template <typename U>
    Modifier<T>& operator+=(const Modifier<U>& rhs)
    {
        if constexpr(!std::is_same_v<T,U>) {
            // todo Throw error
            assert(!"Can't do arithmetic operations on modifiers of different types");
        }
        
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
        return Modifier(-value, duration);
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

    /**
     * Decrement duration, usually done after each turn
     * @return The new duration
     */
    int decrementDuration() {
        duration--;
        return duration;
    }

    int getValue() const { return value; }
    int getAdd() const { return add; }
    int getDuration() const { return duration; }
    float getRate() const { return rate; }

    friend ModifierType;
};

/**
 * Container that can container modifiers of any type
 *
 * todo Specialize so that it can only accept modifiers of certain types
 * for example a character only needs to hold Element, Bullet and Stat modifiers
 * to hold his current buffs.
 * If we specialize the container we will also need foreach specialization to
 * only loop over specific modifier types
 */
class ModifierContainer {
    std::map<ModifierType::Element, Modifier<ModifierType::Element>> elementModifiers;
    std::map<ModifierType::Bullet, Modifier<ModifierType::Bullet>> bulletModifiers;
    std::map<ModifierType::Stat, Modifier<ModifierType::Stat>> statModifiers;
    std::map<ModifierType::Effect, Modifier<ModifierType::Effect>> effectModifiers;

public:
    ModifierContainer(ModifierContainer &&) = default;
    ModifierContainer(const ModifierContainer &) = default;

    template <typename ...Ts>
    ModifierContainer(Ts && ... modifiers) {
        (insert(modifiers), ...);
    }

    template <typename T>
    bool has(T type) {
        return getModifiers<T>().find(type) != getModifiers<T>().end();
    }

    /**
     * Access specified modifier with bounds checking
     * Will throw if the modifier doesn't exist
     * @tparam T
     * @param type
     * @return
     */
    template <typename T>
    Modifier<T>& operator[](T type) {
        return getModifiers<T>().at(type);
    }

    template <typename T>
    const Modifier<T>& operator[](T type) const {
        return getModifiers<T>().at(type);
    }

    template <typename T>
    void erase(T type) {
        getModifiers<T>().erase(type);
    }

    // todo If modifier already exists, add to it instead of remplacing/doing nothing
    template <typename T>
    void insert(std::pair<T, Modifier<T>> modifier) {
        getModifiers<T>().emplace(modifier);
    }

    template <typename T>
    void insert(T &&type, Modifier<T> &&modifier) {
        getModifiers<T>().emplace(type, modifier);
    }

    template <typename T>
    void insert(const T &type, const Modifier<T> &modifier) {
        getModifiers<T>().emplace(type, modifier);
    }

    template <class Function>
    void forEachBuff(Function f) const {
        forEach<ModifierType::Element, ModifierType::Bullet, ModifierType::Stat>(f);
    }

    template <class Function>
    void forEachBuff(Function f) {
        forEach<ModifierType::Element, ModifierType::Bullet, ModifierType::Stat>(f);
    }

    template <class Function>
    void forEachEffect(Function f) {
        forEach<ModifierType::Effect>(f);
    }

    template <class Function>
    void forEachEffect(Function f) const{
        forEach<ModifierType::Effect>(f);
    }

private:
    template <class T>
    std::map<T, Modifier<T>> & getModifiers() {
        if constexpr(std::is_same_v<T, ModifierType::Element>)
            return elementModifiers;
        else if constexpr(std::is_same_v<T, ModifierType::Bullet>)
            return bulletModifiers;
        else if constexpr(std::is_same_v<T, ModifierType::Stat>)
            return statModifiers;
        else if constexpr(std::is_same_v<T, ModifierType::Effect>)
            return effectModifiers;
    }

    template <class T>
    const std::map<T, Modifier<T>> & getModifiers() const {
        if constexpr(std::is_same_v<T, ModifierType::Element>)
            return elementModifiers;
        else if constexpr(std::is_same_v<T, ModifierType::Bullet>)
            return bulletModifiers;
        else if constexpr(std::is_same_v<T, ModifierType::Stat>)
            return statModifiers;
        else if constexpr(std::is_same_v<T, ModifierType::Effect>)
            return effectModifiers;
    }

    template <class ...T, class Function>
    void forEach(Function f) {
        ([&](){
            for (auto &modifier : getModifiers<T>()) f(modifier);
        }, ...);
    }

    template <class ...T, class Function>
    void forEach(Function f) const {
        ([&](){
            for (const auto &modifier : getModifiers<T>()) f(modifier);
        }, ...);
    }
};

#endif /* Modifier_hpp */
