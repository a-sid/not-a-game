#pragma once

#include <any>
#include <filesystem>

#include "util/id.h"

namespace NotAGame {

struct IconSet;

enum class UnitDirection { LookRight, LookLeft };

class Icon {
public:
  Icon() noexcept = default;
  Icon(const std::filesystem::path &Path) noexcept;

  const std::any &GetOpaqueData() const noexcept { return Data_; }

  static std::pair<Icon, Icon> LoadNormalAndMirrored(const std::filesystem::path &Path) noexcept;

private:
  friend struct NotAGame::IconSet;
  std::any Data_;
};

struct IconSet {
  Id<Icon> LookRight;
  Id<Icon> LookLeft;
};

} // namespace NotAGame
