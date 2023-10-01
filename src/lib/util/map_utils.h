#pragma once

namespace NotAGame::Utils {

template <typename Map>
const auto *MapFindPtr(const Map &M, const typename Map::key_type &key) noexcept {
  const auto Found = M.find(key);
  return Found == M.end() ? nullptr : &Found->second;
}

template <typename Map> auto *MapFindPtr(Map &M, const typename Map::key_type &key) noexcept {
  const auto Found = M.find(key);
  return Found == M.end() ? nullptr : &Found->second;
}

} // namespace NotAGame::Utils
