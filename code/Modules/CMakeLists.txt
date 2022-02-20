cmake_minimum_required(VERSION 3.16)

add_subdirectory(Editor)
set_target_properties(Editor PROPERTIES FOLDER "Modules")

add_subdirectory(Render)
set_target_properties(Render PROPERTIES FOLDER "Modules")
