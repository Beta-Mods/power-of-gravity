cmake_minimum_required(VERSION 3.24)

project(
    PowerOfGravity
    VERSION 1.0.0
    DESCRIPTION "Sun and moon gravity pull effect for Skyrim"
    LANGUAGES CXX
)

include(GNUInstallDirs)

option(CMS2_BUILD_PLUGIN "Build the Windows SKSE plugin" ON)
option(CMS2_BUILD_TESTS "Build the platform-independent logic and controller tests" ON)
option(CMS2_ENABLE_SKYRIM_VR "Build with Skyrim VR support" OFF)
set(CMS2_COMMONLIB_TAG "v15.7.3" CACHE STRING "Pinned CommonLibSSE-NG tag")

if(CMS2_BUILD_PLUGIN)
    if(NOT WIN32)
        message(FATAL_ERROR "The SKSE DLL is a Windows target. Configure with -DCMS2_BUILD_PLUGIN=OFF to run portable tests on another host.")
    endif()

    include(FetchContent)

    # CommonLibSSE-NG options must be set before it is added.
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(ENABLE_SKYRIM_SE ON CACHE BOOL "" FORCE)
    set(ENABLE_SKYRIM_AE ON CACHE BOOL "" FORCE)
    set(ENABLE_SKYRIM_VR ${CMS2_ENABLE_SKYRIM_VR} CACHE BOOL "" FORCE)
    set(SKSE_SUPPORT_XBYAK OFF CACHE BOOL "" FORCE)
    # This plugin replaces virtual-table entries and never writes a branch/call patch.
    # Disabling branch-patch diagnostics avoids fetching the unrelated hde64 dependency.
    set(SKSE_SUPPORT_PATCH_SAFETY OFF CACHE BOOL "" FORCE)
    set(COMMONLIB_ENABLE_IPO OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        CommonLibSSE
        GIT_REPOSITORY https://github.com/alandtse/CommonLibSSE-NG.git
        GIT_TAG ${CMS2_COMMONLIB_TAG}
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(CommonLibSSE)
    include("${commonlibsse_SOURCE_DIR}/cmake/CommonLibSSE.cmake")

    set(CMS2_SOURCES
        src/Config.cpp
        src/Config.h
        src/MoonController.cpp
        src/MoonController.h
        src/Plugin.cpp
        src/UI.cpp
        src/UI.h
        src/PCH.h
        extern/SKSEMenuFrameworkLite.h
    )

    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${CMS2_SOURCES})

    if(MSVC)
        enable_language(RC)
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in"
            "${CMAKE_CURRENT_BINARY_DIR}/CustomMoonSizes2.rc"
            @ONLY
        )
        list(APPEND CMS2_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/CustomMoonSizes2.rc")
        source_group("Generated" FILES "${CMAKE_CURRENT_BINARY_DIR}/CustomMoonSizes2.rc")
    endif()

    add_commonlibsse_plugin(
        ${PROJECT_NAME}
        NAME "Power of Gravity"
        AUTHOR "CKW25"
        VERSION ${PROJECT_VERSION}
        MINIMUM_SKSE_VERSION 2.0.20
        USE_ADDRESS_LIBRARY
        SOURCES ${CMS2_SOURCES}
    )

    target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
    target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/src"
            "${CMAKE_CURRENT_SOURCE_DIR}/extern"
    )
    target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

    if(MSVC)
        target_compile_options(
            ${PROJECT_NAME}
            PRIVATE
                /EHsc
                /permissive-
                /utf-8
                /W4
                /Zc:__cplusplus
                /Zc:preprocessor
                $<$<CONFIG:Release>:/Zi>
                /wd4100 # Unreferenced formal parameter in hook-compatible signatures.
                /wd4201 # CommonLib uses nameless structs/unions.
                /wd4324 # CommonLib types may be over-aligned.
        )
        target_link_options(
            ${PROJECT_NAME}
            PRIVATE
                $<$<CONFIG:Release>:/DEBUG:FULL>
                $<$<CONFIG:Release>:/INCREMENTAL:NO>
        )
    endif()

    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
            OUTPUT_NAME "PowerOfGravity"
    )

    # A convenient ready-to-zip loose-files output tree for local testing.
    set(CMS2_DIST_DIR "${CMAKE_BINARY_DIR}/dist")
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMS2_DIST_DIR}/Data/SKSE/Plugins"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMS2_DIST_DIR}/fomod"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${PROJECT_NAME}>"
            "${CMS2_DIST_DIR}/Data/SKSE/Plugins/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/package/Data/SKSE/Plugins/PowerOfGravity.ini"
            "${CMS2_DIST_DIR}/Data/SKSE/Plugins/PowerOfGravity.ini"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/package/fomod/info.xml"
            "${CMS2_DIST_DIR}/fomod/info.xml"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
            "${CMS2_DIST_DIR}/README.md"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md"
            "${CMS2_DIST_DIR}/CHANGELOG.md"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
            "${CMS2_DIST_DIR}/LICENSE"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY_NOTICES.md"
            "${CMS2_DIST_DIR}/THIRD_PARTY_NOTICES.md"
        VERBATIM
    )

    install(
        TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION "Data/SKSE/Plugins"
    )
    install(
        FILES "package/Data/SKSE/Plugins/PowerOfGravity.ini"
        DESTINATION "Data/SKSE/Plugins"
    )
    install(
        FILES "package/fomod/info.xml"
        DESTINATION "fomod"
    )
    install(
        FILES MSNG_Readme.txt LICENSE
        DESTINATION "."
    )
endif()

if(CMS2_BUILD_TESTS)
    enable_testing()


    add_executable(
        PowerOfGravityRuntimeTests
        tests/RuntimeControllerTests.cpp
        tests/TestHarness.h
        src/Config.cpp
        src/Config.h
        src/MoonController.cpp
        src/MoonController.h
    )
    target_compile_features(PowerOfGravityRuntimeTests PRIVATE cxx_std_23)
    target_include_directories(
        PowerOfGravityRuntimeTests
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/stubs"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests"
            "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )
    target_precompile_headers(PowerOfGravityRuntimeTests PRIVATE src/PCH.h)
    add_test(NAME RuntimeController COMMAND PowerOfGravityRuntimeTests)
    set_tests_properties(
        RuntimeController
        PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/runtime-test"
    )
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/runtime-test")
endif()

if(CMS2_BUILD_PLUGIN)
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_PACKAGE_NAME "PowerOfGravity")
    set(CPACK_PACKAGE_VENDOR "CKW25")
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_PACKAGE_FILE_NAME "PowerOfGravity-${PROJECT_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Sun and moon gravity pull effect for Skyrim")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    include(CPack)
endif()