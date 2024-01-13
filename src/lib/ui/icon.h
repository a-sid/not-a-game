#pragma once

#include <any>
#include <filesystem>

namespace NotAGame {

struct Icon {
  Icon() noexcept = default;
  Icon(const std::filesystem::path &Path) noexcept;
  std::any Data;
};

} // namespace NotAGame
