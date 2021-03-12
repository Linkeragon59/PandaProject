cmake_minimum_required(VERSION 3.16)

add_subdirectory(Base)
add_subdirectory(GameWork)
add_subdirectory(Render)

set_target_properties(Base PROPERTIES FOLDER "Frameworks")
set_target_properties(GameWork PROPERTIES FOLDER "Frameworks")
set_target_properties(Render PROPERTIES FOLDER "Frameworks")
