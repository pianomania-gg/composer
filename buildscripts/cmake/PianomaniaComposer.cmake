set(PIANOMANIA_COMPOSER_PROFILE "production" CACHE STRING "Composer product profile")
set_property(CACHE PIANOMANIA_COMPOSER_PROFILE PROPERTY STRINGS development production)
if(NOT PIANOMANIA_COMPOSER_PROFILE MATCHES "^(development|production)$")
    message(FATAL_ERROR "PIANOMANIA_COMPOSER_PROFILE must be development or production")
endif()
if(MUSE_APP_BUILD_MODE MATCHES "^(release|testing)$" AND NOT PIANOMANIA_COMPOSER_PROFILE STREQUAL "production")
    message(FATAL_ERROR "Published Composer builds require PIANOMANIA_COMPOSER_PROFILE=production. Use MUSE_APP_BUILD_MODE=dev for development builds.")
endif()
if(PIANOMANIA_COMPOSER_PROFILE STREQUAL "production")
    set(PIANOMANIA_COMPOSER_SERVICE_URL "https://pianomania.gg")
    add_compile_definitions(PIANOMANIA_COMPOSER_PRODUCTION=1)
    # AppShell instantiates engraving QML types even with developer UI hidden.
    set(MUE_BUILD_ENGRAVING_DEVTOOLS ON CACHE BOOL "Build engraving QML dependencies" FORCE)
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
