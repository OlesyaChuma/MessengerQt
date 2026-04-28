# Юнит-тесты на QTest.
# Тесты на Common требуют только MessengerCommon.
# Тесты на сервер дополнительно линкуют код server-core.

# ----- Code from ChatServer/core (shared with tests) -----
set(SERVER_CORE_DIR ${CMAKE_SOURCE_DIR}/ChatServer/core)
set(SERVER_CORE_SRC
    ${SERVER_CORE_DIR}/Config.h
    ${SERVER_CORE_DIR}/Config.cpp
    ${SERVER_CORE_DIR}/Database.h
    ${SERVER_CORE_DIR}/Database.cpp
    ${SERVER_CORE_DIR}/AuthService.h
    ${SERVER_CORE_DIR}/AuthService.cpp
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

# ----- Хелпер для тестов сервера -----
function(messenger_add_server_test name)
    qt_add_executable(${name} ${name}.cpp ${SERVER_CORE_SRC})
    target_include_directories(${name} PRIVATE ${SERVER_CORE_DIR})
    target_link_libraries(${name} PRIVATE
        MessengerCommon
        Qt6::Core
        Qt6::Sql
        Qt6::Test
    )
    add_test(NAME ${name} COMMAND ${name})

    # Подкладываем SQL-плагины рядом с exe теста
    messenger_deploy_qt_with_sql(${name})
endfunction()

# ----- Тесты Common -----
messenger_add_common_test(test_protocol)
messenger_add_common_test(test_framecodec)
messenger_add_common_test(test_password)

# ----- Тесты сервера (используют SQLite в памяти) -----
messenger_add_server_test(test_database)
messenger_add_server_test(test_auth)
