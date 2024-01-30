# CMakeList.txt: проект CMake для task1; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project ("Spider")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Spider "Spider.cpp")
add_subdirectory(D:/GIT/FQW/libpqxx libpqxx-build)
target_compile_features ( Spider PRIVATE cxx_std_17)
target_link_libraries(Spider  pqxx)
# TODO: Добавьте тесты и целевые объекты, если это необходимо.
