cmake_minimum_required(VERSION 3.14)

project(not-a-game LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GoogleTest)

add_compile_definitions(RAPIDJSON_HAS_STDSTRING RAPIDJSON_HAS_CXX11_RANGE_FOR)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt5 COMPONENTS Core Widgets LinguistTools REQUIRED)

include(FetchContent)

FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG master
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(range-v3
  GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
  GIT_TAG master
)
FetchContent_MakeAvailable(range-v3)

set(TS_FILES not-a-game_en_GB.ts)

include_directories(${CMAKE_SOURCE_DIR}/src/lib)
include_directories(${CMAKE_SOURCE_DIR}/src)

add_library(ui STATIC
  src/lib/ui/animation.h
  src/lib/ui/icon.h
)

add_library(util STATIC
  src/lib/util/assert.h
  src/lib/util/id.h
  src/lib/util/logger.h
  src/lib/util/map_utils.h
  src/lib/util/paged_vector.h
  src/lib/util/registry.h
  src/lib/util/settings.h
  src/lib/util/types.h
)
# target_link_libraries(util Qt5::Core)

add_library(entities STATIC
  src/lib/entities/building.h
  src/lib/entities/common.h
  src/lib/entities/components.h
  src/lib/entities/effect.h
  src/lib/entities/fraction.h
  src/lib/entities/global_map.cpp
  src/lib/entities/global_map.h
  src/lib/entities/item.h
  src/lib/entities/inventory.h
  src/lib/entities/lord.h
  src/lib/entities/map_components.h
  src/lib/entities/spell.h
  src/lib/entities/squad.h
  src/lib/entities/unit.h
  src/lib/entities/unit.cpp
)

target_link_libraries(entities PRIVATE range-v3::range-v3)

add_library(state STATIC
  src/lib/state/state.cpp
  src/lib/state/state.h
  src/lib/state/view.h
)

target_link_libraries(state PRIVATE range-v3::range-v3)

add_library(engine STATIC
  src/lib/engine/engine.cpp
  src/lib/engine/engine.h
  src/lib/engine/event.h
  src/lib/engine/map_view.h
  src/lib/engine/player.h
  src/lib/engine/player_state.h
)

target_link_libraries(engine PRIVATE state)


add_executable(not-a-game
  main.cpp
  src/client/ui/capital_view_window.cpp
  src/client/ui/capital_view_window.h
  src/client/ui/capital_view_window.ui
  src/client/ui/global_map_window.h
  src/client/ui/global_map_window.cpp
  src/client/ui/global_map_window.ui
  src/client/ui/new_turn_dialog.cpp
  src/client/ui/new_turn_dialog.h
  src/client/ui/new_turn_dialog.ui
  src/client/ui/player_setup_dialog.cpp
  src/client/ui/player_setup_dialog.h
  src/client/ui/player_setup_dialog.ui
  src/client/ui/resources_widget.cpp
  src/client/ui/resources_widget.h
  src/client/ui/start_window.cpp
  src/client/ui/start_window.h
  src/client/ui/start_window.ui
  src/client/ui/unit_widget.cpp
  src/client/ui/unit_widget.h
  src/client/ui/unit_widget.ui
  src/client/ui/squad_widget.cpp
  src/client/ui/squad_widget.h
  src/client/ui/squad_widget.ui
  src/lib/status/status.cpp
  src/lib/status/status.h
  src/lib/game/map.h
  src/lib/game/mod.cpp
  src/lib/game/mod.h
  src/lib/game/saga.h
  src/lib/game/settings.h
  ${TS_FILES}
)

target_link_libraries(not-a-game PRIVATE Qt5::Widgets)
target_link_libraries(not-a-game PRIVATE entities state util engine fmt::fmt)

add_executable(test_util
  src/lib/util/ut/test_paged_vector.cpp
  src/lib/util/ut/test_registry.cpp
)
target_link_libraries(test_util gtest gtest_main util)
gtest_add_tests(TARGET test_util)

add_executable(test_entities
  src/lib/entities/ut/test_resource.cpp
)
target_link_libraries(test_entities gtest gtest_main state)
gtest_add_tests(TARGET test_entities)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
