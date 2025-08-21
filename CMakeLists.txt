cmake_minimum_required(VERSION 3.16)
project(TaxonomyDict VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZIP REQUIRED libzip)

# Generate icon header file from PNG
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_data.h
    COMMAND xxd -i ${CMAKE_CURRENT_SOURCE_DIR}/icon/icon.png > ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_data.h.tmp
    COMMAND sed 's/unsigned char _[^\\[]*\\[/unsigned char taxonomy_dict_icon_png[/' ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_data.h.tmp > ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_data.h.tmp2
    COMMAND sed 's/unsigned int _[^=]*=/unsigned int taxonomy_dict_icon_png_len =/' ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_data.h.tmp2 > ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_data.h.tmp3
    COMMAND echo '#pragma once' > ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_data.h
    COMMAND echo '' >> ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_data.h
    COMMAND echo '// Embedded icon data generated from icon/icon.png' >> ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_data.h
    COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_data.h.tmp3 >> ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_data.h
    COMMAND rm -f ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_data.h.tmp ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_data.h.tmp2 ${CMAKE_CURRENT_SOURCE_DIR}/src/icon_data.h.tmp3
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/icon/icon.png
    COMMENT "Generating embedded icon header"
)

# Include directories
include_directories(${wxWidgets_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${LIBZIP_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    src/TaxonomyApp.cpp
    src/MainFrame.cpp
    src/TaxonomyProcessor.cpp
    src/Downloader.cpp
    src/ZipReader.cpp
    src/CSVParser.cpp
    src/DictionaryGenerator.cpp
    src/StarDictGenerator.cpp
)

set(HEADERS
    src/TaxonomyApp.hpp
    src/MainFrame.hpp
    src/TaxonomyProcessor.hpp
    src/Downloader.hpp
    src/ZipReader.hpp
    src/CSVParser.hpp
    src/DictionaryGenerator.hpp
    src/StarDictGenerator.hpp
    src/icon_data.h
)

# Platform-specific resources
if(WIN32)
    set(RESOURCE_FILES resources/TaxonomyDict.rc)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${RESOURCE_FILES})
else()
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
endif()

# Add compile definitions and options
target_compile_definitions(${PROJECT_NAME} PRIVATE ${wxWidgets_DEFINITIONS})

# Add other wxWidgets compile options (like -pthread)
target_compile_options(${PROJECT_NAME} PRIVATE ${wxWidgets_OTHER_OPTIONS})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    ${wxWidgets_LINK_LIBRARIES}
    ${CURL_LIBRARIES}
    ${LIBZIP_LIBRARIES}
)

# Compiler-specific options
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE __WXMSW__)
    # Additional Windows-specific settings
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

if(UNIX AND NOT APPLE)
    # Linux-specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE __WXGTK__)
endif()

# Set compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -O2>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -O2>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /O2>
)

# Installation rules
if(UNIX AND NOT APPLE)
    # Install executable
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
    
    # Install desktop file
    install(FILES resources/TaxonomyDict.desktop 
            DESTINATION share/applications)
    
    # Install icon
    install(FILES resources/icon.png 
            DESTINATION share/icons/hicolor/512x512/apps
            RENAME TaxonomyDict.png)
    
    # Install smaller icon sizes
    install(CODE "
        execute_process(COMMAND convert ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png -resize 48x48 ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps/TaxonomyDict.png)
        execute_process(COMMAND convert ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png -resize 64x64 ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/64x64/apps/TaxonomyDict.png)
        execute_process(COMMAND convert ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png -resize 128x128 ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/128x128/apps/TaxonomyDict.png)
        execute_process(COMMAND convert ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png -resize 256x256 ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/256x256/apps/TaxonomyDict.png)
    ")
endif()