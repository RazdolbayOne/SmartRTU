cmake_minimum_required(VERSION 3.0.0)

##add libs
add_subdirectory(json)
add_subdirectory(fonts)
add_subdirectory(pictures)


#create library called BoardLib
add_library(BoardLib Dummy.cpp)

#linking BoardPic.lib + json.lib + TFont.lib = Public BoardPic.lib
target_link_libraries(BoardLib PUBLIC BoardPic json TFont)
