cmake_minimum_required(VERSION 3.21)
project(MessengerQt LANGUAGES CXX VERSION 1.0.0)

# ---------- Глобальные настройки ----------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Все исполняемые файлы и DLL — в bin/<config>
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Авто-обработка Qt-метатегов
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# UTF-8 в исходниках под MSVC (важно для русских строк)
if(MSVC)
    add_compile_options(/utf-8 /W4 /permissive-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

# ---------- Qt ----------
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Network Sql Test)
qt_standard_project_setup()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(WinDeployQt)

message(STATUS "Qt6 found: ${Qt6_VERSION} at ${Qt6_DIR}")

# ---------- Подпроекты ----------
add_subdirectory(Common)
add_subdirectory(ChatServer)
add_subdirectory(ChatClient)

enable_testing()
add_subdirectory(tests)

# ---------- Установка/деплой ----------
# Копируем пример конфига и SQL-скрипт в каталог бинарников
add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/config
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/config/server.ini.example
        ${CMAKE_BINARY_DIR}/bin/config/server.ini.example
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/sql
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/sql/001_init.sql
        ${CMAKE_BINARY_DIR}/bin/sql/001_init.sql
    COMMENT "Copy config and sql resources to bin/"
)

# ---------- Локализация ----------
# Подключаем модуль LinguistTools для перевода
find_package(Qt6 6.5 QUIET COMPONENTS LinguistTools)

if(Qt6LinguistTools_FOUND)
    message(STATUS "Qt6 LinguistTools found - translations are enabled")
else()
    message(WARNING "Qt6 LinguistTools NOT found - translations will be skipped")
endif()
