typedef uint sc_stat_type;
typedef uint bullet_type;
typedef uchar bullet_element;
typedef uint bullet_attribute;
typedef uint attack_targetting;

typedef struct __attribute__ ((packed)) turn_action
{
    char spell; // 1 - 7 sc1 sc2 sc3 sc4 lw ss fs
    char boost; // 0 1 2 3
    char graze; // 0 1 2 3
    char skill; // bitmask skill1 skill2 skill3
    char target; // 0 1 2 mid top bot
} turn_action;

typedef struct __attribute__ ((packed)) skill
{
    uint modifier_type[SKILL_MAX_MODIFIERS];
    int modifier_value[SKILL_MAX_MODIFIERS];
    uint modifier_duration[SKILL_MAX_MODIFIERS];
} skill;

typedef struct __attribute__ ((packed)) story_card
{
    sc_stat_type stat_type_1;
    uint stat_value_1;
    sc_stat_type stat_type_2;
    uint stat_value_2;

    uint modifier_type[SC_MAX_MODIFIERS];
    int modifier_value[SC_MAX_MODIFIERS];
    uint modifier_duration[SC_MAX_MODIFIERS];
} story_card;

typedef struct __attribute__((packed)) attack_line
{
    float pow;
    float hit;
    float crit;
    bullet_attribute attribute;
    bullet_type bullet;
    bullet_element element;
    uint bullet_quantity;
    ulong status_breaks;
    ulong killers;
    float hard;
    float slicing;
    uchar pierce; // 1 if line has piercing, 0 otherwise, nullifies def buffs/debuffs
    uchar specular;
    uchar elastic;
    uchar precise;
    uchar explosive;
} attack_line;

typedef struct __attribute__((packed)) attack
{
    struct attack_line line[6];
    uint danmaku_modifier[5];
    uint boost_layout[4];
    attack_targetting targetting;

    uint pre_modifier_type[SPELL_MAX_MODIFIERS];
    int pre_modifier_value[SPELL_MAX_MODIFIERS];
    uint pre_modifier_duration[SPELL_MAX_MODIFIERS];
    
    uint post_modifier_type[SPELL_MAX_MODIFIERS];
    int post_modifier_value[SPELL_MAX_MODIFIERS];
    uint post_modifier_duration[SPELL_MAX_MODIFIERS];
} attack;
       
typedef struct __attribute__((packed)) lw_character
{
    struct attack spell[7];

    uint level;
    uint weakness;
    uint resistance;
    uint stat[7];

    ulong races;

    ulong barriers;
    uint barrier_status_duration[BARRIER_MAX_AMOUNT];
        
    int modifier_value[NUMBER_OF_UNIQUE_MODIFIERS];
    uint modifier_duration[NUMBER_OF_UNIQUE_MODIFIERS];

    struct skill skills[3];
} lw_character;
