cmake_minimum_required(VERSION 3.22)
project(ComposerBackgroundTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets Test)
enable_testing()
add_executable(composer_background_tests backgroundtest.cpp
    ../../internal/composerbackgrounddialog.cpp
    ../../resources/composer_backgrounds.qrc)
target_link_libraries(composer_background_tests PRIVATE Qt6::Widgets Qt6::Test)
add_test(NAME composer_background_tests COMMAND composer_background_tests -platform offscreen)
set_tests_properties(composer_background_tests PROPERTIES TIMEOUT 45)
