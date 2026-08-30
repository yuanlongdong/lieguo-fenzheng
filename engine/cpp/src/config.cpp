#include "config.h"

namespace lieguo {

GameConfig CONFIG;

std::vector<Faction> createFactions() {
    std::vector<Faction> factions;

    // 秦
    Faction qin;
    qin.id = "qin"; qin.name = "秦"; qin.color = "#c0392b";
    qin.trait = "军功爵制"; qin.traitDesc = "兵力>敌方时，攻击+20%";
    qin.style = "进攻型，滚雪球"; qin.attackBonusWhenStrong = 0.20;
    factions.push_back(qin);

    // 楚
    Faction chu;
    chu.id = "chu"; chu.name = "楚"; chu.color = "#8e44ad";
    chu.trait = "广袤南疆"; chu.traitDesc = "城池耐久+30%";
    chu.style = "防守型，持久战"; chu.durabilityBonus = 0.30;
    factions.push_back(chu);

    // 齐
    Faction qi;
    qi.id = "qi"; qi.name = "齐"; qi.color = "#2980b9";
    qi.trait = "渔盐之利"; qi.traitDesc = "免费点赞产兵+25%";
    qi.style = "人海型，免费玩家友好"; qi.freeOutputBonus = 0.25;
    factions.push_back(qi);

    // 燕
    Faction yan;
    yan.id = "yan"; yan.name = "燕"; yan.color = "#16a085";
    yan.trait = "荆轲刺秦"; yan.traitDesc = "武将技能伤害+30%";
    yan.style = "技战术型，武将核心"; yan.skillDamageBonus = 0.30;
    factions.push_back(yan);

    // 赵
    Faction zhao;
    zhao.id = "zhao"; zhao.name = "赵"; zhao.color = "#d35400";
    zhao.trait = "胡服骑射"; zhao.traitDesc = "可跨越1格攻击";
    zhao.style = "机动型，奇袭"; zhao.canRangedAttack = true;
    factions.push_back(zhao);

    // 魏
    Faction wei;
    wei.id = "wei"; wei.name = "魏"; wei.color = "#27ae60";
    wei.trait = "魏武卒"; wei.traitDesc = "步兵防御+25%，守城耐久消耗-20%";
    wei.style = "铁壁型，防守反击";
    wei.defenseBonus = 0.25; wei.siegeDefenseBonus = 0.20;
    factions.push_back(wei);

    // 韩
    Faction han;
    han.id = "han"; han.name = "韩"; han.color = "#f39c12";
    han.trait = "劲弩利剑"; han.traitDesc = "弓兵攻击+35%，爱的爆炸伤害+50%";
    han.style = "技能型，礼物爆发";
    han.archerBonus = 0.35; han.bombDamageBonus = 0.50;
    factions.push_back(han);

    return factions;
}

std::vector<City> createCities() {
    std::vector<City> cities;
    auto mk = [](const std::string& id, const std::string& name, CityType type,
                 const std::string& faction, int x, int y,
                 const std::vector<std::string>& neighbors) {
        City c; c.id = id; c.name = name; c.type = type;
        c.faction = faction; c.x = x; c.y = y; c.neighbors = neighbors;
        return c;
    };

    // 秦国
    cities.push_back(mk("xianyang","咸阳",CityType::Capital,"qin",120,280,{"hangu","yong"}));
    cities.push_back(mk("hangu","函谷关",CityType::Fortress,"qin",200,250,{"xianyang","yong","luoyang"}));
    cities.push_back(mk("yong","雍城",CityType::Normal,"qin",100,350,{"xianyang","hangu"}));
    // 楚国
    cities.push_back(mk("ying","郢都",CityType::Capital,"chu",280,480,{"yiling","jiangling"}));
    cities.push_back(mk("yiling","夷陵",CityType::Fortress,"chu",200,450,{"ying","jiangling","yong"}));
    cities.push_back(mk("jiangling","江陵",CityType::Normal,"chu",350,520,{"ying","yiling"}));
    // 齐国
    cities.push_back(mk("linzi","临淄",CityType::Capital,"qi",620,180,{"jimo","ju"}));
    cities.push_back(mk("jimo","即墨",CityType::Fortress,"qi",680,250,{"linzi","ju","qufu"}));
    cities.push_back(mk("ju","莒城",CityType::Normal,"qi",580,250,{"linzi","jimo"}));
    // 燕国
    cities.push_back(mk("ji","蓟城",CityType::Capital,"yan",550,60,{"zhuoxian","shanggu"}));
    cities.push_back(mk("zhuoxian","涿县",CityType::Normal,"yan",500,120,{"ji","shanggu","handan"}));
    cities.push_back(mk("shanggu","上谷",CityType::Normal,"yan",620,100,{"ji","zhuoxian"}));
    // 赵国
    cities.push_back(mk("handan","邯郸",CityType::Capital,"zhao",450,180,{"taiyuan","zhuoxian","xinzheng"}));
    cities.push_back(mk("taiyuan","太原",CityType::Normal,"zhao",380,150,{"handan","linfen"}));
    cities.push_back(mk("linfen","临汾",CityType::Normal,"zhao",320,220,{"taiyuan","handan","daliang"}));
    // 魏国
    cities.push_back(mk("daliang","大梁",CityType::Capital,"wei",450,320,{"shangqiu","xinzheng","linfen"}));
    cities.push_back(mk("shangqiu","商丘",CityType::Fortress,"wei",520,380,{"daliang","xuzhou","qufu"}));
    cities.push_back(mk("xuzhou","徐州",CityType::Normal,"wei",580,350,{"shangqiu","jimo"}));
    // 韩国
    cities.push_back(mk("xinzheng","新郑",CityType::Capital,"han",380,280,{"yangdi","shangdang","daliang","luoyang"}));
    cities.push_back(mk("yangdi","阳翟",CityType::Normal,"han",320,340,{"xinzheng","yiling","shangdang"}));
    cities.push_back(mk("shangdang","上党",CityType::Normal,"han",350,230,{"xinzheng","yangdi","linfen","taiyuan"}));
    // 中立
    cities.push_back(mk("luoyang","洛阳",CityType::Fortress,"",280,300,{"hangu","xinzheng","linfen"}));
    cities.push_back(mk("qufu","曲阜",CityType::Normal,"",550,280,{"jimo","shangqiu","ju"}));

    // 设置耐久
    for (auto& c : cities) {
        c.maxDurability = CONFIG.cityDurability[c.type];
        c.durability = c.maxDurability;
        // 楚国耐久加成
        if (c.faction == "chu") {
            c.maxDurability = static_cast<int>(c.maxDurability * 1.3);
            c.durability = c.maxDurability;
        }
    }
    return cities;
}

std::vector<General> getGeneralPool() {
    std::vector<General> pool;
    auto mk = [](const std::string& id, const std::string& name, GeneralQuality q,
                 const std::string& faction, const std::string& passive,
                 const std::string& skill, const std::string& skillDesc) {
        General g; g.id = id; g.name = name; g.quality = q;
        g.faction = faction; g.passive = passive;
        g.skill = skill; g.skillDesc = skillDesc;
        return g;
    };
    // 橙将
    auto bq = mk("baiqi","白起",GeneralQuality::Orange,"qin","击杀回复20%","人屠","对目标造成15000伤害");
    bq.skillDamage = 15000; pool.push_back(bq);
    auto wj = mk("wangjian","王翦",GeneralQuality::Orange,"qin","攻城速度+30%","灭楚","全属性+40%，20秒");
    wj.skillBuff = 0.40; wj.skillDuration = 20; pool.push_back(wj);
    auto lm = mk("limu","李牧",GeneralQuality::Orange,"zhao","守城防御+50%","破匈奴","召唤3000骑兵");
    lm.skillSummon = 3000; pool.push_back(lm);
    auto sb = mk("sunbin","孙膑",GeneralQuality::Orange,"qi","技能冷却-30%","围魏救赵","敌方攻击力归零10秒");
    pool.push_back(sb);
    // 紫将
    auto lp = mk("lianpo","廉颇",GeneralQuality::Purple,"zhao","步兵生命+30%","负荆请罪","城池耐久回复10000");
    lp.skillHeal = 10000; pool.push_back(lp);
    auto wq = mk("wuqi","吴起",GeneralQuality::Purple,"wei","步兵攻击+30%","魏武卒","步兵+5000");
    wq.skillSummon = 5000; pool.push_back(wq);
    auto ly = mk("leyi","乐毅",GeneralQuality::Purple,"yan","连下城池+攻击","伐齐","对目标造成12000伤害");
    ly.skillDamage = 12000; pool.push_back(ly);
    auto hf = mk("hanfei","韩非",GeneralQuality::Purple,"han","弓兵攻击+25%","法度","下次爱的爆炸伤害+100%");
    pool.push_back(hf);
    // 蓝将
    auto tj = mk("tianji","田忌",GeneralQuality::Blue,"qi","弓兵攻击+20%","赛马","攻速+50%，15秒");
    tj.skillBuff = 0.50; tj.skillDuration = 15; pool.push_back(tj);
    auto jk = mk("jingke","荆轲",GeneralQuality::Blue,"yan","技能伤害+20%","刺秦","对敌方首都造成5000伤害");
    jk.skillDamage = 5000; pool.push_back(jk);
    auto qy = mk("quyuan","屈原",GeneralQuality::Blue,"chu","耐久回复+100%","离骚","全阵营兵力+3000");
    qy.skillSummon = 3000; pool.push_back(qy);
    auto sbh = mk("shenbuhai","申不害",GeneralQuality::Blue,"han","技能冷却-20%","术治","随机敌将沉默30秒");
    pool.push_back(sbh);
    return pool;
}

std::vector<Gift> getGifts() {
    std::vector<Gift> gifts;
    auto mk = [](const std::string& id, const std::string& name, int price,
                 GiftEffect effect, const std::string& desc) {
        Gift g; g.id = id; g.name = name; g.price = price;
        g.effect = effect; g.desc = desc; return g;
    };
    auto g1 = mk("light","灯牌",1,GiftEffect::Join,"+20步兵"); g1.troops = 20; gifts.push_back(g1);
    auto g2 = mk("wand","仙女棒",10,GiftEffect::Archer,"+300弓兵"); g2.troops = 300; gifts.push_back(g2);
    auto g3 = mk("pill","能力药丸",30,GiftEffect::Buff,"攻速+40%，30秒"); g3.buff = 0.40; g3.duration = 30; gifts.push_back(g3);
    auto g4 = mk("mic","派对话筒",52,GiftEffect::General,"召唤随机武将"); g4.quality = "random"; gifts.push_back(g4);
    auto g5 = mk("bomb","爱的爆炸",99,GiftEffect::Damage,"目标城池-8000耐久"); g5.damage = 8000; gifts.push_back(g5);
    auto g6 = mk("mirror","魔法镜",199,GiftEffect::Buff,"全属性+25%，60秒"); g6.buff = 0.25; g6.duration = 60; gifts.push_back(g6);
    auto g7 = mk("drop","神秘空投",520,GiftEffect::General,"+8000兵+紫将"); g7.troops = 8000; g7.quality = "purple"; gifts.push_back(g7);
    auto g8 = mk("car","跑车",1200,GiftEffect::General,"橙将+全属性+30%"); g8.quality = "orange"; g8.buff = 0.30; g8.duration = 60; gifts.push_back(g8);
    auto g9 = mk("carnival","嘉年华",30000,GiftEffect::Capture,"直接夺取城池"); gifts.push_back(g9);
    auto g10 = mk("sabotage","离间计",299,GiftEffect::Sabotage,"50%拆散同盟"); gifts.push_back(g10);
    return gifts;
}

std::string cityTypeToString(CityType type) {
    switch (type) {
        case CityType::Capital: return "首都";
        case CityType::Fortress: return "重镇";
        case CityType::Normal: return "城池";
    }
    return "未知";
}

std::string qualityToString(GeneralQuality q) {
    switch (q) {
        case GeneralQuality::Orange: return "橙";
        case GeneralQuality::Purple: return "紫";
        case GeneralQuality::Blue: return "蓝";
    }
    return "未知";
}

GeneralQuality stringToQuality(const std::string& s) {
    if (s == "orange") return GeneralQuality::Orange;
    if (s == "purple") return GeneralQuality::Purple;
    return GeneralQuality::Blue;
}

} // namespace lieguo
