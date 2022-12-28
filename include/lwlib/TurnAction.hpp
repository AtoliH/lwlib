//
//  TurnAction.hpp
//  lwcalculator
//
//  Created by Fire937 on 6/16/22.
//

#ifndef TurnAction_hpp
#define TurnAction_hpp

#include <vector>
#include <set>

#include <iostream>

struct TurnAction
{
    enum Attack {
        firstSpellCard = 0,
        secondSpellCard = 1,
        thirdSpellCard = 2,
        fourthSpellCard = 3,
        lastWord = 4,
        spreadShot = 5,
        focusShot = 6
    };
    
    enum Boost {
        noBoost,
        boostOnce,
        boostTwice,
        boostThrice
    };
    
    enum Graze {
        noGraze,
        grazeOnce,
        grazeTwice,
        grazeThrice
    };
    
    enum Skill {
        firstSkill,
        secondSkill,
        thirdSkill
    };
    
    enum Target {
        topTarget,
        midTarget,
        botTarget
    };
    
    Attack attack;
    Boost boost;
    Graze graze;
    std::set<Skill> skills;
    Target target;
    
    template<class... Ss>
    TurnAction(Attack attack, Boost boost, Graze graze, Target target, Ss... skills):
    attack(attack), boost(boost), graze(graze), target(target), skills({skills...})
    {

    }
    
    TurnAction(Attack attack = spreadShot, Boost boost = noBoost, Graze graze = noGraze):
    TurnAction(attack, boost, graze, midTarget)
    {
        
    }

    /**
     * Test whether the attack is a shot or a spell
     * @param attack
     * @return true if it's a shot, false otherwise
     */
    static bool isShot(Attack attack)
    {
        const auto shot = { spreadShot, focusShot };
        return std::cend(shot) != std::find(std::cbegin(shot), std::cend(shot), attack);
    }

    /**
     * Test whether the attack is a shot or a spell
     * @param attack
     * @return true if it's a spell, false otherwise
     */
    static bool isSpell(Attack attack)
    {
        return !isShot(attack);
    }
    
    friend bool operator<(const TurnAction& l, const TurnAction& r)
    {
        return std::tie(l.attack, l.boost, l.graze, l.skills, l.target)
            < std::tie(r.attack, r.boost, r.graze, r.skills, r.target);
    }
};

#endif /* TurnAction_hpp */
