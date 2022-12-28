//
//  macro.h
//  lwcalculator
//
//  Created by Fire937 on 6/15/22.
//

#ifndef macro_h
#define macro_h

#define STAT_BLANK    0
#define STAT_HEALTH   1
#define STAT_AGILITY  2
#define STAT_YANG_ATK 3
#define STAT_YANG_DEF 4
#define STAT_YIN_ATK  5
#define STAT_YIN_DEF  6

#define BULLET_TYPE_OFUDA    8
#define BULLET_TYPE_NORMAL   9
#define BULLET_TYPE_ENERGY   10
#define BULLET_TYPE_BODY     11
#define BULLET_TYPE_LIQUID   12
#define BULLET_TYPE_LIGHT    13
#define BULLET_TYPE_LASER    14
#define BULLET_TYPE_SHARP    15
#define BULLET_TYPE_HEAVY    16
#define BULLET_TYPE_MISSILE  17
#define BULLET_TYPE_SLASH    18

#define ELEMENT_NONE  0x00000000
#define ELEMENT_SUN   (1 << 0)
#define ELEMENT_MOON  (1 << 1)
#define ELEMENT_FIRE  (1 << 2)
#define ELEMENT_WATER (1 << 3)
#define ELEMENT_WOOD  (1 << 4)
#define ELEMENT_METAL (1 << 5)
#define ELEMENT_EARTH (1 << 6)
#define ELEMENT_STAR  (1 << 7)
#define ELEMENT_ALL   0xFFFFFFFF

#define RACE_HUMAN              (1 << 0)
#define RACE_YOUKAI             (1 << 1)
#define RACE_GENSOKYO           (1 << 2)
#define RACE_MAGICIAN           (1 << 3)
#define RACE_INCIDENT_RESOLVER  (1 << 4)
#define RACE_TENGU              (1 << 5)

#define BULLET_ATTRIBUTE_YIN 0x0
#define BULLET_ATTRIBUTE_YANG 0x1

#define ATTACK_TARGETTING_SOLO 1
#define ATTACK_TARGETTING_ALL 3

// 0 -> 7 is for element modifier, MOD_ELEMENT_XXX = ctz(ELEMENT_XXX)
#define MOD_ELEMENT_NONE  0
#define MOD_ELEMENT_SUN   1
#define MOD_ELEMENT_MOON  2
#define MOD_ELEMENT_FIRE  3
#define MOD_ELEMENT_WATER 4
#define MOD_ELEMENT_WOOD  5
#define MOD_ELEMENT_METAL 6
#define MOD_ELEMENT_EARTH 7
#define MOD_ELEMENT_STAR  8
#define MOD_ELEMENT_LAST MOD_ELEMENT_STAR
// 8 -> 18 is for bullet modifier, MOD_BULLET_TYPE_XXX = BULLET_TYPE_XXX
#define MOD_BULLET_TYPE_OFUDA    9
#define MOD_BULLET_TYPE_NORMAL   10
#define MOD_BULLET_TYPE_ENERGY   11
#define MOD_BULLET_TYPE_BODY     12
#define MOD_BULLET_TYPE_LIQUID   13
#define MOD_BULLET_TYPE_LIGHT    14
#define MOD_BULLET_TYPE_LASER    15
#define MOD_BULLET_TYPE_SHARP    16
#define MOD_BULLET_TYPE_HEAVY    17
#define MOD_BULLET_TYPE_MISSILE  18
#define MOD_BULLET_TYPE_SLASH    19
#define MOD_BULLET_LAST MOD_BULLET_TYPE_SLASH
// 19+ other modifiers
#define MOD_YANG_ATK 20
#define MOD_YANG_DEF 21
#define MOD_YIN_ATK  22
#define MOD_YIN_DEF  23
#define MOD_EVASION  24
#define MOD_ACCURACY 25
#define MOD_CRIT_ATK 26
#define MOD_CRIT_ACC 27
#define MOD_AGILITY  28
#define MOD_CRIT_DEF 29
#define MOD_CRIT_EVA 30

#define SC_MAX_MODIFIERS 4
#define SPELL_MAX_MODIFIERS 4
#define SKILL_MAX_MODIFIERS 4
#define NUMBER_OF_UNIQUE_MODIFIERS 31

#define BARRIER_BROKEN    0
#define BARRIER_HEALTHY   (1 << 0)
#define BARRIER_POISON    (1 << 1)
#define BARRIER_FREEZE    (1 << 2)
#define BARRIER_PARALYZE  (1 << 3)
#define BARRIER_BLIND     (1 << 4)
#define BARRIER_BURN      (1 << 5)
#define BARRIER_NUMBER_OF_STATES 6
#define FIRST_BARRIER_MASK 0x3F

#define BARRIER_MAX_AMOUNT 7

#define SKILL_1 (1 << 2)
#define SKILL_2 (1 << 1)
#define SKILL_3 (1 << 0)

#define MAXIMUM_ARRAY_SIZE 4

#endif /* macro_h */
