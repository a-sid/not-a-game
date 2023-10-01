#include "engine/engine.h"

namespace NotAGame {

Engine::Engine(Mod &M, GlobalMap &Map) noexcept : State_{PrepareGameState{M, Map}} {}

Status Engine::StartGame() noexcept {
  // Are the slots filled?
  Player AI{.Name = "Neutral", .PlayerColor = NotAGame::ColorByTurnOrder(0)};
  return Status::Success();
}

} // namespace NotAGame
