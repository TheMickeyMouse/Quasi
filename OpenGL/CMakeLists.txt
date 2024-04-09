set(PROJECT_NAME OpenGL)

find_package(Freetype REQUIRED)

################################################################################
# Source groups
################################################################################
set(HEADER_FILES
    src/core.h

    src/Debug/ConsoleColor.h
    src/Debug/internal_debug_break.h
    src/Debug/Logger.h

    src/Graphics/GLs/DynamicIndexBuffer.h
    src/Graphics/GLs/DynamicVertexBuffer.h
    src/Graphics/GLs/IndexBuffer.h
    src/Graphics/GLs/Render.h
    src/Graphics/GLs/Shader.h
    src/Graphics/GLs/VertexArray.h
    src/Graphics/GLs/VertexBuffer.h
    src/Graphics/GLs/VertexBufferLayout.h
    src/Graphics/GLs/VertexElement.h
    src/Graphics/GLs/VertexConverter.h
    src/Graphics/GLs/FrameBuffer.h
    src/Graphics/GLs/GLDebug.h
    src/Graphics/GLs/GLObject.h
    src/Graphics/GLs/GLTypeID.h
    src/Graphics/GLs/RenderBuffer.h

    src/Graphics/Graphicals/GenericMesh.h
    src/Graphics/Graphicals/GraphicsDevice.h
    src/Graphics/Graphicals/Mesh.h
    src/Graphics/Graphicals/RenderData.h
    src/Graphics/Graphicals/RenderObject.h
    src/Graphics/Graphicals/TriIndices.h
    src/Graphics/Graphicals/CameraController.h
    src/Graphics/Graphicals/Light.h

    src/Graphics/Primitives/Quad.h
    src/Graphics/Primitives/Tri.h
    src/Graphics/Utils/ModelLoading/MTLMaterialLoader.h
    src/Graphics/Utils/ModelLoading/OBJModel.h
    src/Graphics/Utils/ModelLoading/OBJModelLoader.h
    src/Graphics/Utils/Textures/TextureConstants.h
    src/Graphics/Utils/Textures/Texture.h
    src/Graphics/Utils/Fonts/Font.h
    src/Graphics/Utils/Fonts/FontDevice.h
    src/Graphics/Utils/Fonts/TextAlign.h
    src/Graphics/Utils/Fonts/TextRenderer.h
    src/Graphics/Utils/Fonts/TextStyle.h
    src/Graphics/Utils/Meshes/Circle.h
    src/Graphics/Utils/Meshes/MeshConstructor.h
    src/Graphics/Utils/Meshes/Cube.h
    src/Graphics/Utils/Meshes/CubeNormless.h
    src/Graphics/Utils/Meshes/Sphere.h
    src/Graphics/Utils/Meshes/Icosphere.h

    src/IO/IO.h
    src/IO/Keyboard.h
    src/IO/Mouse.h
    src/IO/TimeType.h

    src/Maths/Constants.h
    src/Maths/Corner.h
    src/Maths/Direction.h
    src/Maths/Matrix.h
    src/Maths/NumTypes.h
    src/Maths/Rect.h
    src/Maths/Vector.h
    src/Maths/Geometry.h
    src/Maths/Parsing.h
    src/Maths/Random.h
    src/Maths/Color.h

    src/stdu/parsing.h
    src/stdu/unique.h
    src/stdu/enum_utils.h
    src/stdu/io.h
    src/stdu/macros.h
    src/stdu/ref.h
    src/stdu/rich_string.h
    src/stdu/sorted_vector.h
    src/stdu/types.h
    src/stdu/lambdas.h

    src/vendor/imgui/imconfig.h
    src/vendor/imgui/imgui.h
    src/vendor/imgui/imgui_impl_glfw.h
    src/vendor/imgui/imgui_impl_opengl3.h
    src/vendor/imgui/imgui_internal.h
    src/vendor/imgui/imstb_rectpack.h
    src/vendor/imgui/imstb_textedit.h
    src/vendor/imgui/imstb_truetype.h
    src/vendor/imgui/imgui_stdlib.h

    src/vendor/stb_image/stb_image.h
        src/Graphics/Utils/Meshes/Quad.h
        src/Graphics/Utils/Meshes/Plane.h
)
source_group("Header Files" FILES ${HEADER_FILES})

set(SOURCE_FILES
    src/Debug/Logger.cpp

    src/Graphics/GLs/FrameBuffer.cpp
    src/Graphics/GLs/GLDebug.cpp
    src/Graphics/GLs/GLObject.cpp
    src/Graphics/GLs/RenderBuffer.cpp
    src/Graphics/GLs/DynamicIndexBuffer.cpp
    src/Graphics/GLs/DynamicVertexBuffer.cpp
    src/Graphics/GLs/VertexArray.cpp
    src/Graphics/GLs/IndexBuffer.cpp
    src/Graphics/GLs/VertexBuffer.cpp
    src/Graphics/GLs/VertexBufferLayout.cpp
    src/Graphics/GLs/Render.cpp
    src/Graphics/GLs/Shader.cpp

    src/Graphics/Graphicals/CameraController.cpp
    src/Graphics/Graphicals/Light.cpp
    src/Graphics/Graphicals/GenericMesh.cpp
    src/Graphics/Graphicals/GraphicsDevice.cpp
    src/Graphics/Graphicals/RenderData.cpp

    src/Graphics/Primitives/Quad.cpp
    src/Graphics/Primitives/Tri.cpp

    src/Graphics/Utils/ModelLoading/MTLMaterialLoader.cpp
    src/Graphics/Utils/ModelLoading/OBJModel.cpp
    src/Graphics/Utils/ModelLoading/OBJModelLoader.cpp
    src/Graphics/Utils/Fonts/Font.cpp
    src/Graphics/Utils/Fonts/FontDevice.cpp
    src/Graphics/Utils/Fonts/TextAlign.cpp
    src/Graphics/Utils/Fonts/TextRenderer.cpp
    src/Graphics/Utils/Textures/Texture.cpp

    src/IO/IO.cpp
    src/IO/Keyboard.cpp
    src/IO/Mouse.cpp
    src/IO/TimeType.cpp

    src/Maths/Vector.cpp
    src/Maths/Matrix.cpp
    src/Maths/Color.cpp

    src/stdu/rich_string.cpp

    src/vendor/imgui/imgui.cpp
    src/vendor/imgui/imgui_demo.cpp
    src/vendor/imgui/imgui_draw.cpp
    src/vendor/imgui/imgui_impl_glfw.cpp
    src/vendor/imgui/imgui_impl_opengl3.cpp
    src/vendor/imgui/imgui_tables.cpp
    src/vendor/imgui/imgui_widgets.cpp
    src/vendor/imgui/imgui_stdlib.cpp

    src/vendor/stb_image/stb_image.cpp
)
source_group("Source Files" FILES ${SOURCE_FILES})

set(ALL_FILES
    ${no_group_source_files}
    ${HEADER_FILES}
    ${SOURCE_FILES}
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
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)

#######################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME}
        PRIVATE
    "${PROJECT_SOURCE_DIR}/Dependencies/GLFW/include/"
    "${PROJECT_SOURCE_DIR}/Dependencies/GLEW/include/"
    ${FREETYPE_INCLUDE_DIRS}
        PUBLIC
    src/
    src/Debug/
    src/Graphics/GLs/
    src/Graphics/Graphicals/
    src/Graphics/Primitives/
    src/Graphics/Utils/
    src/Graphics/Utils/Fonts/
    src/Graphics/Utils/Textures/
    src/Graphics/Utils/ModelLoading/
    src/Maths/
    src/IO/
    src/stdu/
    src/vendor/imgui/
    src/vendor/
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "GLEW_STATIC;"
    "OPENGL_EXPORTS;"
    "UNICODE;"
    "_UNICODE"
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
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /DEBUG;
        /SUBSYSTEM:WINDOWS
    )
endif()

add_custom_command(
    TARGET OpenGL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${PROJECT_SOURCE_DIR}\\bin\\${CMAKE_VS_PLATFORM_NAME}\\$<CONFIG>\\OpenGL.dll"
    "${PROJECT_SOURCE_DIR}\\out\\$<CONFIG>\\${CMAKE_VS_PLATFORM_NAME}\\out\\Testing\\"
)

################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
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

