
cmake_minimum_required(VERSION 3.20)
project(GLOOPy LANGUAGES C CXX)

add_library(GLOOPy
    src/Window.cpp
    src/Buffer.cpp
    src/VertexArray.cpp
    src/Texture.cpp
    src/Mesh.cpp
    src/Model.cpp
    src/Camera.cpp
    src/Light.cpp
    src/Shader.cpp
    src/Skybox.cpp
    src/Transform.cpp
    "src/FrameTime.cpp"
 "src/HUD.cpp" "include/HUD.h")



target_compile_features(GLOOPy PUBLIC cxx_std_17)

target_include_directories(GLOOPy
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libraries/GLFW/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libraries/glad/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libraries/glm
        ${CMAKE_CURRENT_SOURCE_DIR}/libraries/assimp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libraries/stb
)

find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(ASSIMP REQUIRED)
find_package(glad CONFIG REQUIRED)

target_link_libraries(GLOOPy
    PUBLIC
        OpenGL::GL
        glfw
        glm::glm
        assimp::assimp
        glad::glad
)

add_executable(GLOOPyDemo
    src/GLOOPy.cpp
 "src/HUD.cpp" "include/HUD.h")

target_link_libraries(GLOOPyDemo
    PRIVATE
        GLOOPy
)

file(GLOB_RECURSE GLOOPY_ASSET_FILES
    CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*"
        "${CMAKE_CURRENT_SOURCE_DIR}/images/*"
        "${CMAKE_CURRENT_SOURCE_DIR}/models/*"
)

set_source_files_properties(${GLOOPY_ASSET_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
target_sources(GLOOPyDemo PRIVATE ${GLOOPY_ASSET_FILES})

add_custom_target(GLOOPyAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        "${CMAKE_CURRENT_BINARY_DIR}/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/images"
        "${CMAKE_CURRENT_BINARY_DIR}/images"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/models"
        "${CMAKE_CURRENT_BINARY_DIR}/models"
    COMMENT "Copying shaders, images, and models to the build directory"
    VERBATIM
)
add_dependencies(GLOOPyDemo GLOOPyAssets)

set_target_properties(GLOOPyDemo PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
