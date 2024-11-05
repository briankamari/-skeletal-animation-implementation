cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project( skeletal_animation )

set( DXD3_DIR C:/Program\ Files/Microsoft\ DirectX\ SDK\ \(June\ 2010\))

include_directories(
    ${DXD3_DIR}/Include
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/window
    ${PROJECT_SOURCE_DIR}/objects
    ${PROJECT_SOURCE_DIR}/collada

    )
link_directories(
    ${DXD3_DIR}/Lib/x86
    )

set (HEADERS
    appdefs.h
    app.h

    3d_math.h
    window/d3d_window.h
    window/d3d_manager.h
    objects/grayman.h
    )
set (SRCFILES
    main.cpp
    app.cpp

    window/d3d_window.cpp
    window/d3d_manager.cpp
    objects/grayman.cpp
    )
link_libraries( d3d9 d3dx9)
add_executable( skeletal_animation
    ${HEADERS}
    ${SRCFILES}
    )

add_custom_command(
    TARGET skeletal_animation POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/grayman._mesh" "${CMAKE_CURRENT_BINARY_DIR}/grayman._mesh"
    )

add_executable( collada_loader
    collada/tinyxml2.h
    collada/tinyxml2.cpp

    collada/collada.h
    collada/collada.cpp

    collada/colladaapp.cpp

    )

