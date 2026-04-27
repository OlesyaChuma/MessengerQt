# Юнит-тесты на QTest для модуля Common.

function(messenger_add_test name)
    qt_add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE
        MessengerCommon
        Qt6::Core
        Qt6::Test
    )
    add_test(NAME ${name} COMMAND ${name})
endfunction()

messenger_add_test(test_protocol)
messenger_add_test(test_framecodec)
messenger_add_test(test_password)
