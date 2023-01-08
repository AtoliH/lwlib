//
//  Attack.cpp
//  lwcalculator
//
//  Created by Fire937 on 6/16/22.
//

#include "lwlib/Attack.hpp"

Attack::Line::Line(float power, float accuracy, float critAccuracy, int bulletCount,
                   Attack::Line::YinYang bulletYinYang, BulletType bulletType, Element bulletElement,
                   const std::map<Tag, TagInfo> &tags, const std::set<CharacterTag> &killers,
                   const ModifierContainer &modifiers) : power(power),
                                                         accuracy(accuracy),
                                                         critAccuracy(
                                                                 critAccuracy),
                                                         bulletCount(
                                                                 bulletCount),
                                                         bulletYinYang(
                                                                 bulletYinYang),
                                                         bulletType(bulletType),
                                                         bulletElement(
                                                                 bulletElement),
                                                         tags(tags),
                                                         killers(killers),
                                                         modifiers(modifiers){}
