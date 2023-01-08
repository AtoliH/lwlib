//
//  Analyzer.cpp
//  lwcalculator
//
//  Created by Fire937 on 6/17/22.
//


#include <vector>
#include <deque>
#include <map>
#include <numeric>
#include <cmath>

#include <iostream>

#include "lwlib/TurnAction.hpp"
#include "lwlib/Character.hpp"
#include "lwlib/Analyzer.hpp"

extern "C" {
    #include "lwlib/internal/incbeta.h"
}

/**
 * Compute the probability of at least \param bulletHit bullets connecting
 */
double hitProbability(double accuracy, unsigned int bulletCount, unsigned int bulletHit)
{
    assert(bulletHit <= bulletCount);
    assert(0.0 <= accuracy && accuracy <= 1.0);
    return 1 - incbeta(bulletCount - bulletHit, 1 + bulletHit, 1 - accuracy);
}

int binomialCoefficient(const int n, const int k)
{
    assert(n >= k);
    assert(k >= 0);
    
    if (k == 0)
        return 1;
    
    // Recursion ->  (n, k) = (n - 1, k - 1) * n / k
    int step1 = n - k + 1;
    int step0;
    for (int i = 1; i < k; ++i)
        step1 = (step0 = step1) * (n - k + 1 + i) / (i + 1);
    
    return step1;
}

/**
 * Compute the probability of exactly \param bulletHit bullets connecting
 */
double exactHitProbability(double accuracy, unsigned int bulletCount, unsigned int bulletHit)
{
    assert(bulletHit <= bulletCount);
    assert(0.0f <= accuracy && accuracy <= 1.0f);
    
    return binomialCoefficient(bulletCount, bulletHit) * std::pow(accuracy, bulletHit) * std::pow(1 - accuracy, bulletCount - bulletHit);
}

/**
 * Compute the damage caused by a line if @param bulletHit bullets connect using random multiplier @param randomMultiplier
 * @param led
 * @param bulletHit
 * @param randomMultiplier
 * @return
 */
unsigned int lineDamage(Character::LineEffectiveDamage led, unsigned int bulletHit, float randomMultiplier)
{
    return led.bulletDamage * (led.critAccuracy * led.critMultiplier + 1 - led.critAccuracy) * bulletHit * randomMultiplier;
}

/**
 * Compute the damage cause by a spell (i.e. an array of lines)
 * @param effectiveDamage
 * @param bulletHit
 * @param randomMultiplier
 * @return
 */
unsigned int totalDamage(const std::vector<Character::LineEffectiveDamage> &effectiveDamage, const std::vector<unsigned int> &bulletHit, float randomMultiplier)
{
    float damage = 0;
    unsigned int i = 0;
    for (auto led : effectiveDamage)
        damage += lineDamage(led, bulletHit[i++], randomMultiplier);
    return damage;
}

void analyzeTurnsConsistency(std::vector<TurnAction> actions, Character character, Character enemy)
{
    std::vector<Character::LineEffectiveDamage> inaccurateDamage;
    std::vector<unsigned int> bulletHitRight;
    std::vector<unsigned int> bulletHitLeft;
    std::vector<unsigned int> bulletHitMiddle;
    std::vector<float> bulletHitExpected;
    
    unsigned int guaranteedDamage = 0;
    
    Character initCharacter(character); // save initial character state
    
    float randomMultiplierLow = 0.336;
    float randomMultiplierHigh = 0.4;
    for (auto action : actions)
    {
        auto skills = action.skills;
        for (auto skill : skills)
        {
            // todo Analyze skill rng here...

            character.useSkill(skill);
        }
        
        auto turnEffectiveDamage = character.attack(enemy, action.attack, action.boost, action.graze);
        for (const auto &led : turnEffectiveDamage)
        {
            if (led.accuracy >= 1.0f)
                guaranteedDamage += lineDamage(led, led.bulletCount, randomMultiplierLow);
            else if (led.bulletDamage != 0 && led.accuracy > 0 && led.bulletCount > 0)
                inaccurateDamage.push_back(led);
        }
    }
    
    int lineCount = static_cast<int>(inaccurateDamage.size());
    bulletHitRight.reserve(lineCount);
    bulletHitMiddle.reserve(lineCount);
    bulletHitLeft.reserve(lineCount);
    bulletHitExpected.reserve(lineCount);
    for (auto led : inaccurateDamage)
    {
        bulletHitRight.push_back(led.bulletCount);
        bulletHitMiddle.push_back(led.bulletCount / 2);
        bulletHitExpected.push_back(led.bulletCount * led.accuracy);
        bulletHitLeft.push_back(0);
    }
    
    unsigned int damageTarget = std::max<int>(0, static_cast<int>(enemy.effectiveStat(Character::Health)) - guaranteedDamage);
    auto maxBulletCountIndex = std::distance(bulletHitRight.begin(), std::max_element(bulletHitRight.begin(), bulletHitRight.end()));
    
    std::cout << "innacurate damage: " << totalDamage(inaccurateDamage, bulletHitRight, randomMultiplierLow) << "/" << damageTarget << std::endl;
    
    // Find success rate via (pseudo) binary search, very very approximate, not really viable, can only be used to quickly bust a strategy
    /*while (bulletHitRight[maxBulletCountIndex] - bulletHitLeft[maxBulletCountIndex] > 1)
    {
        float damage = totalDamage(inaccurateDamage, bulletHitMiddle, randomMultiplierLow);
        if (damageTarget < damage)
            bulletHitRight.swap(bulletHitMiddle);
        else
            bulletHitLeft.swap(bulletHitMiddle);
        
        for (unsigned int i = 0; i < bulletHitMiddle.size(); ++i)
            bulletHitMiddle[i] = (bulletHitLeft[i] + bulletHitRight[i]) / 2;
    }*/
    
    // Find success rate by exploring all possible combinations
    int baseSubTarget = damageTarget;
    for (const auto &led : inaccurateDamage)
        baseSubTarget -= lineDamage(led, led.bulletCount, randomMultiplierLow);

    std::map<unsigned int, double> previousDamageProbability{{0, 1.0f}};
    std::map<unsigned int, double> damageProbability;
    unsigned int count = 0;
    for (int i = lineCount - 1; i >= 0; --i)
    {
        const auto &led = inaccurateDamage[i];
        
        unsigned int singleBulletDamage = lineDamage(led, 1, randomMultiplierLow);
                
        unsigned int bulletHit = 0;
        for (bulletHit = 0; bulletHit <= led.bulletCount; ++bulletHit)
        {
            unsigned int damage = singleBulletDamage * bulletHit;
            unsigned int subTarget = std::max<int>(0, baseSubTarget - damage);
            for (auto it = previousDamageProbability.lower_bound(subTarget); it != previousDamageProbability.end(); ++it)
            {
                unsigned int subTotalDamage = damage + it->first;
                double allLinesHitProbability = exactHitProbability(led.accuracy, led.bulletCount, bulletHit) * it->second;
                auto lb = damageProbability.lower_bound(subTotalDamage);
                if (lb != damageProbability.end() && !(damageProbability.key_comp()(subTotalDamage, lb->first)))
                    lb->second += allLinesHitProbability;
                else
                    damageProbability.insert(lb, {subTotalDamage, allLinesHitProbability});
                count++;
            }
        }
        
        baseSubTarget += singleBulletDamage * bulletHit;
        damageProbability.swap(previousDamageProbability);
        damageProbability.clear();
    }

    // Very accurate way to compute consistency, usually precise enough for our needs
    double exactConsistency = 0;
    for (const auto &[damage, probability] : previousDamageProbability)
        exactConsistency += probability;

    // Big shortcut to compute consistency, highly underestimates the real consistency as a lot of the probable branches are ignored
    double consistency = 1.0f;
    for (unsigned int i = 0; i < lineCount; ++i)
    {
        consistency *= hitProbability(inaccurateDamage[i].accuracy, inaccurateDamage[i].bulletCount, bulletHitRight[i]);
    }
    
    std::cout << "consistency: " << consistency << std::endl;
    std::cout << "exact consistency: " << exactConsistency << std::endl;


}

/**
 * Compute the accuracy of a bullet/line, taking the accuracy and evasion modifiers into account
 * @param baseAccuracy
 * @param accuracyModifier
 * @param evasionModifier
 * @return
 */
float computeAccuracy(float baseAccuracy, Modifier<ModifierType::Stat> accuracyModifier, Modifier<ModifierType::Stat> evasionModifier) {
    int aggregatedModifier = (accuracyModifier - evasionModifier).getValue();
    float accuracyMultiplier = std::pow(1 + 0.2f * std::abs(aggregatedModifier), (0 < aggregatedModifier) - (aggregatedModifier < 0));
    float accuracy = std::min(1.0f, baseAccuracy * accuracyMultiplier);
    return accuracy;
}

float computeDamage(float baseDamage) {

}

void analyzeTurns(std::vector<TurnAction> actions, Character character, Character enemy) {

}