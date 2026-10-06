set(PIANOMANIA_COMPOSER_PROFILE "development" CACHE STRING "Composer product profile")
set_property(CACHE PIANOMANIA_COMPOSER_PROFILE PROPERTY STRINGS development production)
if(NOT PIANOMANIA_COMPOSER_PROFILE MATCHES "^(development|production)$")
    message(FATAL_ERROR "PIANOMANIA_COMPOSER_PROFILE must be development or production")
endif()
if(PIANOMANIA_COMPOSER_PROFILE STREQUAL "production")
    set(PIANOMANIA_COMPOSER_SERVICE_URL "https://pianomania.gg")
    add_compile_definitions(PIANOMANIA_COMPOSER_PRODUCTION=1)
    set(MUE_BUILD_ENGRAVING_DEVTOOLS OFF CACHE BOOL "Build engraving devtools" FORCE)
else()
    set(PIANOMANIA_COMPOSER_SERVICE_URL "http://127.0.0.1:5038" CACHE STRING "Development Composer service")
    if(NOT PIANOMANIA_COMPOSER_SERVICE_URL MATCHES "^(https://[a-zA-Z0-9.-]+(:[0-9]+)?|http://127\\.0\\.0\\.1:[0-9]+)$")
        message(FATAL_ERROR "Development Composer requires HTTPS or an explicit local loopback port")
    endif()
endif()
add_compile_definitions(
    PIANOMANIA_COMPOSER_PROFILE="${PIANOMANIA_COMPOSER_PROFILE}"
    PIANOMANIA_COMPOSER_SERVICE_URL="${PIANOMANIA_COMPOSER_SERVICE_URL}"
)
