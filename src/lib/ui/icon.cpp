#include "ui/icon.h"

#include <QPixmap>

namespace NotAGame {

Icon::Icon(const std::filesystem::path &Path) noexcept {
  Data_.emplace<QPixmap>(QString::fromStdString(Path.string()));
}

std::pair<Icon, Icon> Icon::LoadNormalAndMirrored(const std::filesystem::path &Path) noexcept {
  QPixmap NormalPixmap{QString::fromStdString(Path.string())};
  QPixmap MirroredPixmap{NormalPixmap.transformed(QTransform{}.scale(-1, 1))};

  Icon Normal, Mirrored;
  Normal.Data_.emplace<QPixmap>(NormalPixmap);
  Mirrored.Data_.emplace<QPixmap>(MirroredPixmap);

  return {Normal, Mirrored};
}

} // namespace NotAGame
