set(PROJECT_NAME OpenGLPort)

add_library(${PROJECT_NAME} STATIC glp.h glp.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${PROJECT_SOURCE_DIR}/Dependencies/GLEW/include"
    # "/Library/Developer/CommandLineTools/SDKs/MacOSX13.3.sdk/usr/include"
    .
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    "-framework OpenGL"
    ${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/libglew32.a
)