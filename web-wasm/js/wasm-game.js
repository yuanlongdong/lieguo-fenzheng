/**
 * 列国纷争 - WASM 游戏引擎包装器
 * 将 C++ 编译的 WebAssembly 引擎包装为与原 JS 版相同的接口
 */
class Game {
  constructor() {
    this.state = 'idle';
    this.tick = 0;
    this.elapsedTime = 0;
    this.factions = {};
    this.cities = [];
    this.alliances = [];
    this.battleLogs = [];
    this.playerFaction = null;
    this.huFu = 0;
    this.winner = null;
    this.eventListeners = {};
    this._module = null;
    this._engine = null;
    this._loopTimer = null;
    this._gameSpeed = 1;
    this._ready = false;
    this._prevCityOwners = {};
    this._prevWinner = null;
  }

  async initAsync() {
    if (this._ready) return;
    if (typeof createLieGuoModule === 'undefined') {
      throw new Error('WASM 模块未加载，请确认 lieguo.js 已引入');
    }
    this._module = await createLieGuoModule();
    this._engine = new this._module.Game();
    this._ready = true;
    this._syncFromEngine();
    for (const city of this.cities) {
      this._prevCityOwners[city.id] = city.faction;
    }
    this.emit('ready');
  }

  _syncFromEngine() {
    if (!this._engine) return;
    this.factions = this._engine.getFactionsJS();
    this.cities = this._engine.getCitiesJS();
    this.alliances = this._engine.getAlliancesJS();
    this.battleLogs = this._engine.getBattleLogsJS();
    this.elapsedTime = this._engine.getElapsedTime();
    this.huFu = this._engine.getHuFu();
    this.playerFaction = this._engine.getPlayerFaction() || null;
    this.winner = this._engine.getWinner() || null;
    const s = this._engine.getState();
    this.state = s === 0 ? 'idle' : s === 1 ? 'playing' : s === 2 ? 'paused' : 'ended';
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

  init() {}
  start() {
    if (!this._engine) return;
    this._engine.start();
    this._syncFromEngine();
    this.emit('start');
    this._startLoop();
  }
  pause() {
    if (!this._engine) return;
    this._engine.pause();
    this._syncFromEngine();
    this._stopLoop();
    this.emit('pause');
  }
  resume() {
    if (!this._engine) return;
    this._engine.resume();
    this._syncFromEngine();
    this._startLoop();
    this.emit('resume');
  }
  stop() {
    if (!this._engine) return;
    this._engine.stop();
    this._stopLoop();
    this._syncFromEngine();
  }

  _startLoop() {
    this._stopLoop();
    this._loopTimer = setInterval(() => this._loopTick(), 1000 / this._gameSpeed);
  }
  _stopLoop() {
    if (this._loopTimer) { clearInterval(this._loopTimer); this._loopTimer = null; }
  }
  _loopTick() {
    if (!this._engine || this.state !== 'playing') return;
    const prevLogCount = this.battleLogs.length;
    this._engine.tick();
    this.tick++;
    this._syncFromEngine();
    if (this.battleLogs.length > prevLogCount) {
      const newLogs = this.battleLogs.slice(0, this.battleLogs.length - prevLogCount);
      for (let i = newLogs.length - 1; i >= 0; i--) this.emit('log', newLogs[i].msg);
    }
    for (const city of this.cities) {
      const prev = this._prevCityOwners[city.id];
      if (prev !== undefined && prev !== city.faction) {
        this.emit('cityCaptured', { city, factionId: city.faction, oldFactionId: prev });
      }
      this._prevCityOwners[city.id] = city.faction;
    }
    if (this.winner && this._prevWinner !== this.winner) {
      this._prevWinner = this.winner;
      this.emit('victory', this.winner);
    }
    this.emit('update', this.getState());
    if (this.state === 'ended') { this._stopLoop(); this.emit('end', this.winner); }
  }
  setSpeed(speed) {
    this._gameSpeed = parseFloat(speed) || 1;
    if (this.state === 'playing') this._startLoop();
  }

  joinFaction(factionId) {
    if (!this._engine) return false;
    const r = this._engine.joinFaction(factionId);
    this._syncFromEngine();
    if (r) this.emit('join', factionId);
    return r;
  }
  like() {
    if (!this._engine) return false;
    const r = this._engine.like();
    this._syncFromEngine();
    return r;
  }
  comment(text) {
    if (!this._engine) return false;
    const r = this._engine.comment(text);
    this._syncFromEngine();
    if (r) this.emit('log', '弹幕: ' + text);
    return r;
  }
  sendGift(giftId, targetCityId = '') {
    if (!this._engine) return { success: false, msg: '引擎未就绪' };
    const r = this._engine.sendGift(giftId, targetCityId);
    this._syncFromEngine();
    return r;
  }

  createAlliance(members) {
    if (!this._engine) return false;
    const vec = new this._module.VectorString();
    members.forEach(m => vec.push_back(m));
    const r = this._engine.createAlliance(vec);
    vec.delete();
    this._syncFromEngine();
    return r;
  }
  breakAlliance(factionId) {
    if (!this._engine) return false;
    const r = this._engine.breakAlliance(factionId);
    this._syncFromEngine();
    return r;
  }
  isAllied(f1, f2) {
    if (!this._engine) return false;
    return this._engine.isAllied(f1, f2);
  }

  triggerRandomEvent() {
    if (!this._engine) return '';
    const r = this._engine.triggerRandomEvent();
    this._syncFromEngine();
    return r;
  }

  getState() {
    return {
      state: this.state, tick: this.tick, elapsedTime: this.elapsedTime,
      factions: this.factions, cities: this.cities, alliances: this.alliances,
      battleLogs: this.battleLogs, playerFaction: this.playerFaction,
      huFu: this.huFu, winner: this.winner
    };
  }
  formatTime() { return this._engine ? this._engine.formatTime() : '00:00'; }
  addLog(msg) { this.emit('log', msg); }
  updateCityCounts() {}
  checkVictory() {}
}
window.Game = Game;
