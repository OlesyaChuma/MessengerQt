# Юнит-тесты на QTest.

# ----- Code from ChatServer/core (shared with tests) -----
set(SERVER_CORE_DIR ${CMAKE_SOURCE_DIR}/ChatServer/core)
set(SERVER_CORE_SRC
    ${SERVER_CORE_DIR}/Config.h
    ${SERVER_CORE_DIR}/Config.cpp
    ${SERVER_CORE_DIR}/Database.h
    ${SERVER_CORE_DIR}/Database.cpp
    ${SERVER_CORE_DIR}/AuthService.h
    ${SERVER_CORE_DIR}/AuthService.cpp
    ${SERVER_CORE_DIR}/ClientSession.h
    ${SERVER_CORE_DIR}/ClientSession.cpp
    ${SERVER_CORE_DIR}/ChatServer.h
    ${SERVER_CORE_DIR}/ChatServer.cpp
)

# Code from ChatClient/core
set(CLIENT_CORE_DIR ${CMAKE_SOURCE_DIR}/ChatClient/core)
set(CLIENT_CORE_SRC
    ${CLIENT_CORE_DIR}/ChatClientCore.h
    ${CLIENT_CORE_DIR}/ChatClientCore.cpp
)

# ----- Хелпер для тестов модуля Common -----
function(messenger_add_common_test name)
    qt_add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE
        MessengerCommon
        Qt6::Core
        Qt6::Test
    )
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# ----- Хелпер для тестов сервера (юнит, без сети) -----
function(messenger_add_server_test name)
    qt_add_executable(${name} ${name}.cpp
        ${SERVER_CORE_DIR}/Config.h
        ${SERVER_CORE_DIR}/Config.cpp
        ${SERVER_CORE_DIR}/Database.h
        ${SERVER_CORE_DIR}/Database.cpp
        ${SERVER_CORE_DIR}/AuthService.h
        ${SERVER_CORE_DIR}/AuthService.cpp
    )
    target_include_directories(${name} PRIVATE ${SERVER_CORE_DIR})
    target_link_libraries(${name} PRIVATE
        MessengerCommon
        Qt6::Core
        Qt6::Sql
        Qt6::Test
    )
    add_test(NAME ${name} COMMAND ${name})
    messenger_deploy_qt_with_sql(${name})
endfunction()

# ----- Хелпер для интеграционных тестов (сервер + клиент по сети) -----
function(messenger_add_integration_test name)
    qt_add_executable(${name} ${name}.cpp
        ${SERVER_CORE_SRC}
        ${CLIENT_CORE_SRC}
    )
    target_include_directories(${name} PRIVATE
        ${SERVER_CORE_DIR}
        ${CLIENT_CORE_DIR}
    )
    target_link_libraries(${name} PRIVATE
        MessengerCommon
        Qt6::Core
        Qt6::Network
        Qt6::Sql
        Qt6::Test
    )
    add_test(NAME ${name} COMMAND ${name})
    messenger_deploy_qt_with_sql(${name})
endfunction()

# ----- Тесты Common -----
messenger_add_common_test(test_protocol)
messenger_add_common_test(test_framecodec)
messenger_add_common_test(test_password)

# ----- Тесты сервера (SQLite в памяти) -----
messenger_add_server_test(test_database)
messenger_add_server_test(test_auth)

# ----- Интеграция -----
messenger_add_integration_test(test_clientserver)