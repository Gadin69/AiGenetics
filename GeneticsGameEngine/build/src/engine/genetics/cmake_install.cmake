# Install script for directory: C:/Users/Taktix/Desktop/CodingAI/AiProjects/3dGenetics/GeneticsGameEngine/src/engine/genetics

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/GeneticsGameEngine")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Taktix/Desktop/CodingAI/AiProjects/3dGenetics/GeneticsGameEngine/build/src/engine/genetics/genome/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Taktix/Desktop/CodingAI/AiProjects/3dGenetics/GeneticsGameEngine/build/src/engine/genetics/taxonomy/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Taktix/Desktop/CodingAI/AiProjects/3dGenetics/GeneticsGameEngine/build/src/engine/genetics/expression/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("C:/Users/Taktix/Desktop/CodingAI/AiProjects/3dGenetics/GeneticsGameEngine/build/src/engine/genetics/breeding/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/genetics" TYPE DIRECTORY FILES "C:/Users/Taktix/Desktop/CodingAI/AiProjects/3dGenetics/GeneticsGameEngine/src/engine/genetics/" FILES_MATCHING REGEX "/[^/]*\\.h$" REGEX "/cmakelists\\.txt$" EXCLUDE REGEX "/breeding\\/[^/]*\\.cpp$" EXCLUDE REGEX "/expression\\/[^/]*\\.cpp$" EXCLUDE)
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/Taktix/Desktop/CodingAI/AiProjects/3dGenetics/GeneticsGameEngine/build/src/engine/genetics/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
