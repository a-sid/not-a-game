#include "ui/icon.h"

#include <QPixmap>

namespace NotAGame {

Icon::Icon(const std::filesystem::path &Path) noexcept {
    Data.emplace<QPixmap>(QString::fromStdString(Path.string()));
}

}
