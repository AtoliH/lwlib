/**
 * \todo optimization switch to vector data type, use step function instead of branching, unroll loops, use macros to decide loop length at compile time, small register usage optimization
 */
void kernel dmg_calc(global const struct story_card* SC, global const struct lw_character* FRIENDS, global const struct lw_character* ENNEMIES, global uint* OUT)
{
    char number_of_dimensions = get_work_dim();
    uint global_id_1 = get_global_id(0);
    uint global_id_2 = get_global_id(1);
    uint global_id_3 = get_global_id(2);
    // unsigned int is probably too small for this, will have to switch to unsigned long
    uint total_index = global_id_1+(global_id_2+global_id_3*get_global_size(1))*get_global_size(0);
    
    //printf("id1: %u - id2: %u - id3: %u\n", global_id_1, global_id_2, global_id_3);

    short i;

    // uint spirit_power = 1;
    uint danmaku_level = 5; // todo: depends on awakening or spirit power if shot

    struct lw_character targets[3] = { ENNEMIES[0], ENNEMIES[0], ENNEMIES[0] };

    struct lw_character attacker = FRIENDS[0];
        
    struct story_card story_cards[7] = { SC[global_id_1], SC[1], SC[2], SC[global_id_2], SC[global_id_3] };

    // Do all 5 story cards
    for (i = 0; i < 5; i++)
    {
        struct story_card sc = story_cards[i];
        attacker.stat[sc.stat_type_1] += sc.stat_value_1;
        attacker.stat[sc.stat_type_2] += sc.stat_value_2;
    }
    
    // struct turn_action first_action = { 1, 0, 1, 0 }; // spell boost graze skills
    struct turn_action actions[5] = {
        { 1, 0, 1, 0 },
        { 0, 0, 1, 0 },
        { 2, 1, 1, SKILL_1 | SKILL_2 },
        { 3, 1, 1, SKILL_3 },
        { 4, 3, 0, 0 }
    };
    
    int overall_total_dmg = 0;
    bool already_broken = false;
    short t;
    for (t = 0; t < 5; t++)
    {
        struct turn_action action = actions[t];
        
        // Use skills
        if (action.skill & SKILL_1)
        {
            struct skill s = attacker.skills[0];
            for (i = 0; i < SKILL_MAX_MODIFIERS; i++)
            {
                uint mod = s.modifier_type[i];
                int value = attacker.modifier_value[mod];
                uint duration = attacker.modifier_duration[mod];
                attacker.modifier_value[mod] = clamp(value + s.modifier_value[i], -10, 10);
                attacker.modifier_duration[mod] = max(duration, s.modifier_duration[i]);
            }
        }
        if (action.skill & SKILL_2)
        {
            struct skill s = attacker.skills[1];
            for (i = 0; i < SKILL_MAX_MODIFIERS; i++)
            {
                uint mod = s.modifier_type[i];
                int value = attacker.modifier_value[mod];
                uint duration = attacker.modifier_duration[mod];
                attacker.modifier_value[mod] = clamp(value + s.modifier_value[i], -10, 10);
                attacker.modifier_duration[mod] = max(duration, s.modifier_duration[i]);
            }
        }
        if (action.skill & SKILL_3)
        {
            struct skill s = attacker.skills[2];
            for (i = 0; i < SKILL_MAX_MODIFIERS; i++)
            {
                uint mod = s.modifier_type[i];
                int value = attacker.modifier_value[mod];
                uint duration = attacker.modifier_duration[mod];
                attacker.modifier_value[mod] = clamp(value + s.modifier_value[i], -10, 10);
                attacker.modifier_duration[mod] = max(duration, s.modifier_duration[i]);
            }
        }
        
        // Story card buffs
        struct story_card sc = story_cards[action.spell];
        //printf("spell: %hd - %u/%u - mod: %d - atk mod: \n", action.spell, sc.stat_value_1, sc.stat_value_2, sc.modifier_value[0]);
        for (i = 0;i < SC_MAX_MODIFIERS; i++)
        {
            uint mod = sc.modifier_type[i];
            int value = attacker.modifier_value[mod];
            uint duration = attacker.modifier_duration[mod];
            // this is awful, must clean it up
            float mod_step = (1.0f - step((float)20.0f, (float)mod)) * 1000;
            float min_mod_value = -10.0f - mod_step;
            float max_mod_value = +10.0f + mod_step;
            attacker.modifier_value[mod] = clamp((float)(value + sc.modifier_value[i]), min_mod_value, max_mod_value);
            attacker.modifier_duration[mod] = max(duration, sc.modifier_duration[i]);
        }

        struct attack atk = attacker.spell[action.spell];

        // Spell buffs
        for (i = 0; i < SPELL_MAX_MODIFIERS; i++)
        {
            uint mod = atk.pre_modifier_type[i];
            int value = attacker.modifier_value[mod];
            uint duration = attacker.modifier_duration[mod];
            attacker.modifier_value[mod] = clamp(value + atk.pre_modifier_value[i], -10, 10);
            attacker.modifier_duration[mod] = max(duration, atk.pre_modifier_duration[i]);
        }

        float atk_level = attacker.level;
        float danmaku_modifier = atk.danmaku_modifier[danmaku_level-1];

        short number_of_lines = atk.boost_layout[action.boost];
        // todo: destroy grazed barriers
        
        /*if (total_index==115)
        {
            printf("yang_atk yang_def yin_atk yin_def evasion accuracy crit_atk crit_acc agility\n");
            for (i = 20;i < 29; i++)
            {
                printf("%d/%u      ", attacker.modifier_value[i], attacker.modifier_duration[i]);
            }
            printf("\n");
        }*/

        uint total_dmg = 0;
        uint total_dmg_lo = 0;
        uint total_dmg_hi = 0;
        for (i = 0; i < number_of_lines; i++) // Possible optimization by looping 6 times and multiplying by zero
        {
            struct attack_line line = atk.line[i];
            
            // --- line dependant
            // If performance is a problem, may try (((-x) | x) >> (BITS - 1)) & 1 -> yields 1 if x contains a 0, 0 otherwise
            bool is_yang = (line.attribute == BULLET_ATTRIBUTE_YANG);

            float base_atk = is_yang? attacker.stat[STAT_YANG_ATK] : attacker.stat[STAT_YIN_ATK];
            int atk_modifier = is_yang? attacker.modifier_value[MOD_YANG_ATK] : attacker.modifier_value[MOD_YIN_ATK];
            float atk_multiplier = pown((float)(1 + abs((int)atk_modifier) * 0.3), (int)sign((float)atk_modifier));

            float base_hard = is_yang? attacker.stat[STAT_YANG_DEF] : attacker.stat[STAT_YIN_DEF];
            
            float special_atk = line.hard * base_hard + line.slicing * attacker.stat[STAT_AGILITY];
            
            float total_atk = (base_atk + special_atk) * atk_multiplier;
            // printf("--- base atk: %f - atk mul: %f - special: %f\n", base_atk, atk_multiplier, special_atk);

            int element_buff = attacker.modifier_value[8-clz(line.element)];
            int bullet_buff = attacker.modifier_value[line.bullet];
            float shot_buffs = 1 + (element_buff + bullet_buff)/100.0f;
            // if (total_index == 19)
            //    printf("--- shot_buffs: %f - element buff: %d - bullet buff: %d - element: %d - bullet: %d\n", shot_buffs, element_buff, bullet_buff, 8-clz(line.element), line.bullet);

            float shot_power = line.pow;

            float random_multiplier_low = 0.336; // Assuming lowest, ranges from 0.336 to 0.4
            float random_multiplier_high = 0.4;

            float bullet_count = line.bullet_quantity;
            
            // Target dependant multipliers
            int number_of_targets = atk.targetting;
            number_of_targets = 1;
            ushort target_id = number_of_targets == 1? action.target:0;
            short j;
            for (j = target_id; j < number_of_targets+target_id; j++)
            {
                struct lw_character target = targets[j];
                
                float def_level = target.level;
                float targetting_resistance = 1;
                float trait_resistance = 1;
                
                float base_def = is_yang? target.stat[STAT_YANG_DEF] : target.stat[STAT_YIN_DEF];
                float def_modifier =  (is_yang? target.modifier_value[MOD_YANG_DEF] : target.modifier_value[MOD_YIN_DEF]) * (1 - line.pierce);
                float def_multiplier = pown((float)(1 + abs((int)def_modifier) * 0.3), (int)sign((float)def_modifier));
                
                float total_def = base_def * def_multiplier;
                
                // Accuracy depends on target because of evasion
                int acc_modifier = min((int)10, (int)max((int)(-10), (int)(attacker.modifier_value[MOD_ACCURACY] - target.modifier_value[MOD_EVASION])));
                float acc_multiplier = pown((float)(1 + 0.2 * abs(acc_modifier)), (int)sign((float)acc_modifier));
                float accuracy = min(1.0f, line.hit * acc_multiplier);
                uint is_accurate = (uint)accuracy;
                
                // check for barrier break in worst case scenario (everything misses if not 100% accurate)
                uint element_break = (line.element & target.weakness) * is_accurate;

                uint pre_break = popcount(target.barriers);
                // break barriers
                target.barriers &= ~(line.status_breaks * is_accurate); // anomaly breaks
                uint mid_break = popcount(target.barriers);
                target.barriers &= ~(element_break << (BARRIER_MAX_AMOUNT - popcount(target.barriers))); // element break
                uint post_break = popcount(target.barriers);
                bool broken = target.barriers == 0;
                float broken_multiplier = already_broken? 4.0f:1.0f;
                bool already_broken = (!already_broken) && broken;

                float weakness_multiplier = (line.element & target.weakness)? 2.0f:1.0f;
                float resistance_multiplier = (line.element & target.resistance)? 0.5f:1.0f;
                
                // crit depends on target because of killers, crit def and crit eva
                int crit_atk_modifier = attacker.modifier_value[MOD_CRIT_ATK];
                float crit_atk_base = pown((float)(1 + 0.3 * abs(crit_atk_modifier)), (int)sign((float)crit_atk_modifier));
                int crit_def_modifier = target.modifier_value[MOD_CRIT_DEF];
                float crit_def_base = pown((float)(1 + 0.3 * abs(crit_def_modifier)), (int)sign((float)crit_def_modifier));
                // Not taking crit eva into account for now, only killers
                float crit_multiplier = 1 + crit_atk_base / crit_def_base;

                int crit_acc_modifier = min((int)10, (int)max((int)(-10), (int)(attacker.modifier_value[MOD_CRIT_ACC] - target.modifier_value[MOD_CRIT_EVA])));
                float crit_acc_multiplier = pown((float)(1 + 0.2 * abs(crit_acc_modifier)), (int)sign((float)crit_acc_modifier));
                float crit_acc_base = (target.races & line.killers)? 1.0f : line.crit;
                float crit_accuracy = min(1.0f, crit_acc_base * crit_acc_multiplier);
                
                float expected_crit_multiplier = crit_accuracy * crit_multiplier + 1 - crit_accuracy;
            
                if (t == 0 && i == 0)
                printf("total atk: %f - atk level: %f - shot pow: %f - shot buff: %f - crit: %f - danmaku: %f - bullets: %f - weak: %f - resist: %f - broken: %f - total def: %f - trait resist: %f - target resist: %f - def level: %f - accuracy: %f - crit acc: %f - crit mul: %f - expected crit: %f\n", total_atk, atk_level, shot_power, shot_buffs, crit_multiplier, danmaku_modifier, bullet_count, weakness_multiplier, resistance_multiplier, broken_multiplier, total_def, trait_resistance, targetting_resistance, def_level, accuracy, crit_accuracy, crit_multiplier, expected_crit_multiplier);
                float hit_dmg_base = (total_atk * atk_level * shot_power * shot_buffs * danmaku_modifier * bullet_count * weakness_multiplier * resistance_multiplier * broken_multiplier) / (total_def * trait_resistance * targetting_resistance * def_level);

                float hit_dmg_low = hit_dmg_base * random_multiplier_low * expected_crit_multiplier;
                float hit_dmg_very_low = hit_dmg_low * accuracy;
                float hit_dmg_extremely_low = hit_dmg_very_low;//* is_accurate;
            
                float hit_dmg_extremely_high = hit_dmg_base * random_multiplier_high * expected_crit_multiplier;
                float hit_dmg_high = hit_dmg_extremely_high * accuracy;
                
                //printf("#%d elo: %f - w: %f - r: %f - c: %f - a: %f - h: %f - s: %f d: %f - b: %u>%u>%u %f\n", i+1, hit_dmg_extremely_low, weakness_multiplier, resistance_multiplier, crit_multiplier, accuracy, line.hard, line.slicing, def_modifier, pre_break, mid_break, post_break, broken_multiplier);
                //if (total_index == 19) printf("#%d elo: %f - element buff: %f - bullet buff: %f\n", i+1, hit_dmg_extremely_low, (float)element_buff, (float)bullet_buff);
                //printf("#%d - w: %f - r: %f - c: %f - a: %f - b: %u>%u>%u %f\n", i+1, weakness_multiplier, resistance_multiplier, crit_multiplier, accuracy, pre_break, mid_break, post_break, broken_multiplier);
                                               
                overall_total_dmg += hit_dmg_low;
                total_dmg += hit_dmg_extremely_low;
                total_dmg_lo += hit_dmg_very_low;
                total_dmg_hi += hit_dmg_extremely_high;
                
                targets[j].stat[STAT_HEALTH] = sub_sat(targets[j].stat[STAT_HEALTH], hit_dmg_extremely_low);
            }
        }

        for (i = 0; i < SPELL_MAX_MODIFIERS; i++)
        {
            uint mod = atk.post_modifier_type[i];
            int value = attacker.modifier_value[mod];
            uint duration = attacker.modifier_duration[mod];
            attacker.modifier_value[mod] = clamp(value + atk.post_modifier_value[i], -10, 10);
            attacker.modifier_duration[mod] = max(duration, atk.post_modifier_duration[i]);
        }

        ushort skills_used = ((action.skill&(1<<2))?100:0)+((action.skill&(1<<1))?20:0)+((action.skill&(1<<0))?3:0);
        printf("turn %02hd - target %hd - boost %hd - graze %hd - skills %03hd - spell card %hd - ", t+1, action.target, action.boost, action.graze, skills_used, (short)(action.spell+1));
        printf("no: %06u - lo: %06u - hi: %06u\n", total_dmg, total_dmg_lo, total_dmg_hi);
        //printf("yang_atk yang_def yin_atk yin_def evasion accuracy crit_atk crit_acc agility\n");
        for (i = 0;i < NUMBER_OF_UNIQUE_MODIFIERS; i++)
        {
            // At the end of the turn, each mod turn > 0, -1, if turns = 0, mod val = 0
            attacker.modifier_duration[i] = fdim((float)attacker.modifier_duration[i], (float)1.0f);
            attacker.modifier_value[i] = attacker.modifier_duration[i]? attacker.modifier_value[i] : 0;
            //if (i >= 20) printf("%d/%u      ", attacker.modifier_value[i], attacker.modifier_duration[i]);
        }
        //printf("\n");
    }

    printf("overall total damage: %d", overall_total_dmg);
    //printf("#%d: %u\n", (int)global_id_1+(global_id_2+global_id_3*get_global_size(1))*get_global_size(0), targets[0].stat[STAT_HEALTH]);
    OUT[total_index] = (uint)(targets[0].stat[STAT_HEALTH]);
}
