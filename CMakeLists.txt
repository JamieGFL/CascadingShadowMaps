cmake_minimum_required(VERSION 3.3)
project(cascaded_shadow_mapping)


set(CMAKE_CXX_STANDARD 20)

# Add libraries
add_subdirectory(lib/glad)
add_subdirectory(lib/glfw)
add_subdirectory(lib/glm)
add_subdirectory(lib/assimp)


add_executable(cascadedShadowMaps lib/stb_image/stb_image.cpp src/presentation/pres_cascadingShadowMaps.cpp)
add_executable(shadow_mapping lib/stb_image/stb_image.cpp src/presentation/pres_shadowMapping.cpp)
# Include the header files
include_directories(lib/glad/include)
include_directories(lib/glfw/include)
include_directories(lib//glm/glm)
include_directories(lib/stb_image)
include_directories(lib/modelLoad)
include_directories(lib/camera)
include_directories(res)

# Link with libraries
target_link_libraries(cascadedShadowMaps glad glfw glm assimp)
target_link_libraries(shadow_mapping glad glfw glm assimp)
