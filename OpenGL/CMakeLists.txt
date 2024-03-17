set(PROJECT_NAME OpenGL)
#cmake_minimum_required(VERSION 3.28)
#project(OpenGL)
#include("${PROJECT_SOURCE_DIR}/../CMake/Utils.cmake")
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_COMPILER_ID "MSVC")
#set(CMAKE_CXX_COMPILER_VERSION "19.latest")
add_compile_definitions(OPENGL_EXPORTS)

find_package(Freetype REQUIRED)

################################################################################
# Source groups
################################################################################
set(no_group_source_files
        "src/core.h"
    "src/Debug/ConsoleColor.h"
    "src/Debug/internal_debug_break.h"
    "src/Debug/Logger.cpp"
    "src/Debug/Logger.h"
    "src/Graphics/GLs/FrameBuffer.cpp"
    "src/Graphics/GLs/FrameBuffer.h"
    "src/Graphics/GLs/GLDebug.cpp"
    "src/Graphics/GLs/GLDebug.h"
    "src/Graphics/GLs/GLObject.cpp"
    "src/Graphics/GLs/GLObject.h"
    "src/Graphics/GLs/GLTypeID.h"
    "src/Graphics/GLs/RenderBuffer.cpp"
    "src/Graphics/GLs/RenderBuffer.h"
    "src/Graphics/Graphicals/CameraController.cpp"
    "src/Graphics/Graphicals/CameraController.h"
    "src/Graphics/Graphicals/Light.cpp"
    "src/Graphics/Graphicals/Light.h"
        "src/Graphics/Graphicals/MeshUtils.h"
    "src/Graphics/Graphicals/MeshUtilsImpl.h"
    "src/Graphics/Utils/Model Loading/MTLMaterialLoader.cpp"
    "src/Graphics/Utils/Model Loading/MTLMaterialLoader.h"
    "src/Graphics/Utils/Model Loading/OBJModel.cpp"
    "src/Graphics/Utils/Model Loading/OBJModel.h"
    "src/Graphics/Utils/Model Loading/OBJModelLoader.cpp"
    "src/Graphics/Utils/Model Loading/OBJModelLoader.h"
    "src/Graphics/Utils/Textures/TextureConstants.h"
    "src/IO/TimeType.cpp"
    "src/IO/TimeType.h"
    "src/Maths/Geometry.h"
    "src/Maths/Parsing.h"
    "src/Maths/Random.h"
    "src/stdu/parsing.h"
    "src/stdu/unique.h"
    "src/vendor/imgui/imgui_stdlib.cpp"
    "src/vendor/imgui/imgui_stdlib.h"
)
source_group("" FILES ${no_group_source_files})

set(Header_Files
    "src/Graphics/GLs/DynamicIndexBuffer.h"
    "src/Graphics/GLs/DynamicVertexBuffer.h"
    "src/Graphics/GLs/IndexBuffer.h"
    "src/Graphics/GLs/Render.h"
    "src/Graphics/GLs/Shader.h"
    "src/Graphics/GLs/VertexArray.h"
    "src/Graphics/GLs/VertexBuffer.h"
    "src/Graphics/GLs/VertexBufferLayout.h"
    "src/Graphics/GLs/VertexElement.h"
    "src/Graphics/Graphicals/GenericMesh.h"
    "src/Graphics/Graphicals/GraphicsDevice.h"
    "src/Graphics/Graphicals/Mesh.h"
    "src/Graphics/Graphicals/MeshUtils.h"
    "src/Graphics/Graphicals/MeshUtilsImpl.h"
    "src/Graphics/Graphicals/RenderData.h"
    "src/Graphics/Graphicals/RenderObject.h"
    "src/Graphics/Graphicals/TriIndices.h"
    "src/Graphics/Primitives/Quad.h"
    "src/Graphics/Primitives/Tri.h"
    "src/Graphics/Utils/Fonts/Font.h"
    "src/Graphics/Utils/Fonts/FontDevice.h"
    "src/Graphics/Utils/Fonts/TextAlign.h"
    "src/Graphics/Utils/Fonts/TextRenderer.h"
    "src/Graphics/Utils/Fonts/TextStyle.h"
    "src/Graphics/Utils/Textures/Texture.h"
    "src/IO/IO.h"
    "src/IO/Keyboard.h"
    "src/IO/Mouse.h"
    "src/Maths/Constants.h"
    "src/Maths/Corner.h"
    "src/Maths/Direction.h"
    "src/Maths/Matrix.h"
    "src/Maths/NumTypes.h"
    "src/Maths/Rect.h"
    "src/Maths/Vector.h"
    "src/Maths/VectorImpl.h"
    "src/stdu/enum_utils.h"
    "src/stdu/io.h"
    "src/stdu/macros.h"
    "src/stdu/ref.h"
    "src/stdu/rich_string.h"
    "src/stdu/sorted_vector.h"
    "src/stdu/types.h"
    "src/vendor/freetype/freetype/config/ftconfig.h"
    "src/vendor/freetype/freetype/config/ftheader.h"
    "src/vendor/freetype/freetype/config/ftmodule.h"
    "src/vendor/freetype/freetype/config/ftoption.h"
    "src/vendor/freetype/freetype/config/ftstdlib.h"
    "src/vendor/freetype/freetype/freetype.h"
    "src/vendor/freetype/freetype/ftadvanc.h"
    "src/vendor/freetype/freetype/ftbbox.h"
    "src/vendor/freetype/freetype/ftbdf.h"
    "src/vendor/freetype/freetype/ftbitmap.h"
    "src/vendor/freetype/freetype/ftbzip2.h"
    "src/vendor/freetype/freetype/ftcache.h"
    "src/vendor/freetype/freetype/ftchapters.h"
    "src/vendor/freetype/freetype/ftcid.h"
    "src/vendor/freetype/freetype/ftcolor.h"
    "src/vendor/freetype/freetype/ftdriver.h"
    "src/vendor/freetype/freetype/fterrdef.h"
    "src/vendor/freetype/freetype/fterrors.h"
    "src/vendor/freetype/freetype/ftfntfmt.h"
    "src/vendor/freetype/freetype/ftgasp.h"
    "src/vendor/freetype/freetype/ftglyph.h"
    "src/vendor/freetype/freetype/ftgxval.h"
    "src/vendor/freetype/freetype/ftgzip.h"
    "src/vendor/freetype/freetype/ftimage.h"
    "src/vendor/freetype/freetype/ftincrem.h"
    "src/vendor/freetype/freetype/ftlcdfil.h"
    "src/vendor/freetype/freetype/ftlist.h"
    "src/vendor/freetype/freetype/ftlzw.h"
    "src/vendor/freetype/freetype/ftmac.h"
    "src/vendor/freetype/freetype/ftmm.h"
    "src/vendor/freetype/freetype/ftmodapi.h"
    "src/vendor/freetype/freetype/ftmoderr.h"
    "src/vendor/freetype/freetype/ftotval.h"
    "src/vendor/freetype/freetype/ftoutln.h"
    "src/vendor/freetype/freetype/ftparams.h"
    "src/vendor/freetype/freetype/ftpfr.h"
    "src/vendor/freetype/freetype/ftrender.h"
    "src/vendor/freetype/freetype/ftsizes.h"
    "src/vendor/freetype/freetype/ftsnames.h"
    "src/vendor/freetype/freetype/ftstroke.h"
    "src/vendor/freetype/freetype/ftsynth.h"
    "src/vendor/freetype/freetype/ftsystem.h"
    "src/vendor/freetype/freetype/fttrigon.h"
    "src/vendor/freetype/freetype/fttypes.h"
    "src/vendor/freetype/freetype/ftwinfnt.h"
    "src/vendor/freetype/freetype/internal/autohint.h"
    "src/vendor/freetype/freetype/internal/cffotypes.h"
    "src/vendor/freetype/freetype/internal/cfftypes.h"
    "src/vendor/freetype/freetype/internal/ftcalc.h"
    "src/vendor/freetype/freetype/internal/ftdebug.h"
    "src/vendor/freetype/freetype/internal/ftdrv.h"
    "src/vendor/freetype/freetype/internal/ftgloadr.h"
    "src/vendor/freetype/freetype/internal/fthash.h"
    "src/vendor/freetype/freetype/internal/ftmemory.h"
    "src/vendor/freetype/freetype/internal/ftobjs.h"
    "src/vendor/freetype/freetype/internal/ftpsprop.h"
    "src/vendor/freetype/freetype/internal/ftrfork.h"
    "src/vendor/freetype/freetype/internal/ftserv.h"
    "src/vendor/freetype/freetype/internal/ftstream.h"
    "src/vendor/freetype/freetype/internal/fttrace.h"
    "src/vendor/freetype/freetype/internal/ftvalid.h"
    "src/vendor/freetype/freetype/internal/internal.h"
    "src/vendor/freetype/freetype/internal/psaux.h"
    "src/vendor/freetype/freetype/internal/pshints.h"
    "src/vendor/freetype/freetype/internal/services/svbdf.h"
    "src/vendor/freetype/freetype/internal/services/svcfftl.h"
    "src/vendor/freetype/freetype/internal/services/svcid.h"
    "src/vendor/freetype/freetype/internal/services/svfntfmt.h"
    "src/vendor/freetype/freetype/internal/services/svgldict.h"
    "src/vendor/freetype/freetype/internal/services/svgxval.h"
    "src/vendor/freetype/freetype/internal/services/svkern.h"
    "src/vendor/freetype/freetype/internal/services/svmetric.h"
    "src/vendor/freetype/freetype/internal/services/svmm.h"
    "src/vendor/freetype/freetype/internal/services/svotval.h"
    "src/vendor/freetype/freetype/internal/services/svpfr.h"
    "src/vendor/freetype/freetype/internal/services/svpostnm.h"
    "src/vendor/freetype/freetype/internal/services/svprop.h"
    "src/vendor/freetype/freetype/internal/services/svpscmap.h"
    "src/vendor/freetype/freetype/internal/services/svpsinfo.h"
    "src/vendor/freetype/freetype/internal/services/svsfnt.h"
    "src/vendor/freetype/freetype/internal/services/svttcmap.h"
    "src/vendor/freetype/freetype/internal/services/svtteng.h"
    "src/vendor/freetype/freetype/internal/services/svttglyf.h"
    "src/vendor/freetype/freetype/internal/services/svwinfnt.h"
    "src/vendor/freetype/freetype/internal/sfnt.h"
    "src/vendor/freetype/freetype/internal/t1types.h"
    "src/vendor/freetype/freetype/internal/tttypes.h"
    "src/vendor/freetype/freetype/t1tables.h"
    "src/vendor/freetype/freetype/ttnameid.h"
    "src/vendor/freetype/freetype/tttables.h"
    "src/vendor/freetype/freetype/tttags.h"
    "src/vendor/freetype/ft2build.h"
    "src/vendor/imgui/imconfig.h"
    "src/vendor/imgui/imgui.h"
    "src/vendor/imgui/imgui_impl_glfw.h"
    "src/vendor/imgui/imgui_impl_opengl3.h"
    "src/vendor/imgui/imgui_internal.h"
    "src/vendor/imgui/imstb_rectpack.h"
    "src/vendor/imgui/imstb_textedit.h"
    "src/vendor/imgui/imstb_truetype.h"
    "src/vendor/stb_image/stb_image.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "src/Graphics/GLs/DynamicIndexBuffer.cpp"
    "src/Graphics/GLs/DynamicVertexBuffer.cpp"
    "src/Graphics/GLs/IndexBuffer.cpp"
    "src/Graphics/GLs/Render.cpp"
    "src/Graphics/GLs/Shader.cpp"
    "src/Graphics/GLs/VertexArray.cpp"
    "src/Graphics/GLs/VertexBuffer.cpp"
    "src/Graphics/GLs/VertexBufferLayout.cpp"
    "src/Graphics/Graphicals/GenericMesh.cpp"
    "src/Graphics/Graphicals/GraphicsDevice.cpp"
    "src/Graphics/Graphicals/RenderData.cpp"
    "src/Graphics/Primitives/Quad.cpp"
    "src/Graphics/Primitives/Tri.cpp"
    "src/Graphics/Utils/Fonts/Font.cpp"
    "src/Graphics/Utils/Fonts/FontDevice.cpp"
    "src/Graphics/Utils/Fonts/TextAlign.cpp"
    "src/Graphics/Utils/Fonts/TextRenderer.cpp"
    "src/Graphics/Utils/Textures/Texture.cpp"
    "src/IO/IO.cpp"
    "src/IO/Keyboard.cpp"
    "src/IO/Mouse.cpp"
    "src/Maths/Color.cpp"
    "src/Maths/Matrix.cpp"
    "src/stdu/rich_string.cpp"
    "src/vendor/imgui/imgui.cpp"
    "src/vendor/imgui/imgui_demo.cpp"
    "src/vendor/imgui/imgui_draw.cpp"
    "src/vendor/imgui/imgui_impl_glfw.cpp"
    "src/vendor/imgui/imgui_impl_opengl3.cpp"
    "src/vendor/imgui/imgui_tables.cpp"
    "src/vendor/imgui/imgui_widgets.cpp"
    "src/vendor/imgui/main.cpp"
    "src/vendor/stb_image/stb_image.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${no_group_source_files}
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} SHARED ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE OpenGL)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
    OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
)
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
    "${CMAKE_CURRENT_SOURCE_DIR}/src;"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Debug;"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics/Utils/Fonts;"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics/Utils/Model Loading"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics/GLs;"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics/Graphicals;"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics/Primitives;"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Maths;"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics/Utils/Textures;"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/IO;"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/imgui;"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor;"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/test;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/GLFW/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/GLEW/include"
)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${FREETYPE_INCLUDE_DIRS}
)

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "GLEW_STATIC;"
        "OPENGL_EXPORTS;"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "GLEW_STATIC;"
        "OPENGL_EXPORTS;"
        "UNICODE;"
        "_UNICODE"
    )
endif()
set_source_files_properties(src/Graphics/GLs/RenderBuffer.cpp PROPERTIES
    COMPILE_DEFINITIONS "GLEW_STATIC;OPENGL_EXPORTS"
)
set_source_files_properties(src/IO/TimeType.cpp PROPERTIES
    COMPILE_DEFINITIONS "GLEW_STATIC;OPENGL_EXPORTS"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /TP
            >
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
            /DEBUG;
            /SUBSYSTEM:WINDOWS
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
            /DEBUG;
            /SUBSYSTEM:WINDOWS
        )
    endif()
endif()

################################################################################
# Post build events
################################################################################
add_custom_command_if(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMANDS
    COMMAND   $<CONFIG:Debug> $<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/>post_build.bat ${CMAKE_VS_PLATFORM_NAME} $<CONFIG>
)

################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
#    "${CMAKE_CURRENT_SOURCE_DIR}\\src\\vendor\\freetype\\freetype.lib"
    "glew32s;"
    "glfw3;"
    "Opengl32"
)
target_link_libraries(${PROJECT_NAME} PUBLIC
    "${ADDITIONAL_LIBRARY_DEPENDENCIES};"
    ${FREETYPE_LIBRARIES}
)

target_link_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2022/${CMAKE_VS_PLATFORM_NAME};"
    "${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/${CMAKE_VS_PLATFORM_NAME}"
)

