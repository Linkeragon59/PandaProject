# PandaProject

WIP Todo list : https://docs.google.com/document/d/1zTnrLVwBnSkQkD5-gCt3v7O8c8u-FKoWOFQj0SIL81w/edit

Corentin's WIP doc : https://docs.google.com/document/d/11bgWyhwrEoCtvF0ndbinEszqc-oJ8s50h0C9FBs_dVQ/edit

# Prerequisites
Windows:
- Visual Studio 2019
- CMake

Linux:
- Visual Studio Code (https://code.visualstudio.com/docs/cpp/cmake-linux)
  - Extensions (Ctrl+Shift+X)
    C/C++
    CMake Tools
  - GNU compiler, make, GDB debugger
    Check if already installed with "$ gcc -v", if not, install with:
    - $ sudo apt-get update
    - $ sudo apt-get install build-essential gdb
  - This project uses glm, glfw3 and vulkan, but those are copied under Extern/, so no need to install the dev packages

# How to build
Windows:
- Clone the depot and run cmake to generate a Visual Studio solution
  - git clone https://github.com/Linkeragon59/PandaProject
  - cd <project-location>
  - mkdir build
  - cd build
  - cmake "Visual Studio 16 2019" ../
- Open the VS solution under the build/ folder and build

Linux:
- Clone the depot, open a terminal at its location and open the project with VS Code
  - $ git clone https://github.com/Linkeragon59/PandaProject
  - $ cd <project-location>
  - $ code .
- Choose GCC as your compiler and build

# Troubleshooting
Linux:
- You may have to manually install the drivers for vulkan: https://linuxconfig.org/install-and-test-vulkan-on-linux
- If you run Linux from a virtual machine (e.g. VM Virtual Box), don't forget to enable 3D acceleration in the VM display settings, this is necessary for Vulkan. If not enabled, you may get "vulkan: no dri3 support detected" errors when running a program using vulkan.
