#构建目标
get_filename_component(BUILD_TARGET_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_executable(${BUILD_TARGET_NAME})

#目标包含目录
target_include_directories(${BUILD_TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
#目标源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src PRIVATE_SRC)
target_sources(${BUILD_TARGET_NAME} PRIVATE ${PRIVATE_SRC})

#SDL2
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${BUILD_TARGET_NAME}
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
#Volk
find_package(volk CONFIG REQUIRED)
target_link_libraries(${BUILD_TARGET_NAME} PRIVATE volk::volk volk::volk_headers)
#GLM
find_package(glm CONFIG REQUIRED)
target_link_libraries(${BUILD_TARGET_NAME} PRIVATE glm::glm)
#ImGUI
find_package(imgui CONFIG REQUIRED)
target_link_libraries(${BUILD_TARGET_NAME} PRIVATE imgui::imgui)
#STB
find_package(Stb REQUIRED)
target_include_directories(${BUILD_TARGET_NAME} PRIVATE ${Stb_INCLUDE_DIR})
#Assimp
find_package(assimp CONFIG REQUIRED)
target_link_libraries(${BUILD_TARGET_NAME} PRIVATE assimp::assimp)
