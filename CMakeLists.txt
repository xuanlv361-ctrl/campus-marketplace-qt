cmake_minimum_required(VERSION 3.16)

project(SecondHandSystem VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
    main.cpp

    languagemanager.h
    languagemanager.cpp

    welcomewindow.h
    welcomewindow.cpp

    userloginwindow.h
    userloginwindow.cpp

    registerwindow.h
    registerwindow.cpp

    userhomewindow.h
    userhomewindow.cpp

    adminloginwindow.h
    adminloginwindow.cpp

    accountstore.h
    accountstore.cpp

    productimagemanager.h
    productimagemanager.cpp

    adminwindow.h
    adminwindow.cpp

    goodswindow.h
    goodswindow.cpp

    orderswindow.h
    orderswindow.cpp

    userswindow.h
    userswindow.cpp

    statswindow.h
    statswindow.cpp

    cartwindow.h
    cartwindow.cpp

    historywindow.h
    historywindow.cpp

    mainwindow.h
    mainwindow.cpp
    mainwindow.ui

)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SecondHandSystem
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(SecondHandSystem
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(SecondHandSystem PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql)

set(MYSQL_ROOT "C:/Program Files/MySQL/MySQL Server 8.0" CACHE PATH
    "Path to the MySQL installation directory")

find_path(MYSQL_INCLUDE_DIR
    NAMES mysql.h
    PATHS "${MYSQL_ROOT}/include"
    PATH_SUFFIXES mysql mariadb
    REQUIRED
)

find_library(MYSQL_CLIENT_LIBRARY
    NAMES libmysql mysqlclient
    PATHS "${MYSQL_ROOT}/lib"
    REQUIRED
)

target_include_directories(SecondHandSystem PRIVATE "${MYSQL_INCLUDE_DIR}")
target_link_libraries(SecondHandSystem PRIVATE "${MYSQL_CLIENT_LIBRARY}")

if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SecondHandSystem)
endif()

set_target_properties(SecondHandSystem PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)

install(TARGETS SecondHandSystem
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_custom_command(TARGET SecondHandSystem POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/ui_assets"
        "$<TARGET_FILE_DIR:SecondHandSystem>/ui_assets"
)


if(WIN32)
    find_file(MYSQL_RUNTIME_DLL
        NAMES libmysql.dll
        PATHS "${MYSQL_ROOT}/lib" "${MYSQL_ROOT}/bin"
    )

    if(MYSQL_RUNTIME_DLL)
        add_custom_command(TARGET SecondHandSystem POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MYSQL_RUNTIME_DLL}"
                $<TARGET_FILE_DIR:SecondHandSystem>
        )
    endif()
endif()
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SecondHandSystem)
endif()
