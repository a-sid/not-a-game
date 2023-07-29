#pragma once

namespace NotAGame::Utils {

template <typename Map> auto *MapFindPtr(const Map &M, const typename Map::key_type &key) noexcept {
  const auto Found = M.find(key);
  return Found == M.end() ? nullptr : &Found->second;
}

} // namespace NotAGame::Utils
