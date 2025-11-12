cmake_minimum_required(VERSION 3.16)
project(TaxonomyTools VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_TAXONOMY_DICT "Build TaxonomyDict application" ON)
option(BUILD_TAXONOMY_TREE "Build TaxonomyTree application" ON)

# Set output directory to build root instead of build/src
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Find required packages
find_package(PkgConfig REQUIRED)

# Find wxWidgets using wx-config
execute_process(
    COMMAND wx-config --cxxflags
    OUTPUT_VARIABLE wxWidgets_CXX_FLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND wx-config --libs
    OUTPUT_VARIABLE wxWidgets_LIBRARIES
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Convert flags to lists for proper handling
separate_arguments(wxWidgets_CXX_FLAGS_LIST UNIX_COMMAND "${wxWidgets_CXX_FLAGS}")
separate_arguments(wxWidgets_LINK_LIBRARIES UNIX_COMMAND "${wxWidgets_LIBRARIES}")

# Extract include directories
set(wxWidgets_INCLUDE_DIRS)
foreach(flag ${wxWidgets_CXX_FLAGS_LIST})
    if(flag MATCHES "^-I")
        string(SUBSTRING ${flag} 2 -1 include_dir)
        list(APPEND wxWidgets_INCLUDE_DIRS ${include_dir})
    endif()
endforeach()

# Extract definitions
set(wxWidgets_DEFINITIONS)
foreach(flag ${wxWidgets_CXX_FLAGS_LIST})
    if(flag MATCHES "^-D")
        string(SUBSTRING ${flag} 2 -1 definition)
        list(APPEND wxWidgets_DEFINITIONS ${definition})
    endif()
endforeach()

# Extract other options
set(wxWidgets_OTHER_OPTIONS)
foreach(flag ${wxWidgets_CXX_FLAGS_LIST})
    if(flag MATCHES "^-[^DI]")
        list(APPEND wxWidgets_OTHER_OPTIONS ${flag})
    endif()
endforeach()

find_package(CURL REQUIRED)

# Find libzip
pkg_check_modules(LIBZIP REQUIRED libzip)

# Common library sources
set(COMMON_SOURCES
    src/common/Downloader.cpp
    src/common/ZipReader.cpp
    src/common/CSVParser.cpp
    src/common/Translator.cpp
)

set(COMMON_HEADERS
    src/common/Downloader.hpp
    src/common/ZipReader.hpp
    src/common/CSVParser.hpp
    src/common/Translator.hpp
    src/common/TranslatorValidator.hpp
)

# Create static library for common functionality
add_library(TaxonomyCommon STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

# Include directories for common library
target_include_directories(TaxonomyCommon PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${wxWidgets_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${LIBZIP_INCLUDE_DIRS}
)

# Add compile definitions and options to common library
target_compile_definitions(TaxonomyCommon PUBLIC ${wxWidgets_DEFINITIONS})
target_compile_options(TaxonomyCommon PUBLIC ${wxWidgets_OTHER_OPTIONS})

# Link libraries to common library
target_link_libraries(TaxonomyCommon PUBLIC
    ${wxWidgets_LINK_LIBRARIES}
    ${CURL_LIBRARIES}
    ${LIBZIP_LIBRARIES}
)

# Platform-specific settings for common library
if(WIN32)
    target_compile_definitions(TaxonomyCommon PUBLIC __WXMSW__)
endif()

if(UNIX AND NOT APPLE)
    target_compile_definitions(TaxonomyCommon PUBLIC __WXGTK__)
endif()

# Compiler flags for common library
target_compile_options(TaxonomyCommon PUBLIC
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -O2>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -O2>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /O2>
)

# TaxonomyDict application
if(BUILD_TAXONOMY_DICT)
    # Generate icon header file from PNG
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/taxonomy-dict/icon_data.h
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/generate_icon.sh
                ${CMAKE_CURRENT_SOURCE_DIR}/icon/icon.png
                ${CMAKE_CURRENT_SOURCE_DIR}/src/taxonomy-dict/icon_data.h
                taxonomy_dict_icon_png
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/icon/icon.png
                ${CMAKE_CURRENT_SOURCE_DIR}/generate_icon.sh
        COMMENT "Generating embedded icon header for TaxonomyDict"
    )

    set(TAXONOMY_DICT_SOURCES
        src/taxonomy-dict/main.cpp
        src/taxonomy-dict/TaxonomyApp.cpp
        src/taxonomy-dict/MainFrame.cpp
        src/taxonomy-dict/SettingsDialog.cpp
        src/taxonomy-dict/TaxonomyProcessor.cpp
        src/taxonomy-dict/DictionaryGenerator.cpp
        src/taxonomy-dict/StarDictGenerator.cpp
        src/taxonomy-dict/DictStrings_EN.cpp
        src/taxonomy-dict/DictStrings_LT.cpp
        src/taxonomy-dict/DictStrings_ES.cpp
        src/taxonomy-tree/TreeStrings_EN.cpp
        src/taxonomy-tree/TreeStrings_LT.cpp
        src/taxonomy-tree/TreeStrings_ES.cpp
    )

    set(TAXONOMY_DICT_HEADERS
        src/taxonomy-dict/TaxonomyApp.hpp
        src/taxonomy-dict/MainFrame.hpp
        src/taxonomy-dict/SettingsDialog.hpp
        src/taxonomy-dict/TaxonomyProcessor.hpp
        src/taxonomy-dict/DictionaryGenerator.hpp
        src/taxonomy-dict/StarDictGenerator.hpp
        src/taxonomy-dict/DictStrings.hpp
        src/taxonomy-dict/icon_data.h
    )

    # Platform-specific resources
    if(WIN32)
        set(RESOURCE_FILES resources/TaxonomyDict.rc)
        add_executable(TaxonomyDict WIN32 ${TAXONOMY_DICT_SOURCES} ${TAXONOMY_DICT_HEADERS} ${RESOURCE_FILES})
    else()
        add_executable(TaxonomyDict ${TAXONOMY_DICT_SOURCES} ${TAXONOMY_DICT_HEADERS})
    endif()

    # Link with common library
    target_link_libraries(TaxonomyDict PRIVATE TaxonomyCommon)

    # Include directories
    target_include_directories(TaxonomyDict PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/taxonomy-dict
        ${CMAKE_CURRENT_SOURCE_DIR}/src/taxonomy-tree
    )

    # Windows-specific settings
    if(WIN32)
        set_target_properties(TaxonomyDict PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()

    # Installation rules for TaxonomyDict
    if(UNIX AND NOT APPLE)
        install(TARGETS TaxonomyDict DESTINATION bin)
        install(FILES resources/TaxonomyDict.desktop DESTINATION share/applications)
        install(FILES resources/icon.png DESTINATION share/icons/hicolor/512x512/apps RENAME TaxonomyDict.png)

        # Install smaller icon sizes
        install(CODE "
            execute_process(COMMAND convert ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png -resize 48x48 ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps/TaxonomyDict.png)
            execute_process(COMMAND convert ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png -resize 64x64 ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/64x64/apps/TaxonomyDict.png)
            execute_process(COMMAND convert ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png -resize 128x128 ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/128x128/apps/TaxonomyDict.png)
            execute_process(COMMAND convert ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png -resize 256x256 ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/256x256/apps/TaxonomyDict.png)
        ")
    endif()
endif()

# TaxonomyTree application
if(BUILD_TAXONOMY_TREE)
    # Generate icon header file from PNG
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/taxonomy-tree/icon_data.h
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/generate_icon.sh
                ${CMAKE_CURRENT_SOURCE_DIR}/icon/tree-icon.png
                ${CMAKE_CURRENT_SOURCE_DIR}/src/taxonomy-tree/icon_data.h
                taxonomy_tree_icon_png
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/icon/tree-icon.png
                ${CMAKE_CURRENT_SOURCE_DIR}/generate_icon.sh
        COMMENT "Generating embedded icon header for TaxonomyTree"
    )

    set(TAXONOMY_TREE_SOURCES
        src/taxonomy-tree/main.cpp
        src/taxonomy-tree/TreeApp.cpp
        src/taxonomy-tree/TreeFrame.cpp
        src/taxonomy-tree/TaxonomyData.cpp
        src/taxonomy-tree/SearchIndex.cpp
        src/taxonomy-tree/TreeRenderer.cpp
        src/taxonomy-tree/SettingsDialog.cpp
        src/taxonomy-tree/TreeStrings_EN.cpp
        src/taxonomy-tree/TreeStrings_LT.cpp
        src/taxonomy-tree/TreeStrings_ES.cpp
    )

    set(TAXONOMY_TREE_HEADERS
        src/taxonomy-tree/TreeApp.hpp
        src/taxonomy-tree/TreeFrame.hpp
        src/taxonomy-tree/TaxonomyData.hpp
        src/taxonomy-tree/SearchIndex.hpp
        src/taxonomy-tree/TreeRenderer.hpp
        src/taxonomy-tree/SettingsDialog.hpp
        src/taxonomy-tree/TreeStrings.hpp
        src/taxonomy-tree/icon_data.h
    )

    # Platform-specific resources
    if(WIN32)
        set(TREE_RESOURCE_FILES resources/TaxonomyTree.rc)
        add_executable(TaxonomyTree WIN32 ${TAXONOMY_TREE_SOURCES} ${TAXONOMY_TREE_HEADERS} ${TREE_RESOURCE_FILES})
    else()
        add_executable(TaxonomyTree ${TAXONOMY_TREE_SOURCES} ${TAXONOMY_TREE_HEADERS})
    endif()

    # Link with common library
    target_link_libraries(TaxonomyTree PRIVATE TaxonomyCommon)

    # Include directories
    target_include_directories(TaxonomyTree PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/taxonomy-tree
    )

    # Windows-specific settings
    if(WIN32)
        set_target_properties(TaxonomyTree PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()

    # Installation rules for TaxonomyTree
    if(UNIX AND NOT APPLE)
        install(TARGETS TaxonomyTree DESTINATION bin)
        install(FILES resources/TaxonomyTree.desktop DESTINATION share/applications)
        install(FILES icon/tree-icon.png DESTINATION share/icons/hicolor/512x512/apps RENAME TaxonomyTree.png)
    endif()
endif()

# CPack configuration for DEB packages
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "taxonomy-tools")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_CONTACT "darauble@github.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Taxonomic Dictionary and Family Tree Tools")
set(CPACK_PACKAGE_DESCRIPTION "Tools for working with iNaturalist taxonomic data:
 TaxonomyDict - Generate StarDict dictionaries from taxonomic data
 TaxonomyTree - Visualize taxonomic family trees and compare species")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libwxgtk3.2-1, libcurl4, libzip4")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/darauble/TaxonomyTools")
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-amd64")

include(CPack)
