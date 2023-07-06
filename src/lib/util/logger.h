#pragma once

#include <iostream>
//#include <QtDebug>

namespace NotAGame {

inline auto &GetLogger() noexcept { return std::cerr; /*qDebug(); */ }

#define LOG(SEVERITY) GetLogger()

using Logger = decltype(GetLogger());

} // namespace NotAGame
