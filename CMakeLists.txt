cmake_minimum_required(VERSION 3.16)
project(RaftPirates LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(CTest)
set(HYDRA_LEGACY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../HydraLegacy" CACHE PATH "Path to the HydraLegacy source repository")
if(NOT TARGET hydra)
  if(NOT EXISTS "${HYDRA_LEGACY_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Clone HydraLegacy next to RaftPirates or set HYDRA_LEGACY_DIR.")
  endif()
  add_subdirectory("${HYDRA_LEGACY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/HydraLegacy")
endif()
add_library(raftpirates_game STATIC
    src/GameRuntime.cpp
    src/Ocean.cpp
    src/RaftPirates.cpp
    src/Messages.cpp
    src/Game.cpp
    src/BuildModeState.cpp
    src/GridObject.cpp
    src/OverlapTree.cpp
    src/PerimeterTree.cpp
    src/GridObjectDataRes.cpp
    src/DiscoveryGridObject.cpp
    src/GridObjectHelpers.cpp
    src/FootprintSprite.cpp
    src/ItemPlacementSprite.cpp
    src/PlacementSprite.cpp
    src/ExplosionFXSprite.cpp
    src/ItemOnRaftPlacementSprite.cpp
    src/WaterBucketSprite.cpp
    src/ThrobingIndicatorSprite.cpp
    src/UnitHealthBarSprite.cpp
    src/GridObjectInfo.cpp
    src/GridSprite.cpp
    src/AccessorySprite.cpp
    src/Player.cpp
    src/Material.cpp
    src/Blueprint.cpp
    src/DivingGame.cpp
    src/ScrollingLayer.cpp
    src/BattleArrowSprite.cpp
    src/DamageIndicatorSprite.cpp
    src/RaftComponent.cpp
    src/Wreckage.cpp
    src/Barracks.cpp
    src/RaftFactory.cpp
    src/RaftInfo.cpp
    src/RaftComponentFactory.cpp
    src/Raft.cpp
    src/OceanView.cpp
    src/GfxOceanGradient.cpp
    src/GfxOceanGrid.cpp
    src/MsgQueuer.cpp
    src/Transition.cpp
    src/PortView.cpp
    src/SplashScreen.cpp
    src/TitleScreen.cpp
    src/LoginScreen.cpp
    src/Zone.cpp
    src/Discovery.cpp
    src/Hud.cpp
    src/Quest.cpp
    src/MapView.cpp
    src/RPMenuHandler.cpp
    src/SparseGrid.cpp
    src/BattleModeState.cpp
    src/BattlePlacementScreen.cpp
    src/BattleShipPlacementSprite.cpp
    src/GfxBattleScreen.cpp
    src/DiscoveryModeState.cpp
    src/StaticDatabases.cpp
    src/StoreDB.cpp
    src/DivingUnit.cpp
    src/TouchControls.cpp
    src/LoadingManager.cpp
    src/Loader.cpp
    src/LoaderImage.cpp
    src/LoaderAEAnim.cpp
    src/LoaderSpriteSheet.cpp
    src/LoaderXml.cpp
    src/LoaderMenu.cpp
    src/LoaderManifest.cpp
)
target_include_directories(raftpirates_game PUBLIC include)
target_link_libraries(raftpirates_game PUBLIC hydra sfs_legacy)
if(ANDROID OR HYDRA_DESKTOP_JNI)
  add_library(raftpirates SHARED $<TARGET_OBJECTS:hydra_android_game>)
  target_link_libraries(raftpirates PRIVATE raftpirates_game hydra_java sfs_java)
  if(ANDROID)
    target_link_options(raftpirates PRIVATE "-Wl,--no-undefined" "-Wl,-z,max-page-size=16384")
  endif()
endif()
if(NOT ANDROID)
  add_executable(raftpirates_preview src/main.cpp)
  target_link_libraries(raftpirates_preview PRIVATE raftpirates_game)
  if(BUILD_TESTING)
    add_executable(raftpirates_tests tests/test.cpp)
    target_link_libraries(raftpirates_tests PRIVATE raftpirates_game)
    add_test(NAME raftpirates_protocol COMMAND raftpirates_tests)
    add_executable(raftpirates_hud_tests tests/hud.cpp)
    target_link_libraries(raftpirates_hud_tests PRIVATE raftpirates_game)
    set(RAFT_ASSET_ROOT "" CACHE PATH "Optional local original assets for HUD integration tests")
    if(RAFT_ASSET_ROOT)
      add_test(NAME raftpirates_original_hud COMMAND raftpirates_hud_tests "${RAFT_ASSET_ROOT}")
      add_executable(raftpirates_login_tests tests/login.cpp)
      target_link_libraries(raftpirates_login_tests PRIVATE raftpirates_game)
      add_test(NAME raftpirates_original_login COMMAND raftpirates_login_tests "${RAFT_ASSET_ROOT}")
    endif()
  endif()
endif()

if(NOT ANDROID AND BUILD_TESTING)
  add_executable(raftpirates_helper_tests tests/helpers.cpp)
  target_link_libraries(raftpirates_helper_tests PRIVATE raftpirates_game)
  add_test(NAME raftpirates_helpers COMMAND raftpirates_helper_tests)
  if(RAFT_ASSET_ROOT)
    add_test(NAME raftpirates_original_manifest COMMAND raftpirates_helper_tests "${RAFT_ASSET_ROOT}")
  endif()
  add_executable(raftpirates_play_tests tests/play.cpp)
  target_link_libraries(raftpirates_play_tests PRIVATE raftpirates_game)
  add_test(NAME raftpirates_sparse_grid COMMAND raftpirates_play_tests)
  if(RAFT_ASSET_ROOT)
    add_test(NAME raftpirates_original_map COMMAND raftpirates_play_tests "${RAFT_ASSET_ROOT}")
  endif()
endif()

if(NOT ANDROID AND BUILD_TESTING)
  add_executable(raftpirates_component_tests tests/game_components.cpp)
  target_link_libraries(raftpirates_component_tests PRIVATE raftpirates_game)
  add_test(NAME raftpirates_components COMMAND raftpirates_component_tests)
endif()

if(NOT ANDROID AND BUILD_TESTING)
  add_executable(raftpirates_session_tests tests/session.cpp)
  target_link_libraries(raftpirates_session_tests PRIVATE raftpirates_game)
  add_test(NAME raftpirates_session COMMAND raftpirates_session_tests)
endif()

if(NOT ANDROID AND BUILD_TESTING)
  add_executable(raftpirates_multiplayer_tests tests/multiplayer.cpp)
  target_link_libraries(raftpirates_multiplayer_tests PRIVATE raftpirates_game)
  add_test(NAME raftpirates_multiplayer COMMAND raftpirates_multiplayer_tests)
  if(RAFT_ASSET_ROOT)
    add_test(NAME raftpirates_original_battle COMMAND raftpirates_multiplayer_tests "${RAFT_ASSET_ROOT}")
    add_test(NAME hydra_original_menu_components COMMAND $<TARGET_FILE:hydra_menu_component_tests> "${RAFT_ASSET_ROOT}")
  endif()
endif()
