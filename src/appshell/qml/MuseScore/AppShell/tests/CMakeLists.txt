# SPDX-License-Identifier: GPL-3.0-only

set(MODULE_TEST appshell_qml_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/../authoringcreationmenus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../authoringcreationmenus.h
    ${CMAKE_CURRENT_LIST_DIR}/authoringcreationmenus_tests.cpp
)

set(MODULE_TEST_LINK
    muse_uicomponents_qml
)

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(SetupGTest)
