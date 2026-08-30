/**
 * 列国纷争 - 游戏配置
 * 所有游戏数据、数值参数集中在此
 */
const CONFIG = {
  // 游戏节奏
  gameSpeed: 1,           // 游戏速度倍率
  tickInterval: 1000,     // 游戏tick间隔(ms)
  maxGenerals: 3,         // 每阵营最多上阵武将数
  allianceDuration: 600,  // 同盟持续时间(秒)
  allianceUnlockTime: 1200, // 合纵解锁时间(秒) = 20分钟
  // 战斗参数
  cityDurability: {
    capital: 50000,
    normal: 20000,
    fortress: 30000
  },
  cityDefense: {
    capital: 1.5,
    normal: 1.2,
    fortress: 1.3
  },
  cityProduction: {
    capital: 5,
    normal: 2,
    fortress: 4
  },
  durabilityDecayCoeff: 20,  // 耐久削减系数
  siegeLossRate: 0.01,       // 攻城方每秒兵力损耗比例
  // 免费产出
  freeOutput: {
    like: 1,           // 每次点赞产兵
    comment: 5,        // 每条评论产兵
    join: 20,          // 加入阵营产兵
    share: 100         // 分享产兵
  },
  // 人数平衡
  balance: {
    overpopulatedRatio: 1.5,  // 超过平均人数1.5倍为过热
    underpopulatedRatio: 0.5, // 低于平均人数0.5倍为过冷
    overpopulatedBonus: 10,   // 过热阵营加入奖励
    underpopulatedBonus: 50   // 过冷阵营加入奖励
  }
};
// 战国七雄阵营
const FACTIONS = {
  qin: {
    id: 'qin',
    name: '秦',
    color: '#c0392b',
    trait: '军功爵制',
    traitDesc: '兵力>敌方时，攻击+20%',
    style: '进攻型，滚雪球',
    attackBonusWhenStrong: 0.20
  },
  chu: {
    id: 'chu',
    name: '楚',
    color: '#8e44ad',
    trait: '广袤南疆',
    traitDesc: '城池耐久+30%，首都是其他国的1.5倍',
    style: '防守型，持久战',
    durabilityBonus: 0.30
  },
  qi: {
    id: 'qi',
    name: '齐',
    color: '#2980b9',
    trait: '渔盐之利',
    traitDesc: '免费点赞产兵+25%',
    style: '人海型，免费玩家友好',
    freeOutputBonus: 0.25
  },
  yan: {
    id: 'yan',
    name: '燕',
    color: '#16a085',
    trait: '荆轲刺秦',
    traitDesc: '武将技能伤害+30%',
    style: '技战术型，武将核心',
    skillDamageBonus: 0.30
  },
  zhao: {
    id: 'zhao',
    name: '赵',
    color: '#d35400',
    trait: '胡服骑射',
    traitDesc: '可跨越1格攻击',
    style: '机动型，奇袭',
    canRangedAttack: true
  },
  wei: {
    id: 'wei',
    name: '魏',
    color: '#27ae60',
    trait: '魏武卒',
    traitDesc: '步兵防御+25%，守城时耐久消耗-20%',
    style: '铁壁型，防守反击',
    defenseBonus: 0.25,
    siegeDefenseBonus: 0.20
  },
  han: {
    id: 'han',
    name: '韩',
    color: '#f39c12',
    trait: '劲弩利剑',
    traitDesc: '弓兵攻击+35%，爱的爆炸伤害+50%',
    style: '技能型，礼物爆发',
    archerBonus: 0.35,
    bombDamageBonus: 0.50
  }
};
// 城池定义 (id, name, type, faction, x, y, neighbors)
const CITIES = [
  // 秦国
  { id: 'xianyang', name: '咸阳', type: 'capital', faction: 'qin', x: 120, y: 280, neighbors: ['hangu', 'yong'] },
  { id: 'hangu', name: '函谷关', type: 'fortress', faction: 'qin', x: 200, y: 250, neighbors: ['xianyang', 'yong', 'luoyang'] },
  { id: 'yong', name: '雍城', type: 'normal', faction: 'qin', x: 100, y: 350, neighbors: ['xianyang', 'hangu'] },
  // 楚国
  { id: 'ying', name: '郢都', type: 'capital', faction: 'chu', x: 280, y: 480, neighbors: ['yiling', 'jiangling'] },
  { id: 'yiling', name: '夷陵', type: 'fortress', faction: 'chu', x: 200, y: 450, neighbors: ['ying', 'jiangling', 'yong'] },
  { id: 'jiangling', name: '江陵', type: 'normal', faction: 'chu', x: 350, y: 520, neighbors: ['ying', 'yiling'] },
  // 齐国
  { id: 'linzi', name: '临淄', type: 'capital', faction: 'qi', x: 620, y: 180, neighbors: ['jimo', 'ju'] },
  { id: 'jimo', name: '即墨', type: 'fortress', faction: 'qi', x: 680, y: 250, neighbors: ['linzi', 'ju', 'qufu'] },
  { id: 'ju', name: '莒城', type: 'normal', faction: 'qi', x: 580, y: 250, neighbors: ['linzi', 'jimo'] },
  // 燕国
  { id: 'ji', name: '蓟城', type: 'capital', faction: 'yan', x: 550, y: 60, neighbors: ['zhuoxian', 'shanggu'] },
  { id: 'zhuoxian', name: '涿县', type: 'normal', faction: 'yan', x: 500, y: 120, neighbors: ['ji', 'shanggu', 'handan'] },
  { id: 'shanggu', name: '上谷', type: 'normal', faction: 'yan', x: 620, y: 100, neighbors: ['ji', 'zhuoxian'] },
  // 赵国
  { id: 'handan', name: '邯郸', type: 'capital', faction: 'zhao', x: 450, y: 180, neighbors: ['taiyuan', 'zhuoxian', 'xinzheng'] },
  { id: 'taiyuan', name: '太原', type: 'normal', faction: 'zhao', x: 380, y: 150, neighbors: ['handan', 'linfen'] },
  { id: 'linfen', name: '临汾', type: 'normal', faction: 'zhao', x: 320, y: 220, neighbors: ['taiyuan', 'handan', 'daliang'] },
  // 魏国
  { id: 'daliang', name: '大梁', type: 'capital', faction: 'wei', x: 450, y: 320, neighbors: ['shangqiu', 'xinzheng', 'linfen'] },
  { id: 'shangqiu', name: '商丘', type: 'fortress', faction: 'wei', x: 520, y: 380, neighbors: ['daliang', 'xuzhou', 'qufu'] },
  { id: 'xuzhou', name: '徐州', type: 'normal', faction: 'wei', x: 580, y: 350, neighbors: ['shangqiu', 'jimo'] },
  // 韩国
  { id: 'xinzheng', name: '新郑', type: 'capital', faction: 'han', x: 380, y: 280, neighbors: ['yangdi', 'shangdang', 'daliang', 'luoyang'] },
  { id: 'yangdi', name: '阳翟', type: 'normal', faction: 'han', x: 320, y: 340, neighbors: ['xinzheng', 'yiling', 'shangdang'] },
  { id: 'shangdang', name: '上党', type: 'normal', faction: 'han', x: 350, y: 230, neighbors: ['xinzheng', 'yangdi', 'linfen', 'taiyuan'] },
  // 中立重镇
  { id: 'luoyang', name: '洛阳', type: 'fortress', faction: null, x: 280, y: 300, neighbors: ['hangu', 'xinzheng', 'linfen'] },
  { id: 'qufu', name: '曲阜', type: 'normal', faction: null, x: 550, y: 280, neighbors: ['jimo', 'shangqiu', 'ju'] }
];
// 武将库
const GENERALS = [
  // 橙将
  { id: 'baiqi', name: '白起', quality: 'orange', faction: 'qin',
    passive: '击杀敌方兵力时，己方回复击杀数20%的兵',
    skill: '人屠', skillDesc: '对目标城池造成15000伤害', skillDamage: 15000, skillCooldown: 60 },
  { id: 'wangjian', name: '王翦', quality: 'orange', faction: 'qin',
    passive: '攻城速度+30%',
    skill: '灭楚', skillDesc: '己方全属性+40%，持续20秒', skillBuff: 0.40, skillDuration: 20, skillCooldown: 60 },
  { id: 'limu', name: '李牧', quality: 'orange', faction: 'zhao',
    passive: '守城时防御+50%',
    skill: '破匈奴', skillDesc: '召唤3000骑兵', skillSummon: 3000, skillCooldown: 60 },
  { id: 'sunbin', name: '孙膑', quality: 'orange', faction: 'qi',
    passive: '武将技能冷却-30%',
    skill: '围魏救赵', skillDesc: '敌方正在攻击的城池攻击力归零10秒', skillCooldown: 60 },
  // 紫将
  { id: 'lianpo', name: '廉颇', quality: 'purple', faction: 'zhao',
    passive: '步兵生命+30%',
    skill: '负荆请罪', skillDesc: '己方城池耐久回复10000', skillHeal: 10000, skillCooldown: 60 },
  { id: 'wuqi', name: '吴起', quality: 'purple', faction: 'wei',
    passive: '步兵攻击+30%',
    skill: '魏武卒', skillDesc: '己方步兵瞬间+5000', skillSummon: 5000, skillCooldown: 60 },
  { id: 'leyi', name: '乐毅', quality: 'purple', faction: 'yan',
    passive: '连下城池时，每座+10%攻击（最多50%）',
    skill: '伐齐', skillDesc: '对目标城池造成12000伤害', skillDamage: 12000, skillCooldown: 60 },
  { id: 'hanfei', name: '韩非', quality: 'purple', faction: 'han',
    passive: '弓兵攻击+25%',
    skill: '法度', skillDesc: '下一次爱的爆炸伤害+100%', skillCooldown: 60 },
  // 蓝将
  { id: 'tianji', name: '田忌', quality: 'blue', faction: 'qi',
    passive: '弓兵攻击+20%',
    skill: '赛马', skillDesc: '己方攻速+50%，持续15秒', skillBuff: 0.50, skillDuration: 15, skillCooldown: 60 },
  { id: 'jingke', name: '荆轲', quality: 'blue', faction: 'yan',
    passive: '武将技能伤害+20%',
    skill: '刺秦', skillDesc: '对敌方首都造成5000伤害（无视防御）', skillDamage: 5000, skillCooldown: 60 },
  { id: 'quyuan', name: '屈原', quality: 'blue', faction: 'chu',
    passive: '城池耐久回复速度+100%',
    skill: '离骚', skillDesc: '己方全阵营兵力+3000', skillSummon: 3000, skillCooldown: 60 },
  { id: 'shenbuhai', name: '申不害', quality: 'blue', faction: 'han',
    passive: '技能冷却-20%',
    skill: '术治', skillDesc: '随机敌方武将技能沉默30秒', skillCooldown: 60 }
];
// 礼物定义
const GIFTS = [
  { id: 'light', name: '灯牌', price: 1, effect: 'join', troops: 20, desc: '加入阵营标识，+20步兵' },
  { id: 'wand', name: '仙女棒', price: 10, effect: 'archer', troops: 300, desc: '+300弓兵' },
  { id: 'pill', name: '能力药丸', price: 30, effect: 'buff', buff: 0.40, duration: 30, desc: '阵营攻速+40%，持续30秒' },
  { id: 'mic', name: '派对话筒', price: 52, effect: 'general', quality: 'random', desc: '召唤随机武将（蓝70%/紫30%）' },
  { id: 'bomb', name: '爱的爆炸', price: 99, effect: 'damage', damage: 8000, desc: '对目标城池造成8000点直接伤害' },
  { id: 'mirror', name: '魔法镜', price: 199, effect: 'buff', buff: 0.25, duration: 60, desc: '阵营全属性+25%，持续60秒' },
  { id: 'drop', name: '神秘空投', price: 520, effect: 'general', troops: 8000, quality: 'purple', desc: '+8000步兵 + 随机紫将' },
  { id: 'car', name: '跑车', price: 1200, effect: 'general', quality: 'orange', buff: 0.30, duration: 60, desc: '召唤橙将 + 全阵营+30%属性60秒' },
  { id: 'carnival', name: '嘉年华', price: 30000, effect: 'capture', desc: '直接夺取一座非首都城池（需耐久<50%）' },
  { id: 'sabotage', name: '离间计', price: 299, effect: 'sabotage', desc: '50%概率使目标阵营退出同盟' }
];
// 随机事件
const EVENTS = [
  { id: 'disaster', name: '天灾', desc: '随机某阵营城池耐久-20%' },
  { id: 'harvest', name: '丰收', desc: '随机某阵营兵力+30%' },
  { id: 'mutiny', name: '兵变', desc: '随机某阵营一名武将叛变' },
  { id: 'reform', name: '变法', desc: '随机某阵营全属性+20%，持续5分钟' },
  { id: 'alliance', name: '合纵', desc: '强制触发一次结盟投票' }
];
