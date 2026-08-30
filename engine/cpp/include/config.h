#pragma once
#include "types.h"
#include <vector>

namespace lieguo {

// 游戏全局配置
extern GameConfig CONFIG;

// 战国七雄阵营模板（用于初始化）
std::vector<Faction> createFactions();

// 城池定义
std::vector<City> createCities();

// 武将库
std::vector<General> getGeneralPool();

// 礼物定义
std::vector<Gift> getGifts();

// 工具函数
std::string cityTypeToString(CityType type);
std::string qualityToString(GeneralQuality q);
GeneralQuality stringToQuality(const std::string& s);

} // namespace lieguo
