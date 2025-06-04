cmake_minimum_required(VERSION 3.10.0)
project(project VERSION 0.1.0 LANGUAGES C CXX)

add_executable(project csv_reader.cpp ler-colunas.cpp lista_dupla_ligada.cpp avl_tree.cpp kd_tree.cpp classification.cpp operations_menu.cpp benchmark.cpp main.cpp)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
