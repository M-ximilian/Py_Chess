cmake_minimum_required(VERSION 3.21)
project(Py_Chess)

set(CMAKE_CXX_STANDARD 23)

add_executable(Py_Chess main.cpp Board.cpp Board.h Piece.cpp Piece.h main.h)
