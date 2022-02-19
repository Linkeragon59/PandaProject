cmake_minimum_required(VERSION 3.16)

add_subdirectory(Base)
set_target_properties(Base PROPERTIES FOLDER "Frameworks")

add_subdirectory(GameCore)
set_target_properties(GameCore PROPERTIES FOLDER "Frameworks")

add_subdirectory(Render)
set_target_properties(Render PROPERTIES FOLDER "Frameworks")
