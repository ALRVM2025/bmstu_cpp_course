message(STATUS "Running tasks/bmstu_unordered_map/CMakeLists.txt")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
get_filename_component(NAME_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# Собираем все .cpp и .h файлы в текущей директории и поддиректориях
file(GLOB_RECURSE SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)

message(STATUS "SOURCES: ${SOURCES}")

add_executable(${NAME_EXECUTABLE} ${SOURCES})

# Добавляем include директории
target_include_directories(${NAME_EXECUTABLE} PUBLIC 
    ${PROJECT_SOURCE_DIR}/tasks/bmstu_abstract_iterator/task_abstract_iterator
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Линкуем с Google Test
target_link_libraries(
        ${NAME_EXECUTABLE}
        GTest::gtest_main
        fast-streebog-lib
)