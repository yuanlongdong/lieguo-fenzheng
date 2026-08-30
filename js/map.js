/**
 * 列国纷争 - 地图渲染
 * Canvas 绘制战国地图、城池、兵力、战斗特效
 */
class MapRenderer {
  constructor(canvas, game) {
    this.canvas = canvas;
    this.ctx = canvas.getContext('2d');
    this.game = game;
    this.width = canvas.width;
    this.height = canvas.height;
    this.hoveredCity = null;
    this.selectedCity = null;
    this.effects = []; // 特效列表
    this.setupEvents();
  }
  setupEvents() {
    this.canvas.addEventListener('mousemove', (e) => {
      const rect = this.canvas.getBoundingClientRect();
      const x = (e.clientX - rect.left) * (this.canvas.width / rect.width);
      const y = (e.clientY - rect.top) * (this.canvas.height / rect.height);
      this.hoveredCity = this.findCityAt(x, y);
      this.canvas.style.cursor = this.hoveredCity ? 'pointer' : 'default';
    });
    this.canvas.addEventListener('click', (e) => {
      const rect = this.canvas.getBoundingClientRect();
      const x = (e.clientX - rect.left) * (this.canvas.width / rect.width);
      const y = (e.clientY - rect.top) * (this.canvas.height / rect.height);
      const city = this.findCityAt(x, y);
      if (city) {
        this.selectedCity = city;
        this.game.emit('citySelected', city);
      }
    });
  }
  findCityAt(x, y) {
    for (const city of this.game.cities) {
      const dx = x - city.x;
      const dy = y - city.y;
      if (Math.sqrt(dx * dx + dy * dy) < 25) return city;
    }
    return null;
  }
  render() {
    const ctx = this.ctx;
    ctx.clearRect(0, 0, this.width, this.height);
    // 背景
    this.drawBackground();
    // 道路连接
    this.drawRoads();
    // 城池
    for (const city of this.game.cities) {
      this.drawCity(city);
    }
    // 战斗特效
    this.drawEffects();
    // 悬停提示
    if (this.hoveredCity) {
      this.drawTooltip(this.hoveredCity);
    }
    // 选中框
    if (this.selectedCity) {
      ctx.strokeStyle = '#fff';
      ctx.lineWidth = 2;
      ctx.setLineDash([5, 5]);
      ctx.beginPath();
      ctx.arc(this.selectedCity.x, this.selectedCity.y, 30, 0, Math.PI * 2);
      ctx.stroke();
      ctx.setLineDash([]);
    }
  }
  drawBackground() {
    const ctx = this.ctx;
    // 渐变背景 - 古地图风格
    const gradient = ctx.createLinearGradient(0, 0, 0, this.height);
    gradient.addColorStop(0, '#2c1810');
    gradient.addColorStop(0.5, '#3d2817');
    gradient.addColorStop(1, '#2c1810');
    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, this.width, this.height);
    // 网格纹理
    ctx.strokeStyle = 'rgba(139, 90, 43, 0.1)';
    ctx.lineWidth = 1;
    for (let x = 0; x < this.width; x += 40) {
      ctx.beginPath();
      ctx.moveTo(x, 0);
      ctx.lineTo(x, this.height);
      ctx.stroke();
    }
    for (let y = 0; y < this.height; y += 40) {
      ctx.beginPath();
      ctx.moveTo(0, y);
      ctx.lineTo(this.width, y);
      ctx.stroke();
    }
    // 标题
    ctx.fillStyle = 'rgba(218, 165, 32, 0.3)';
    ctx.font = 'bold 28px serif';
    ctx.textAlign = 'center';
    ctx.fillText('列 国 纷 争', this.width / 2, 35);
  }
  drawRoads() {
    const ctx = this.ctx;
    const drawn = new Set();
    for (const city of this.game.cities) {
      for (const neighborId of city.neighbors) {
        const key = [city.id, neighborId].sort().join('-');
        if (drawn.has(key)) continue;
        drawn.add(key);
        const neighbor = this.game.cities.find(c => c.id === neighborId);
        if (!neighbor) continue;
        // 同阵营道路高亮
        const sameFaction = city.faction && city.faction === neighbor.faction;
        const allied = city.faction && neighbor.faction &&
          this.game.isAllied(city.faction, neighbor.faction);
        ctx.beginPath();
        ctx.moveTo(city.x, city.y);
        ctx.lineTo(neighbor.x, neighbor.y);
        if (sameFaction) {
          ctx.strokeStyle = FACTIONS[city.faction].color;
          ctx.lineWidth = 3;
          ctx.globalAlpha = 0.6;
        } else if (allied) {
          ctx.strokeStyle = '#f1c40f';
          ctx.lineWidth = 2;
          ctx.setLineDash([8, 4]);
          ctx.globalAlpha = 0.7;
        } else {
          ctx.strokeStyle = 'rgba(139, 90, 43, 0.4)';
          ctx.lineWidth = 1.5;
          ctx.globalAlpha = 1;
        }
        ctx.stroke();
        ctx.setLineDash([]);
        ctx.globalAlpha = 1;
      }
    }
  }
  drawCity(city) {
    const ctx = this.ctx;
    const faction = city.faction ? FACTIONS[city.faction] : null;
    const color = faction ? faction.color : '#7f8c8d';
    const isCapital = city.type === 'capital';
    const isFortress = city.type === 'fortress';
    // 城池底座
    ctx.beginPath();
    if (isCapital) {
      // 首都：星形
      this.drawStar(city.x, city.y, 5, 22, 12);
    } else if (isFortress) {
      // 重镇：方形
      ctx.rect(city.x - 16, city.y - 16, 32, 32);
    } else {
      // 普通城：圆形
      ctx.arc(city.x, city.y, 16, 0, Math.PI * 2);
    }
    ctx.fillStyle = color;
    ctx.fill();
    ctx.strokeStyle = '#f5deb3';
    ctx.lineWidth = 2;
    ctx.stroke();
    // 耐久条
    const barWidth = 40;
    const barHeight = 5;
    const barX = city.x - barWidth / 2;
    const barY = city.y - 30;
    const durabilityRatio = city.durability / city.maxDurability;
    ctx.fillStyle = 'rgba(0,0,0,0.5)';
    ctx.fillRect(barX, barY, barWidth, barHeight);
    // 耐久颜色：绿->黄->红
    let durColor = '#2ecc71';
    if (durabilityRatio < 0.3) durColor = '#e74c3c';
    else if (durabilityRatio < 0.6) durColor = '#f39c12';
    ctx.fillStyle = durColor;
    ctx.fillRect(barX, barY, barWidth * durabilityRatio, barHeight);
    // 攻击中特效
    if (city.underAttack) {
      ctx.strokeStyle = '#e74c3c';
      ctx.lineWidth = 2;
      ctx.beginPath();
      ctx.arc(city.x, city.y, 24 + Math.sin(Date.now() / 100) * 3, 0, Math.PI * 2);
      ctx.stroke();
      // 攻击方标识
      if (city.attacker) {
        ctx.fillStyle = FACTIONS[city.attacker].color;
        ctx.font = 'bold 10px sans-serif';
        ctx.textAlign = 'center';
        ctx.fillText('⚔', city.x, city.y - 38);
      }
    }
    // 城池名称
    ctx.fillStyle = '#f5deb3';
    ctx.font = isCapital ? 'bold 13px serif' : '11px serif';
    ctx.textAlign = 'center';
    ctx.fillText(city.name, city.x, city.y + 32);
    // 中立标识
    if (!city.faction) {
      ctx.fillStyle = '#bdc3c7';
      ctx.font = '9px sans-serif';
      ctx.fillText('(中立)', city.x, city.y + 44);
    }
  }
  drawStar(cx, cy, spikes, outerRadius, innerRadius) {
    const ctx = this.ctx;
    let rot = Math.PI / 2 * 3;
    let x = cx;
    let y = cy;
    const step = Math.PI / spikes;
    ctx.beginPath();
    ctx.moveTo(cx, cy - outerRadius);
    for (let i = 0; i < spikes; i++) {
      x = cx + Math.cos(rot) * outerRadius;
      y = cy + Math.sin(rot) * outerRadius;
      ctx.lineTo(x, y);
      rot += step;
      x = cx + Math.cos(rot) * innerRadius;
      y = cy + Math.sin(rot) * innerRadius;
      ctx.lineTo(x, y);
      rot += step;
    }
    ctx.lineTo(cx, cy - outerRadius);
    ctx.closePath();
  }
  drawEffects() {
    const ctx = this.ctx;
    const now = Date.now();
    this.effects = this.effects.filter(effect => {
      const elapsed = now - effect.start;
      if (elapsed > effect.duration) return false;
      const progress = elapsed / effect.duration;
      if (effect.type === 'gift') {
        // 礼物特效：上升的文字
        ctx.globalAlpha = 1 - progress;
        ctx.fillStyle = effect.color || '#f1c40f';
        ctx.font = 'bold 16px sans-serif';
        ctx.textAlign = 'center';
        ctx.fillText(effect.text, effect.x, effect.y - progress * 50);
        ctx.globalAlpha = 1;
      } else if (effect.type === 'damage') {
        // 伤害数字
        ctx.globalAlpha = 1 - progress;
        ctx.fillStyle = '#e74c3c';
        ctx.font = 'bold 18px sans-serif';
        ctx.textAlign = 'center';
        ctx.fillText(`-${effect.value}`, effect.x, effect.y - progress * 40);
        ctx.globalAlpha = 1;
      } else if (effect.type === 'capture') {
        // 占领特效：扩散圆环
        ctx.globalAlpha = 1 - progress;
        ctx.strokeStyle = effect.color;
        ctx.lineWidth = 3;
        ctx.beginPath();
        ctx.arc(effect.x, effect.y, 20 + progress * 40, 0, Math.PI * 2);
        ctx.stroke();
        ctx.globalAlpha = 1;
      }
      return true;
    });
  }
  addEffect(type, x, y, data = {}) {
    this.effects.push({
      type, x, y,
      start: Date.now(),
      duration: data.duration || 1500,
      ...data
    });
  }
  drawTooltip(city) {
    const ctx = this.ctx;
    const faction = city.faction ? FACTIONS[city.faction] : null;
    const lines = [
      `${city.name} (${city.type === 'capital' ? '首都' : city.type === 'fortress' ? '重镇' : '城池'})`,
      `归属: ${faction ? faction.name + '国' : '中立'}`,
      `耐久: ${Math.floor(city.durability)} / ${city.maxDurability}`,
      `防御: ${CONFIG.cityDefense[city.type]}x`,
      `产兵: ${CONFIG.cityProduction[city.type]}/秒`
    ];
    if (city.underAttack && city.attacker) {
      lines.push(`正在被${FACTIONS[city.attacker].name}国攻击！`);
    }
    const padding = 8;
    const lineHeight = 18;
    const boxWidth = 180;
    const boxHeight = lines.length * lineHeight + padding * 2;
    let tx = city.x + 30;
    let ty = city.y - boxHeight / 2;
    if (tx + boxWidth > this.width) tx = city.x - boxWidth - 30;
    if (ty < 0) ty = 0;
    if (ty + boxHeight > this.height) ty = this.height - boxHeight;
    ctx.fillStyle = 'rgba(0, 0, 0, 0.85)';
    ctx.fillRect(tx, ty, boxWidth, boxHeight);
    ctx.strokeStyle = faction ? faction.color : '#7f8c8d';
    ctx.lineWidth = 1;
    ctx.strokeRect(tx, ty, boxWidth, boxHeight);
    ctx.fillStyle = '#f5deb3';
    ctx.font = '12px sans-serif';
    ctx.textAlign = 'left';
    lines.forEach((line, i) => {
      ctx.fillText(line, tx + padding, ty + padding + (i + 1) * lineHeight - 4);
    });
  }
}
