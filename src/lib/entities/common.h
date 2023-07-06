#pragma once

#include "util/id.h"
#include "util/types.h"

#include <vector>

namespace NotAGame {

class Named {
public:
  Named(std::string Name, std::string Title, std::string Description)
      : Name_{std::move(Name)}, Title_{std::move(Title)}, Description_{std::move(Description)} {}

  const std::string &GetName() const noexcept { return Name_; }
  const std::string &GetTitle() const noexcept { return Title_; }
  const std::string &GetDescription() const noexcept { return Description_; }

private:
  std::string Name_;
  std::string Title_;
  std::string Description_;
};

} // namespace NotAGame
