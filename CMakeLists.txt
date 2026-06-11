cmake_minimum_required(VERSION 3.31)
project(Labo8_1)

set(CMAKE_CXX_STANDARD 20)

add_executable(Labo8_1 main.cpp
                        ast.cpp
                        parser.cpp
                        scanner.cpp
                        token.cpp
                        visitor.cpp)
