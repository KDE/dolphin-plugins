find_package(LibKonq REQUIRED)

if(NOT WIN32)
  add_subdirectory(git)
endif(NOT WIN32)
add_subdirectory(svn)
