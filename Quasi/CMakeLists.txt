set(PROJECT_NAME Quasi)

set(HEADER_FILES
    src/Debug/ConsoleColor.h
    src/Debug/internal_debug_break.h
    src/Debug/Logger.h
    src/Debug/Timer.h

    src/Graphics/GLs/IndexBuffer.h
    src/Graphics/GLs/VertexBuffer.h
    src/Graphics/GLs/Render.h
    src/Graphics/GLs/Shader.h
    src/Graphics/GLs/VertexArray.h
    src/Graphics/GLs/VertexBufferLayout.h
    src/Graphics/GLs/VertexElement.h
    src/Graphics/GLs/FrameBuffer.h
    src/Graphics/GLs/GLDebug.h
    src/Graphics/GLs/GLObject.h
    src/Graphics/GLs/GLTypeID.h
    src/Graphics/GLs/RenderBuffer.h
    src/Graphics/GLs/VertexBlueprint.h

    src/Graphics/Graphicals/GraphicsDevice.h
    src/Graphics/Graphicals/Mesh.h
    src/Graphics/Graphicals/Mesh.tpp
    src/Graphics/Graphicals/RenderData.h
    src/Graphics/Graphicals/RenderObject.h
    src/Graphics/Graphicals/TriIndices.h
    src/Graphics/Graphicals/CameraController.h
    src/Graphics/Graphicals/Light.h

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
    src/Graphics/Utils/Meshes/Quad.h
    src/Graphics/Utils/Meshes/Plane.h
    src/Graphics/Utils/Meshes/Capsule.h
    src/Graphics/Utils/Meshes/Stadium.h
    src/Graphics/Utils/Extension/ImGuiExt.h

    src/IO/IO.h
    src/IO/Keyboard.h
    src/IO/Mouse.h
    src/IO/TimeType.h

    src/Math/Constants.h
    src/Math/Matrix.h
    src/Math/Rect.h
    src/Math/Vector.h
    src/Math/Geometry.h
    src/Math/Random.h
    src/Math/Color.h
    src/Math/Complex.h
    src/Math/Quaternion.h
    src/Math/Transform2D.h
    src/Math/Transform3D.h

    src/Physics/World2D.h
    src/Physics/Shape2D.h
    src/Physics/Body2D.h
    src/Physics/Collision2D.h
    src/Physics/PhysicsTransform2D.h
    src/Physics/Manifold2D.h
    src/Physics/SeperatingAxisSolver.h
    src/Physics/CircleShape2D.h
    src/Physics/CapsuleShape2D.h
    src/Physics/PolygonShape2D.h
    src/Physics/RectShape2D.h

    src/Utils/Enum.h
    src/Utils/Text.h
    src/Utils/Option.h
    src/Utils/Macros.h
    src/Utils/RichString.h
    src/Utils/Type.h
    src/Utils/ArenaAllocator.h
    src/Utils/Match.h
    src/Utils/Format.h
    src/Utils/Memory.h

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
)
source_group("Header Files" FILES ${HEADER_FILES})

set(SOURCE_FILES
    src/Debug/Logger.cpp
    src/Debug/Timer.cpp

    src/Graphics/GLs/FrameBuffer.cpp
    src/Graphics/GLs/GLDebug.cpp
    src/Graphics/GLs/RenderBuffer.cpp
    src/Graphics/GLs/IndexBuffer.cpp
    src/Graphics/GLs/VertexBuffer.cpp
    src/Graphics/GLs/VertexArray.cpp
    src/Graphics/GLs/VertexBufferLayout.cpp
    src/Graphics/GLs/Render.cpp
    src/Graphics/GLs/Shader.cpp

    src/Graphics/Graphicals/CameraController.cpp
    src/Graphics/Graphicals/Light.cpp
    src/Graphics/Graphicals/GraphicsDevice.cpp
    src/Graphics/Graphicals/RenderData.cpp

    src/Graphics/Utils/ModelLoading/MTLMaterialLoader.cpp
    src/Graphics/Utils/ModelLoading/OBJModel.cpp
    src/Graphics/Utils/ModelLoading/OBJModelLoader.cpp
    src/Graphics/Utils/Fonts/Font.cpp
    src/Graphics/Utils/Fonts/FontDevice.cpp
    src/Graphics/Utils/Fonts/TextAlign.cpp
    src/Graphics/Utils/Fonts/TextRenderer.cpp
    src/Graphics/Utils/Textures/Texture.cpp
    src/Graphics/Utils/Extension/ImGuiExt.cpp

    src/IO/IO.cpp
    src/IO/Keyboard.cpp
    src/IO/Mouse.cpp
    src/IO/TimeType.cpp

    src/Math/Vector.cpp
    src/Math/Matrix.cpp
    src/Math/Color.cpp
    src/Math/Complex.cpp
    src/Math/Quaternion.cpp
    src/Math/Transform2D.cpp
    src/Math/Transform3D.cpp

    src/Physics/World2D.cpp
    src/Physics/Shape2D.cpp
    src/Physics/Body2D.cpp
    src/Physics/Collision2D.cpp
    src/Physics/PhysicsTransform2D.cpp
    src/Physics/Manifold2D.cpp
    src/Physics/SeperatingAxisSolver.cpp
    src/Physics/CircleShape2D.cpp
    src/Physics/CapsuleShape2D.cpp
    src/Physics/PolygonShape2D.cpp
    src/Physics/RectShape2D.cpp

    src/Utils/RichString.cpp
    src/Utils/StringList.cpp
    src/Utils/Text.cpp
    src/Utils/ArenaAllocator.cpp
    src/Utils/Format.cpp

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
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

target_include_directories(${PROJECT_NAME}
        PRIVATE
    ${PROJECT_SOURCE_DIR}/Dependencies/GLFW/include/
    # /Library/Developer/CommandLineTools/SDKs/MacOSX13.3.sdk/System/Library/Frameworks/
        PUBLIC
    src/
    src/Debug/
    src/Graphics/GLs/
    src/Graphics/Graphicals/
    src/Graphics/Utils/
    src/Math/
    src/IO/
    src/Utils/
    src/vendor/imgui/
    src/vendor/
    src/vendor/freetype/
    ${PROJECT_SOURCE_DIR}/OpenGLPort/
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    Q_EXT_MATCH_SYNTAX # for cool syntax features for pattern matching
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    OpenGLPort
    # opengl32.dll
    ${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-mingw-w64/libglfw3.a
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/freetype/libfreetype.a
)
# execute_process()