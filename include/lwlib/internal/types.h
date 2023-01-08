//
//  types.h
//  lwcalculator
//
//  Created by Fire937 on 6/8/22.
//

#ifndef types_h
#define types_h

#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#ifdef __APPLE__
    #include "../thirdparty/OpenCL/cl.hpp"
#else
    #include <CL/cl.hpp>
#endif
#include "macro.h"

typedef cl_uint sc_stat_type;
typedef cl_uint bullet_type;
typedef cl_uchar bullet_element;
typedef cl_uint bullet_attribute; // YIN/YANG
typedef cl_uint attack_targetting; // SOLO/ALL

typedef struct __attribute__ ((packed)) turn_action
{
    cl_char spell; // 1 - 7 sc1 sc2 sc3 sc4 lw ss fs
    cl_char boost; // 0 1 2 3
    cl_char graze; // 0 1 2 3
    cl_char skill; // bitmask skill1 skill2 skill3
    cl_char target; // 0 1 2 mid top bot
} turn_action;

typedef struct __attribute__ ((packed)) skill
{
    cl_uint modifier_type[SKILL_MAX_MODIFIERS];
    cl_int modifier_value[SKILL_MAX_MODIFIERS];
    cl_uint modifier_duration[SKILL_MAX_MODIFIERS];
} skill;

typedef struct __attribute__ ((packed)) story_card
{
    sc_stat_type stat_type_1;
    cl_uint stat_value_1;
    sc_stat_type stat_type_2;
    cl_uint stat_value_2;
    
    cl_uint modifier_type[SC_MAX_MODIFIERS];
    cl_int modifier_value[SC_MAX_MODIFIERS];
    cl_uint modifier_duration[SC_MAX_MODIFIERS];
} story_card;

typedef struct __attribute__((packed)) attack_line
{
    cl_float pow;
    cl_float hit;
    cl_float crit;
    bullet_attribute attribute;
    bullet_type bullet;
    bullet_element element;
    cl_uint bullet_count;
    cl_ulong status_breaks;
    cl_ulong killers;
    cl_float hard;
    cl_float slicing;
    cl_ushort piercing;
    cl_ushort specular;
    cl_ushort elastic;
    cl_ushort precise;
    cl_ushort explosive;
} attack_line;

typedef struct __attribute__((packed)) attack
{
    attack_line line[6];
    cl_uint danmaku_modifier[5];
    cl_uint boost_layout[4];
    attack_targetting targetting;
    
    cl_uint pre_modifier_type[SPELL_MAX_MODIFIERS];
    cl_int pre_modifier_value[SPELL_MAX_MODIFIERS];
    cl_uint pre_modifier_duration[SPELL_MAX_MODIFIERS];
    
    cl_uint post_modifier_type[SPELL_MAX_MODIFIERS];
    cl_int post_modifier_value[SPELL_MAX_MODIFIERS];
    cl_uint post_modifier_duration[SPELL_MAX_MODIFIERS];
} attack;

typedef struct __attribute__((packed)) lw_character
{
    attack spell[7];
    
    cl_uint level;
    cl_uint weakness;
    cl_uint resistance;
    cl_uint stats[7];
    
    cl_ulong races;
    
    cl_ulong barriers;
    cl_uint barrier_status_duration[BARRIER_MAX_AMOUNT];
        
    cl_int modifier_value[NUMBER_OF_UNIQUE_MODIFIERS];
    cl_uint modifier_duration[NUMBER_OF_UNIQUE_MODIFIERS];
    
    skill skills[3];
} lw_character;

#endif /* types_h */
