/**
 * 列国纷争 - UI 管理
 * 负责侧边栏、阵营状态、礼物面板、战斗日志、结算界面
 */

class UIManager {
  constructor(game, mapRenderer) {
    this.game = game;
    this.map = mapRenderer;
    this.selectedTargetCity = null;
    this.initElements();
    this.bindEvents();
    this.bindGameEvents();
  }

  initElements() {
    this.el = {
      factionList: document.getElementById('faction-list'),
      giftPanel: document.getElementById('gift-panel'),
      battleLog: document.getElementById('battle-log'),
      gameInfo: document.getElementById('game-info'),
      playerInfo: document.getElementById('player-info'),
      joinPanel: document.getElementById('join-panel'),
      controlPanel: document.getElementById('control-panel'),
      startBtn: document.getElementById('start-btn'),
      pauseBtn: document.getElementById('pause-btn'),
      eventBtn: document.getElementById('event-btn'),
      speedSelect: document.getElementById('speed-select'),
      targetCityName: document.getElementById('target-city-name'),
      alliancePanel: document.getElementById('alliance-panel'),
      generalPanel: document.getElementById('general-panel'),
      endScreen: document.getElementById('end-screen'),
      winnerText: document.getElementById('winner-text'),
      restartBtn: document.getElementById('restart-btn')
    };
  }

  bindEvents() {
    const safe = (el, evt, cb) => { if (el) el.addEventListener(evt, cb); };

    safe(this.el.startBtn, 'click', () => {
      if (this.game.state === 'idle') {
        this.game.start();
        if (this.el.startBtn) { this.el.startBtn.textContent = '游戏中...'; this.el.startBtn.disabled = true; }
      }
    });
    safe(this.el.restartBtn, 'click', () => location.reload());

    // 地图点击选目标
    this.game.on('citySelected', (city) => {
      this.selectedTargetCity = city;
      if (this.el.targetCityName) this.el.targetCityName.textContent = city.name;
    });
  }

  bindGameEvents() {
    this.game.on('update', () => this.update());
    this.game.on('log', (msg) => this.addLog(msg));
    this.game.on('cityCaptured', ({ city, factionId }) => {
      this.map.addEffect('capture', city.x, city.y, { color: FACTIONS[factionId].color, duration: 2000 });
    });
    this.game.on('victory', (winner) => this.showEndScreen(winner));
  }

  update() {
    this.updateFactionList();
    this.updateGameInfo();
    this.updatePlayerInfo();
    this.updateAlliancePanel();
    this.updateGeneralPanel();
    this.map.render();
  }

  updateFactionList() {
    if (!this.el.factionList) return;
    const html = Object.values(this.game.factions).map(f => {
      const data = FACTIONS[f.id];
      const isPlayer = f.id === this.game.playerFaction;
      const alive = f.alive;
      return `
        <div class="faction-item ${isPlayer ? 'player-faction' : ''} ${!alive ? 'dead-faction' : ''}" style="border-left: 4px solid ${data.color}">
          <div class="faction-header">
            <span class="faction-name" style="color: ${data.color}">${data.name}国</span>
            <span class="faction-cities">${f.citiesOwned}城</span>
          </div>
          <div class="faction-stats">
            <span>兵: ${Math.floor(f.troops)}</span>
            <span>弓: ${f.archers}</span>
            <span>骑: ${f.cavalry}</span>
          </div>
          <div class="faction-trait">${data.trait}</div>
          ${f.buffs.length > 0 ? `<div class="faction-buffs">${f.buffs.map(b => `<span class="buff-tag">${b.source}</span>`).join('')}</div>` : ''}
          ${!alive ? '<div class="dead-tag">已灭亡</div>' : ''}
        </div>
      `;
    }).join('');
    this.el.factionList.innerHTML = html;
  }

  updateGameInfo() {
    if (!this.el.gameInfo) return;
    const minutes = Math.floor(this.game.elapsedTime / 60);
    const seconds = Math.floor(this.game.elapsedTime % 60);
    const aliveCount = Object.values(this.game.factions).filter(f => f.alive).length;
    this.el.gameInfo.innerHTML = `
      <div class="info-row"><span>时间:</span> <span>${minutes}:${seconds.toString().padStart(2, '0')}</span></div>
      <div class="info-row"><span>尚存国家:</span> <span>${aliveCount}/7</span></div>
      <div class="info-row"><span>游戏速度:</span> <span>${CONFIG.gameSpeed}x</span></div>
      <div class="info-row"><span>合纵解锁:</span> <span>${this.game.elapsedTime >= CONFIG.allianceUnlockTime ? '已解锁' : Math.ceil((CONFIG.allianceUnlockTime - this.game.elapsedTime) / 60) + '分钟后'}</span></div>
    `;
  }

  updatePlayerInfo() {
    if (!this.el.playerInfo) return;
    if (!this.game.playerFaction) {
      this.el.playerInfo.innerHTML = '<div class="info-empty">未加入阵营</div>';
      return;
    }
    const faction = this.game.factions[this.game.playerFaction];
    const data = FACTIONS[this.game.playerFaction];
    this.el.playerInfo.innerHTML = `
      <div class="info-row"><span>阵营:</span> <span style="color:${data.color}">${data.name}国</span></div>
      <div class="info-row"><span>虎符:</span> <span style="color:#f1c40f">${this.game.huFu}</span></div>
      <div class="info-row"><span>贡献:</span> <span>${faction.contribution['player'] || 0}</span></div>
      <div class="info-row"><span>目标城池:</span> <span>${this.selectedTargetCity ? this.selectedTargetCity.name : '点击地图选择'}</span></div>
    `;
  }

  updateAlliancePanel() {
    if (!this.el.alliancePanel) return;
    if (this.game.alliances.length === 0) {
      this.el.alliancePanel.innerHTML = '<div class="info-empty">当前无同盟</div>';
      return;
    }
    this.el.alliancePanel.innerHTML = this.game.alliances.map((a, i) => `
      <div class="alliance-item">
        <span>同盟${i + 1}: ${a.members.map(m => FACTIONS[m].name).join(' + ')}</span>
        <span class="alliance-time">剩余${Math.ceil(a.remaining / 60)}分钟</span>
      </div>
    `).join('');
  }

  updateGeneralPanel() {
    if (!this.el.generalPanel) return;
    if (!this.game.playerFaction) {
      this.el.generalPanel.innerHTML = '<div class="info-empty">加入阵营后可召唤武将</div>';
      return;
    }
    const faction = this.game.factions[this.game.playerFaction];
    if (faction.generals.length === 0) {
      this.el.generalPanel.innerHTML = '<div class="info-empty">尚无武将，送派对话筒召唤</div>';
      return;
    }
    this.el.generalPanel.innerHTML = faction.generals.map(g => {
      const qualityColor = g.quality === 'orange' ? '#f39c12' : g.quality === 'purple' ? '#9b59b6' : '#3498db';
      return `
        <div class="general-item" style="border-left: 3px solid ${qualityColor}">
          <div class="general-name">${g.name} <span class="general-level">Lv.${g.level || 1}</span></div>
          <div class="general-skill">【${g.skill}】${g.skillDesc}</div>
          <div class="general-cd">冷却: ${g.cooldown > 0 ? Math.ceil(g.cooldown) + '秒' : '就绪'}</div>
        </div>
      `;
    }).join('');
  }

  addLog(msg) {
    if (!this.el.battleLog) return;
    const div = document.createElement('div');
    div.className = 'log-item';
    const minutes = Math.floor(this.game.elapsedTime / 60);
    const seconds = Math.floor(this.game.elapsedTime % 60);
    div.innerHTML = `<span class="log-time">[${minutes}:${seconds.toString().padStart(2, '0')}]</span> ${msg}`;
    this.el.battleLog.insertBefore(div, this.el.battleLog.firstChild);
    while (this.el.battleLog.children.length > 30) {
      this.el.battleLog.removeChild(this.el.battleLog.lastChild);
    }
  }

  flashMessage(msg) {
    const flash = document.createElement('div');
    flash.className = 'flash-message';
    flash.textContent = msg;
    document.body.appendChild(flash);
    setTimeout(() => flash.remove(), 2000);
  }

  showEndScreen(winner) {
    if (this.el.endScreen) {
      this.el.endScreen.style.display = 'flex';
      if (this.el.winnerText) {
        this.el.winnerText.textContent = `${FACTIONS[winner].name}国一统天下！`;
        this.el.winnerText.style.color = FACTIONS[winner].color;
      }
    }
  }
}
