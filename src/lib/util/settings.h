#pragma once

#include "src/lib/util/logger.h"

#include <filesystem>

namespace NotAGame::Utils {

inline std::filesystem::path GetGameDir() noexcept {
  return "/home/aleksei/proj/not-a-game"; // FIXME hardcode
}

inline std::filesystem::path GetDataDir() noexcept { return GetGameDir() / "data"; }
inline std::filesystem::path GetModsDir() noexcept { return GetDataDir() / "mods"; }
inline std::filesystem::path GetDefaultModDir() noexcept { return GetModsDir() / "basic"; }

inline const std::string DEFAULT_LANG = "ru";

} // namespace NotAGame::Utils
