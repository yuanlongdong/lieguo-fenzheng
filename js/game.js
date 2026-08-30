/**
 * 列国纷争 - 游戏核心引擎
 * 负责游戏状态管理、主循环、阵营状态
 */
class Game {
  constructor() {
    this.state = 'idle'; // idle, playing, paused, ended
    this.tick = 0;
    this.elapsedTime = 0; // 秒
    this.factions = {};   // 阵营状态
    this.cities = [];     // 城池状态
    this.alliances = [];  // 同盟列表
    this.battleLogs = []; // 战斗日志
    this.playerFaction = null; // 玩家所属阵营
    this.huFu = 0;        // 虎符
    this.winner = null;
    this.eventListeners = {};
    this.init();
  }
  init() {
    // 初始化阵营
    for (const [id, data] of Object.entries(FACTIONS)) {
      this.factions[id] = {
        id,
        name: data.name,
        color: data.color,
        troops: 500,        // 初始兵力
        archers: 0,         // 弓兵
        cavalry: 0,         // 骑兵
        playerCount: 0,     // 玩家数
        generals: [],       // 上阵武将
        buffs: [],          // 当前buff列表
        alive: true,
        citiesOwned: 0,
        contribution: {},   // 玩家贡献
        totalContribution: 0
      };
    }
    // 初始化城池
    this.cities = CITIES.map(c => ({
      ...c,
      durability: CONFIG.cityDurability[c.type],
      maxDurability: CONFIG.cityDurability[c.type],
      underAttack: false,
      attacker: null
    }));
    this.updateCityCounts();
  }
  on(event, callback) {
    if (!this.eventListeners[event]) this.eventListeners[event] = [];
    this.eventListeners[event].push(callback);
  }
  emit(event, data) {
    if (this.eventListeners[event]) {
      this.eventListeners[event].forEach(cb => cb(data));
    }
  }
  start() {
    this.state = 'playing';
    this.tick = 0;
    this.elapsedTime = 0;
    this.addLog('列国纷争开始！七雄并立，逐鹿中原！');
    this.emit('start');
    this.loop();
  }
  pause() {
    this.state = 'paused';
    this.emit('pause');
  }
  resume() {
    if (this.state === 'paused') {
      this.state = 'playing';
      this.loop();
      this.emit('resume');
    }
  }
  loop() {
    if (this.state !== 'playing') return;
    this.update();
    this.tick++;
    this.elapsedTime += CONFIG.tickInterval / 1000 * CONFIG.gameSpeed;
    // 检查胜利条件
    this.checkVictory();
    if (this.state === 'playing') {
      setTimeout(() => this.loop(), CONFIG.tickInterval / CONFIG.gameSpeed);
    }
  }
  update() {
    // 1. 城池产兵
    this.produceTroops();
    // 2. 自动攻城
    this.processSieges();
    // 3. 武将技能冷却
    this.updateGenerals();
    // 4. Buff倒计时
    this.updateBuffs();
    // 5. 同盟倒计时
    this.updateAlliances();
    // 6. 城池耐久自然回复（小幅）
    this.regenerateDurability();
    this.emit('update', this.getState());
  }
  produceTroops() {
    for (const city of this.cities) {
      if (city.faction && this.factions[city.faction]?.alive) {
        const production = CONFIG.cityProduction[city.type];
        this.factions[city.faction].troops += production;
      }
    }
  }
  processSieges() {
    // 每个阵营自动攻击相邻敌方城池
    for (const [factionId, faction] of Object.entries(this.factions)) {
      if (!faction.alive || faction.troops < 100) continue;
      // 找到可攻击的相邻城池
      const ownCities = this.cities.filter(c => c.faction === factionId);
      for (const ownCity of ownCities) {
        for (const neighborId of ownCity.neighbors) {
          const target = this.cities.find(c => c.id === neighborId);
          if (!target || target.faction === factionId) continue;
          if (target.faction && this.isAllied(factionId, target.faction)) continue;
          // 赵国可以跨格攻击（额外检查）
          this.attackCity(factionId, target);
        }
      }
    }
  }
  attackCity(factionId, city) {
    const faction = this.factions[factionId];
    if (!faction || faction.troops <= 0) return;
    const defenseBonus = CONFIG.cityDefense[city.type];
    const attackPower = this.calcAttackPower(factionId);
    const defensePower = this.calcDefensePower(city);
    if (attackPower > defensePower) {
      // 削减耐久
      const decay = (attackPower - defensePower) / CONFIG.durabilityDecayCoeff;
      city.durability = Math.max(0, city.durability - decay);
      city.underAttack = true;
      city.attacker = factionId;
      // 攻城方损耗
      const loss = Math.floor(faction.troops * CONFIG.siegeLossRate);
      faction.troops = Math.max(0, faction.troops - loss);
      // 白起被动：击杀回复
      const baiqi = faction.generals.find(g => g.id === 'baiqi');
      if (baiqi) {
        faction.troops += Math.floor(loss * 0.2);
      }
      // 耐久归零，城池易主
      if (city.durability <= 0) {
        this.captureCity(factionId, city);
      }
    } else {
      city.underAttack = false;
      city.attacker = null;
    }
  }
  calcAttackPower(factionId) {
    const faction = this.factions[factionId];
    const data = FACTIONS[factionId];
    let power = faction.troops + faction.archers * 1.25 + faction.cavalry * 1.1;
    // 阵营特性
    if (data.attackBonusWhenStrong) {
      // 简化：兵力超过5000触发
      if (faction.troops > 5000) power *= (1 + data.attackBonusWhenStrong);
    }
    // 武将被动
    for (const g of faction.generals) {
      if (g.id === 'wuqi') power *= 1.30; // 吴起步兵攻击+30%
      if (g.id === 'wangjian') power *= 1.30; // 王翦攻城速度+30%
      if (g.id === 'leyi') {
        const bonus = Math.min(0.50, faction.citiesOwned * 0.10);
        power *= (1 + bonus);
      }
    }
    // Buff
    for (const buff of faction.buffs) {
      if (buff.type === 'attack') power *= (1 + buff.value);
      if (buff.type === 'all') power *= (1 + buff.value);
    }
    return power;
  }
  calcDefensePower(city) {
    if (!city.faction) return 100; // 中立城少量防御
    const faction = this.factions[city.faction];
    const data = FACTIONS[city.faction];
    let power = (faction.troops / Math.max(1, faction.citiesOwned)) * CONFIG.cityDefense[city.type];
    // 阵营特性
    if (data.defenseBonus) power *= (1 + data.defenseBonus);
    // 武将被动
    for (const g of faction.generals) {
      if (g.id === 'lianpo') power *= 1.30; // 廉颇步兵生命+30%
      if (g.id === 'limu') power *= 1.50; // 李牧守城防御+50%
    }
    // Buff
    for (const buff of faction.buffs) {
      if (buff.type === 'defense') power *= (1 + buff.value);
      if (buff.type === 'all') power *= (1 + buff.value);
    }
    return power;
  }
  captureCity(factionId, city) {
    const oldFaction = city.faction;
    city.faction = factionId;
    city.durability = city.maxDurability * 0.5; // 占领后耐久恢复50%
    city.underAttack = false;
    city.attacker = null;
    this.updateCityCounts();
    this.addLog(`${FACTIONS[factionId].name}国攻占了${city.name}！`);
    this.emit('cityCaptured', { city, factionId, oldFaction });
    // 检查灭国
    if (oldFaction) {
      const oldFactionCities = this.cities.filter(c => c.faction === oldFaction);
      if (oldFactionCities.length === 0) {
        this.eliminateFaction(oldFaction);
      }
    }
  }
  eliminateFaction(factionId) {
    const faction = this.factions[factionId];
    faction.alive = false;
    faction.troops = 0;
    faction.generals = [];
    // 所有城池变中立
    for (const city of this.cities) {
      if (city.faction === factionId) {
        city.faction = null;
        city.durability = city.maxDurability * 0.3;
      }
    }
    this.addLog(`${FACTIONS[factionId].name}国灭亡！遗民可加入其他阵营。`);
    this.emit('factionEliminated', factionId);
  }
  updateGenerals() {
    for (const faction of Object.values(this.factions)) {
      for (const general of faction.generals) {
        if (general.cooldown > 0) {
          general.cooldown -= CONFIG.tickInterval / 1000 * CONFIG.gameSpeed;
          if (general.cooldown <= 0) {
            general.cooldown = 0;
            this.castGeneralSkill(faction.id, general);
          }
        }
      }
    }
  }
  castGeneralSkill(factionId, general) {
    const faction = this.factions[factionId];
    const factionData = FACTIONS[factionId];
    let damageMultiplier = 1;
    if (factionData.skillDamageBonus) damageMultiplier = 1 + factionData.skillDamageBonus;
    // 荆轲被动
    const jingke = faction.generals.find(g => g.id === 'jingke');
    if (jingke && general.id !== 'jingke') damageMultiplier *= 1.2;
    if (general.skillDamage) {
      // 伤害技能：攻击最近的敌方城池
      const target = this.findNearestEnemyCity(factionId);
      if (target) {
        const dmg = general.skillDamage * damageMultiplier;
        target.durability = Math.max(0, target.durability - dmg);
        this.addLog(`${general.name}释放【${general.skill}】，对${target.name}造成${Math.floor(dmg)}伤害！`);
        if (target.durability <= 0) this.captureCity(factionId, target);
      }
    }
    if (general.skillSummon) {
      if (general.id === 'limu') {
        faction.cavalry += general.skillSummon;
      } else {
        faction.troops += general.skillSummon;
      }
      this.addLog(`${general.name}释放【${general.skill}】，获得${general.skillSummon}兵力！`);
    }
    if (general.skillHeal) {
      const ownCities = this.cities.filter(c => c.faction === factionId);
      for (const c of ownCities) {
        c.durability = Math.min(c.maxDurability, c.durability + general.skillHeal / ownCities.length);
      }
      this.addLog(`${general.name}释放【${general.skill}】，城池耐久回复！`);
    }
    if (general.skillBuff) {
      faction.buffs.push({
        type: 'all',
        value: general.skillBuff,
        duration: general.skillDuration || 20,
        source: general.name
      });
      this.addLog(`${general.name}释放【${general.skill}】，全属性提升！`);
    }
    // 重置冷却
    let cooldown = general.skillCooldown || 60;
    // 孙膑被动
    const sunbin = faction.generals.find(g => g.id === 'sunbin');
    if (sunbin && general.id !== 'sunbin') cooldown *= 0.7;
    // 申不害被动
    const shenbuhai = faction.generals.find(g => g.id === 'shenbuhai');
    if (shenbuhai) cooldown *= 0.8;
    general.cooldown = cooldown;
  }
  findNearestEnemyCity(factionId) {
    const ownCities = this.cities.filter(c => c.faction === factionId);
    for (const ownCity of ownCities) {
      for (const nid of ownCity.neighbors) {
        const target = this.cities.find(c => c.id === nid);
        if (target && target.faction !== factionId && target.faction) return target;
      }
    }
    // 找中立城
    for (const ownCity of ownCities) {
      for (const nid of ownCity.neighbors) {
        const target = this.cities.find(c => c.id === nid);
        if (target && target.faction !== factionId) return target;
      }
    }
    return null;
  }
  updateBuffs() {
    for (const faction of Object.values(this.factions)) {
      faction.buffs = faction.buffs.filter(buff => {
        buff.duration -= CONFIG.tickInterval / 1000 * CONFIG.gameSpeed;
        return buff.duration > 0;
      });
    }
  }
  updateAlliances() {
    this.alliances = this.alliances.filter(ally => {
      ally.remaining -= CONFIG.tickInterval / 1000 * CONFIG.gameSpeed;
      if (ally.remaining <= 0) {
        this.addLog(`${ally.members.map(m => FACTIONS[m].name).join('、')}的同盟到期解散！`);
        return false;
      }
      return true;
    });
  }
  regenerateDurability() {
    for (const city of this.cities) {
      if (city.faction && !city.underAttack) {
        let regen = 10;
        // 屈原被动
        const faction = this.factions[city.faction];
        if (faction?.generals.find(g => g.id === 'quyuan')) regen *= 2;
        city.durability = Math.min(city.maxDurability, city.durability + regen);
      }
    }
  }
  isAllied(f1, f2) {
    return this.alliances.some(a => a.members.includes(f1) && a.members.includes(f2));
  }
  createAlliance(members) {
    if (members.length < 2 || members.length > 3) return false;
    // 检查是否已有同盟
    for (const m of members) {
      if (this.alliances.some(a => a.members.includes(m))) return false;
    }
    this.alliances.push({
      members,
      remaining: CONFIG.allianceDuration,
      totalDuration: CONFIG.allianceDuration
    });
    this.addLog(`${members.map(m => FACTIONS[m].name).join('、')}结成合纵同盟！共同抗敌！`);
    this.emit('allianceCreated', members);
    return true;
  }
  breakAlliance(factionId) {
    const ally = this.alliances.find(a => a.members.includes(factionId));
    if (!ally) return false;
    ally.members = ally.members.filter(m => m !== factionId);
    if (ally.members.length < 2) {
      this.alliances = this.alliances.filter(a => a !== ally);
    }
    // 背盟惩罚
    this.factions[factionId].buffs.push({
      type: 'all', value: -0.20, duration: 300, source: '背盟惩罚'
    });
    this.addLog(`${FACTIONS[factionId].name}国背盟！全属性-20%持续5分钟。`);
    this.emit('allianceBroken', factionId);
    return true;
  }
  updateCityCounts() {
    for (const faction of Object.values(this.factions)) {
      faction.citiesOwned = this.cities.filter(c => c.faction === faction.id).length;
    }
  }
  checkVictory() {
    const aliveFactions = Object.values(this.factions).filter(f => f.alive);
    if (aliveFactions.length === 1) {
      this.state = 'ended';
      this.winner = aliveFactions[0].id;
      this.addLog(`${aliveFactions[0].name}国一统天下！游戏结束！`);
      this.emit('victory', this.winner);
    }
  }
  // 玩家操作：加入阵营
  joinFaction(factionId) {
    if (!this.factions[factionId]?.alive) return false;
    this.playerFaction = factionId;
    this.factions[factionId].playerCount++;
    // 人数平衡奖励
    const avgPlayers = Object.values(this.factions).reduce((s, f) => s + f.playerCount, 0) / 7;
    let bonus = CONFIG.freeOutput.join;
    if (this.factions[factionId].playerCount > avgPlayers * CONFIG.balance.overpopulatedRatio) {
      bonus = CONFIG.balance.overpopulatedBonus;
    } else if (this.factions[factionId].playerCount < avgPlayers * CONFIG.balance.underpopulatedRatio) {
      bonus = CONFIG.balance.underpopulatedBonus;
    }
    this.factions[factionId].troops += bonus;
    this.addLog(`你加入了${FACTIONS[factionId].name}国，获得${bonus}兵力！`);
    this.emit('playerJoined', factionId);
    return true;
  }
  // 玩家操作：点赞
  like() {
    if (!this.playerFaction) return false;
    const faction = this.factions[this.playerFaction];
    const data = FACTIONS[this.playerFaction];
    let amount = CONFIG.freeOutput.like;
    if (data.freeOutputBonus) amount *= (1 + data.freeOutputBonus);
    faction.troops += amount;
    this.addContribution(this.playerFaction, 'player', amount);
    return true;
  }
  // 玩家操作：评论
  comment(text) {
    if (!this.playerFaction) return false;
    const faction = this.factions[this.playerFaction];
    faction.troops += CONFIG.freeOutput.comment;
    this.addContribution(this.playerFaction, 'player', CONFIG.freeOutput.comment);
    this.addLog(`弹幕: ${text}`);
    return true;
  }
  // 玩家操作：送礼物
  sendGift(giftId, targetCityId = null) {
    if (!this.playerFaction) return { success: false, msg: '请先加入阵营' };
    const gift = GIFTS.find(g => g.id === giftId);
    if (!gift) return { success: false, msg: '礼物不存在' };
    const faction = this.factions[this.playerFaction];
    const factionData = FACTIONS[this.playerFaction];
    switch (gift.effect) {
      case 'join':
        faction.troops += gift.troops;
        this.addLog(`送出${gift.name}，+${gift.troops}兵力`);
        break;
      case 'archer':
        faction.archers += gift.troops;
        this.addLog(`送出${gift.name}，+${gift.troops}弓兵`);
        break;
      case 'buff':
        faction.buffs.push({
          type: gift.id === 'mirror' ? 'all' : 'attack',
          value: gift.buff,
          duration: gift.duration,
          source: gift.name
        });
        this.addLog(`送出${gift.name}，阵营获得增益！`);
        break;
      case 'general':
        this.summonGeneral(this.playerFaction, gift.quality);
        if (gift.troops) faction.troops += gift.troops;
        if (gift.buff) {
          faction.buffs.push({ type: 'all', value: gift.buff, duration: gift.duration, source: gift.name });
        }
        break;
      case 'damage':
        if (!targetCityId) {
          const target = this.findNearestEnemyCity(this.playerFaction);
          if (target) targetCityId = target.id;
        }
        if (targetCityId) {
          const city = this.cities.find(c => c.id === targetCityId);
          if (city) {
            let dmg = gift.damage;
            if (factionData.bombDamageBonus) dmg *= (1 + factionData.bombDamageBonus);
            // 韩非被动
            if (faction.generals.find(g => g.id === 'hanfei') && faction.nextBombDouble) {
              dmg *= 2;
              faction.nextBombDouble = false;
            }
            city.durability = Math.max(0, city.durability - dmg);
            this.addLog(`送出${gift.name}，对${city.name}造成${Math.floor(dmg)}伤害！`);
            if (city.durability <= 0) this.captureCity(this.playerFaction, city);
          }
        }
        break;
      case 'capture':
        if (targetCityId) {
          const city = this.cities.find(c => c.id === targetCityId);
          if (city && city.type !== 'capital' && city.durability < city.maxDurability * 0.5) {
            this.captureCity(this.playerFaction, city);
            this.addLog(`嘉年华！直接夺取${city.name}！`);
          } else {
            return { success: false, msg: '只能夺取耐久低于50%的非首都城池' };
          }
        }
        break;
      case 'sabotage':
        // 离间计：随机选一个有同盟的敌方阵营
        const alliedEnemies = Object.values(this.factions).filter(f =>
          f.alive && f.id !== this.playerFaction && this.alliances.some(a => a.members.includes(f.id))
        );
        if (alliedEnemies.length > 0 && Math.random() < 0.5) {
          const target = alliedEnemies[Math.floor(Math.random() * alliedEnemies.length)];
          this.breakAlliance(target.id);
          faction.troops += Math.floor(target.troops * 0.10);
          this.addLog(`离间计成功！${target.name}国退出同盟，获得其10%兵力！`);
        } else {
          this.addLog(`离间计失败...`);
        }
        break;
    }
    this.addContribution(this.playerFaction, 'player', gift.price * 10);
    this.huFu += Math.floor(gift.price / 100);
    this.emit('giftSent', { gift, factionId: this.playerFaction });
    return { success: true };
  }
  summonGeneral(factionId, quality) {
    const faction = this.factions[factionId];
    let pool;
    if (quality === 'orange') {
      pool = GENERALS.filter(g => g.quality === 'orange');
    } else if (quality === 'purple') {
      pool = GENERALS.filter(g => g.quality === 'purple' || g.quality === 'orange');
    } else {
      // random: blue 70%, purple 30%
      pool = Math.random() < 0.7
        ? GENERALS.filter(g => g.quality === 'blue')
        : GENERALS.filter(g => g.quality === 'purple');
    }
    const general = pool[Math.floor(Math.random() * pool.length)];
    // 检查是否已有，有则升级
    const existing = faction.generals.find(g => g.id === general.id);
    if (existing) {
      existing.level = Math.min(3, (existing.level || 1) + 1);
      this.addLog(`${general.name}升级到${existing.level}级！`);
    } else {
      if (faction.generals.length >= CONFIG.maxGenerals) {
        // 替换最弱的
        faction.generals.sort((a, b) => {
          const order = { blue: 1, purple: 2, orange: 3 };
          return order[a.quality] - order[b.quality];
        });
        faction.generals.shift();
      }
      faction.generals.push({ ...general, level: 1, cooldown: 30 });
      this.addLog(`${FACTIONS[factionId].name}国召唤出${general.name}！`);
    }
    this.emit('generalSummoned', { factionId, general });
  }
  addContribution(factionId, playerId, amount) {
    const faction = this.factions[factionId];
    if (!faction.contribution[playerId]) faction.contribution[playerId] = 0;
    faction.contribution[playerId] += amount;
    faction.totalContribution += amount;
  }
  addLog(msg) {
    this.battleLogs.unshift({
      time: this.elapsedTime,
      msg
    });
    if (this.battleLogs.length > 50) this.battleLogs.pop();
    this.emit('log', msg);
  }
  getState() {
    return {
      state: this.state,
      tick: this.tick,
      elapsedTime: this.elapsedTime,
      factions: this.factions,
      cities: this.cities,
      alliances: this.alliances,
      battleLogs: this.battleLogs,
      playerFaction: this.playerFaction,
      huFu: this.huFu,
      winner: this.winner
    };
  }
  triggerRandomEvent() {
    const event = EVENTS[Math.floor(Math.random() * EVENTS.length)];
    const aliveFactions = Object.values(this.factions).filter(f => f.alive);
    const target = aliveFactions[Math.floor(Math.random() * aliveFactions.length)];
    switch (event.id) {
      case 'disaster':
        const targetCity = this.cities.find(c => c.faction === target.id);
        if (targetCity) {
          targetCity.durability = Math.floor(targetCity.durability * 0.8);
          this.addLog(`【天灾】${target.name}国的${targetCity.name}耐久-20%！`);
        }
        break;
      case 'harvest':
        target.troops = Math.floor(target.troops * 1.3);
        this.addLog(`【丰收】${target.name}国兵力+30%！`);
        break;
      case 'mutiny':
        if (target.generals.length > 0) {
          const g = target.generals.pop();
          this.addLog(`【兵变】${target.name}国的${g.name}叛变！`);
        }
        break;
      case 'reform':
        target.buffs.push({ type: 'all', value: 0.20, duration: 300, source: '变法' });
        this.addLog(`【变法】${target.name}国全属性+20%，持续5分钟！`);
        break;
      case 'alliance':
        this.addLog(`【合纵】天下大势，合纵连横！`);
        break;
    }
    this.emit('eventTriggered', event);
    return event;
  }
}
// 全局游戏实例
let game = null;
function initGame() {
  game = new Game();
  return game;
}
