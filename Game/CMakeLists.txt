set(PROJECT_NAME Game)
#cmake_minimum_required(VERSION 3.28)
#project(Game)
#include("${PROJECT_SOURCE_DIR}/../CMake/Utils.cmake")

################################################################################
# Source groups
################################################################################
set(Source_Files
    "Game.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE Game)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Output directory
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/out/$<CONFIG>/${CMAKE_VS_PLATFORM_NAME}/out/"
        OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/out/$<CONFIG>/${CMAKE_VS_PLATFORM_NAME}/out/"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/out/$<CONFIG>/${CMAKE_VS_PLATFORM_NAME}/out/${PROJECT_NAME}/"
        OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/out/$<CONFIG>/${CMAKE_VS_PLATFORM_NAME}/out/"
    )
endif()
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
endif()
################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/../OpenGL/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../OpenGL/src/Graphics/GLs;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../OpenGL/src/Graphics/Graphicals;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../OpenGL/src/Graphics/Meshes;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../OpenGL/src/Graphics/Utils/Textures;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../OpenGL/src/Graphics/Utils/Fonts;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../OpenGL/src/Graphics/Utils/Model Loading;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../OpenGL/src/IO;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../OpenGL/src/Maths;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../OpenGL/src/vendor/imgui;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../OpenGL/src/vendor/freetype;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../OpenGL/src/stdu;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/GLEW/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/GLFW/include"
)

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "WIN32;"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /Od
            >
            $<$<CONFIG:Release>:
                /O2;
                /Oi;
                /Gy
            >
            /permissive-;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Y-
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /Od
            >
            $<$<CONFIG:Release>:
                /O2;
                /Oi;
                /Gy
            >
            /permissive-;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Y-
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /INCREMENTAL
            >
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /INCREMENTAL
            >
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    endif()
endif()

################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "${PROJECT_SOURCE_DIR}\\..\\bin\\${CMAKE_VS_PLATFORM_NAME}\\${CMAKE_BUILD_TYPE}\\OpenGL;"
    "glew32s;"
    "glfw3;"
    "Opengl32;"
    "$$(CoreLibraryDependencies)"
)
#if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
#    set(ADDITIONAL_LIBRARY_DEPENDENCIES
#        "$(SolutionDir)bin\$(Platform)\$(Configuration)\\OpenGL;"
#        "glew32s;"
#        "glfw3;"
#        "Opengl32;"
#        "$(CoreLibraryDependencies)"
#    )
#elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
#    set(ADDITIONAL_LIBRARY_DEPENDENCIES
#        "$<$<CONFIG:Debug>:"
#            "OpenGL"
#        ">"
#        "$<$<CONFIG:Release>:"
#            "$(SolutionDir)bin\$(Platform)\$(Configuration)\\OpenGL;"
#            "glew32s;"
#            "glfw3;"
#            "Opengl32"
#        ">"
#        "$(CoreLibraryDependencies)"
#    )
#endif()
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_SOURCE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>;"
        "${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2022/${CMAKE_VS_PLATFORM_NAME};"
        "${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/${CMAKE_VS_PLATFORM_NAME}"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Release>:"
            "${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2022/${CMAKE_VS_PLATFORM_NAME};"
            "${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/${CMAKE_VS_PLATFORM_NAME}"
        ">"
        "${CMAKE_SOURCE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>"
    )
endif()

