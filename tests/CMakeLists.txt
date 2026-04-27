# Юнит-тесты на QTest. Реальные тесты добавим в Пакете 2.

qt_add_executable(test_smoke test_smoke.cpp)

target_link_libraries(test_smoke PRIVATE
    MessengerCommon
    Qt6::Core
    Qt6::Test
)

add_test(NAME test_smoke COMMAND test_smoke)