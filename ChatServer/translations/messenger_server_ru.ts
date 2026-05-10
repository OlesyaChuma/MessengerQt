<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru_RU" sourcelanguage="en_US">
<context>
    <name>QObject</name>
    <message>
        <location filename="../main.cpp" line="67"/>
        <source>Database connection failed</source>
        <translation>Ошибка подключения к базе данных</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="68"/>
        <source>Cannot open Postgres database:
%1

Check Database section in %2 and ensure messenger_db is reachable.</source>
        <translation>Не удалось открыть базу данных Postgres:
%1

Проверьте секцию Database в %2 и убедитесь, что messenger_db доступна.</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="87"/>
        <source>Server start failed</source>
        <translation>Не удалось запустить сервер</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="88"/>
        <source>Cannot start TCP listener on %1:%2:
%3</source>
        <translation>Не удалось запустить TCP-слушатель на %1:%2:
%3</translation>
    </message>
</context>
<context>
    <name>messenger::server::ChatServer</name>
    <message>
        <location filename="../core/ChatServer.cpp" line="33"/>
        <source>Server is listening on %1:%2</source>
        <translation>Сервер слушает на %1:%2</translation>
    </message>
    <message>
        <location filename="../core/ChatServer.cpp" line="123"/>
        <source>User logged in: %1 (id=%2)</source>
        <translation>Пользователь вошёл: %1 (id=%2)</translation>
    </message>
</context>
<context>
    <name>messenger::server::gui::BanDialog</name>
    <message>
        <location filename="../gui/BanDialog.cpp" line="17"/>
        <source>Ban user</source>
        <translation>Забанить пользователя</translation>
    </message>
    <message>
        <location filename="../gui/BanDialog.cpp" line="17"/>
        <source>Kick user</source>
        <translation>Отключить пользователя</translation>
    </message>
    <message>
        <location filename="../gui/BanDialog.cpp" line="24"/>
        <source>Block user &apos;%1&apos; permanently</source>
        <translation>Заблокировать пользователя «%1» навсегда</translation>
    </message>
    <message>
        <location filename="../gui/BanDialog.cpp" line="25"/>
        <source>Disconnect user &apos;%1&apos; (one-time)</source>
        <translation>Отключить пользователя «%1» (одноразово)</translation>
    </message>
    <message>
        <location filename="../gui/BanDialog.cpp" line="30"/>
        <source>The user will not be able to log in until unbanned.</source>
        <translation>Пользователь не сможет войти, пока бан не будет снят.</translation>
    </message>
    <message>
        <location filename="../gui/BanDialog.cpp" line="31"/>
        <source>The user can reconnect immediately, but the current session ends.</source>
        <translation>Пользователь сможет переподключиться сразу, но текущая сессия будет прервана.</translation>
    </message>
    <message>
        <location filename="../gui/BanDialog.cpp" line="40"/>
        <source>Why are you banning this user?</source>
        <translation>Причина блокировки</translation>
    </message>
    <message>
        <location filename="../gui/BanDialog.cpp" line="41"/>
        <source>Optional reason</source>
        <translation>Причина (необязательно)</translation>
    </message>
    <message>
        <location filename="../gui/BanDialog.cpp" line="42"/>
        <source>Reason:</source>
        <translation>Причина:</translation>
    </message>
    <message>
        <location filename="../gui/BanDialog.cpp" line="48"/>
        <source>Ban</source>
        <translation>Забанить</translation>
    </message>
    <message>
        <location filename="../gui/BanDialog.cpp" line="48"/>
        <source>Kick</source>
        <translation>Отключить</translation>
    </message>
    <message>
        <location filename="../gui/BanDialog.cpp" line="61"/>
        <source>Banned by admin</source>
        <translation>Заблокирован администратором</translation>
    </message>
    <message>
        <location filename="../gui/BanDialog.cpp" line="62"/>
        <source>Kicked by admin</source>
        <translation>Отключён администратором</translation>
    </message>
</context>
<context>
    <name>messenger::server::gui::ConnectionLogModel</name>
    <message>
        <location filename="../gui/ConnectionLogModel.cpp" line="149"/>
        <source>Time</source>
        <translation>Время</translation>
    </message>
    <message>
        <location filename="../gui/ConnectionLogModel.cpp" line="150"/>
        <source>Event</source>
        <translation>Событие</translation>
    </message>
    <message>
        <location filename="../gui/ConnectionLogModel.cpp" line="151"/>
        <source>User</source>
        <translation>Пользователь</translation>
    </message>
    <message>
        <location filename="../gui/ConnectionLogModel.cpp" line="152"/>
        <source>IP address</source>
        <translation>IP-адрес</translation>
    </message>
    <message>
        <location filename="../gui/ConnectionLogModel.cpp" line="153"/>
        <source>Details</source>
        <translation>Подробности</translation>
    </message>
</context>
<context>
    <name>messenger::server::gui::ConnectionLogTab</name>
    <message>
        <location filename="../gui/ConnectionLogTab.cpp" line="159"/>
        <source>Category:</source>
        <translation>Категория:</translation>
    </message>
    <message>
        <location filename="../gui/ConnectionLogTab.cpp" line="167"/>
        <source>All events</source>
        <translation>Все события</translation>
    </message>
    <message>
        <location filename="../gui/ConnectionLogTab.cpp" line="168"/>
        <source>Logins only</source>
        <translation>Только входы</translation>
    </message>
    <message>
        <location filename="../gui/ConnectionLogTab.cpp" line="169"/>
        <source>Security events</source>
        <translation>Безопасность</translation>
    </message>
    <message>
        <location filename="../gui/ConnectionLogTab.cpp" line="170"/>
        <source>Sanctions (ban/kick/unban)</source>
        <translation>Санкции (бан/кик/разбан)</translation>
    </message>
    <message>
        <location filename="../gui/ConnectionLogTab.cpp" line="160"/>
        <source>User:</source>
        <translation>Пользователь:</translation>
    </message>
    <message>
        <location filename="../gui/ConnectionLogTab.cpp" line="161"/>
        <source>login or empty</source>
        <translation>логин или пусто</translation>
    </message>
    <message>
        <location filename="../gui/ConnectionLogTab.cpp" line="162"/>
        <source>Apply</source>
        <translation>Применить</translation>
    </message>
    <message>
        <location filename="../gui/ConnectionLogTab.cpp" line="163"/>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="../gui/ConnectionLogTab.cpp" line="109"/>
        <location filename="../gui/ConnectionLogTab.cpp" line="183"/>
        <source>Loaded %1 events</source>
        <translation>Загружено: %1 событий</translation>
    </message>
    <message>
        <location filename="../gui/ConnectionLogTab.cpp" line="148"/>
        <source>Loaded %1 events (live)</source>
        <translation>Загружено: %1 событий (live)</translation>
    </message>
</context>
<context>
    <name>messenger::server::gui::LoginDialog</name>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="17"/>
        <source>MessengerQt Server — Login</source>
        <translation>MessengerQt Server — Вход</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="30"/>
        <source>Sign in as administrator</source>
        <translation>Вход администратора</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="31"/>
        <source>Create the first administrator</source>
        <translation>Создание первого администратора</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="41"/>
        <location filename="../gui/LoginDialog.cpp" line="64"/>
        <source>Sign in</source>
        <translation>Войти</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="56"/>
        <location filename="../gui/LoginDialog.cpp" line="78"/>
        <source>admin</source>
        <translation>admin</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="57"/>
        <location filename="../gui/LoginDialog.cpp" line="79"/>
        <source>Login:</source>
        <translation>Логин:</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="61"/>
        <source>password</source>
        <translation>пароль</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="62"/>
        <location filename="../gui/LoginDialog.cpp" line="87"/>
        <source>Password:</source>
        <translation>Пароль:</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="82"/>
        <source>Administrator</source>
        <translation>Администратор</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="83"/>
        <source>Display name:</source>
        <translation>Отображаемое имя:</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="91"/>
        <source>Repeat:</source>
        <translation>Повтор:</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="93"/>
        <source>Create administrator</source>
        <translation>Создать администратора</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="151"/>
        <source>Login and password cannot be empty.</source>
        <translation>Логин и пароль не должны быть пустыми.</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="161"/>
        <source>This account is blocked.</source>
        <translation>Эта учётная запись заблокирована.</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="162"/>
        <source>Wrong login or password.</source>
        <translation>Неверный логин или пароль.</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="169"/>
        <source>This account is not an administrator.</source>
        <translation>Эта учётная запись не является администратором.</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="185"/>
        <source>All fields are required.</source>
        <translation>Все поля обязательны.</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="189"/>
        <source>Passwords do not match.</source>
        <translation>Пароли не совпадают.</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="193"/>
        <source>Password must be at least 4 characters.</source>
        <translation>Пароль должен быть не короче 4 символов.</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="203"/>
        <source>Login is already taken.</source>
        <translation>Этот логин уже занят.</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="204"/>
        <source>Cannot create administrator. Check the database.</source>
        <translation>Не удалось создать администратора. Проверьте базу данных.</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="209"/>
        <source>Success</source>
        <translation>Готово</translation>
    </message>
    <message>
        <location filename="../gui/LoginDialog.cpp" line="210"/>
        <source>Administrator &apos;%1&apos; created. You are now signed in.</source>
        <translation>Администратор «%1» создан. Вход выполнен.</translation>
    </message>
</context>
<context>
    <name>messenger::server::gui::MainWindow</name>
    <message>
        <location filename="../gui/MainWindow.cpp" line="38"/>
        <location filename="../gui/MainWindow.cpp" line="328"/>
        <source>MessengerQt — Server Administration</source>
        <translation>MessengerQt — Администрирование сервера</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="73"/>
        <location filename="../gui/MainWindow.cpp" line="339"/>
        <source>Users</source>
        <translation>Пользователи</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="74"/>
        <location filename="../gui/MainWindow.cpp" line="340"/>
        <source>Messages</source>
        <translation>Сообщения</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="75"/>
        <location filename="../gui/MainWindow.cpp" line="341"/>
        <source>Connection log</source>
        <translation>Журнал подключений</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="81"/>
        <source>&amp;File</source>
        <translation>&amp;Файл</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="82"/>
        <source>&amp;View</source>
        <translation>&amp;Вид</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="83"/>
        <source>&amp;Help</source>
        <translation>&amp;Справка</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="85"/>
        <source>&amp;Settings...</source>
        <translation>&amp;Настройки...</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="92"/>
        <source>Minimize to &amp;tray</source>
        <translation>Свернуть в &amp;трей</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="96"/>
        <source>E&amp;xit</source>
        <translation>В&amp;ыход</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="103"/>
        <source>Toggle &amp;theme (Light/Dark)</source>
        <translation>Сменить &amp;тему (светлая/тёмная)</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="107"/>
        <source>&amp;Language...</source>
        <translation>&amp;Язык...</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="112"/>
        <source>&amp;Refresh current tab</source>
        <translation>&amp;Обновить вкладку</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="121"/>
        <source>&amp;About...</source>
        <translation>&amp;О программе...</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="129"/>
        <location filename="../gui/MainWindow.cpp" line="331"/>
        <source>DB: connected</source>
        <translation>БД: подключено</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="135"/>
        <location filename="../gui/MainWindow.cpp" line="332"/>
        <source>Server: starting...</source>
        <translation>Сервер: запуск...</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="138"/>
        <source>Server is listening on %1:%2</source>
        <translation type="unfinished">Сервер слушает на %1:%2</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="145"/>
        <source>Online: 0</source>
        <translation>Онлайн: 0</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="149"/>
        <location filename="../gui/MainWindow.cpp" line="329"/>
        <source>Signed in as: %1</source>
        <translation>Вход выполнен: %1</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="184"/>
        <source>About MessengerQt Server</source>
        <translation>О программе MessengerQt Server</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="185"/>
        <source>&lt;h3&gt;MessengerQt — Server Administration&lt;/h3&gt;&lt;p&gt;Version %1&lt;/p&gt;&lt;p&gt;Multi-user network messenger.&lt;br&gt;Final qualifying project — C++ developer programme.&lt;/p&gt;&lt;p&gt;Built with Qt 6.5, PostgreSQL, MSVC 2022.&lt;/p&gt;</source>
        <translation>&lt;h3&gt;MessengerQt — Администрирование сервера&lt;/h3&gt;&lt;p&gt;Версия %1&lt;/p&gt;&lt;p&gt;Многопользовательский сетевой мессенджер.&lt;br&gt;Итоговая аттестация по программе «C++ разработчик».&lt;/p&gt;&lt;p&gt;Собран на Qt 6.5, PostgreSQL, MSVC 2022.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="203"/>
        <source>New connection from %1</source>
        <translation>Новое подключение с %1</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="207"/>
        <source>User logged in: %1 (id=%2)</source>
        <translation>Вход пользователя: %1 (id=%2)</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="210"/>
        <source>User connected</source>
        <translation>Пользователь подключился</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="211"/>
        <source>%1 logged in</source>
        <translation>%1 вошёл</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="218"/>
        <source>User logged out: %1</source>
        <translation>Выход пользователя: %1</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="229"/>
        <source>Online: %1</source>
        <translation>Онлайн: %1</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="252"/>
        <source>Close server</source>
        <translation>Закрыть сервер</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="254"/>
        <source>&lt;b&gt;Close MessengerQt Server?&lt;/b&gt;</source>
        <translation>&lt;b&gt;Закрыть MessengerQt Server?&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="255"/>
        <source>If you choose &lt;b&gt;Quit&lt;/b&gt;, the server will stop and all connected clients will be disconnected.&lt;br&gt;&lt;br&gt;If you choose &lt;b&gt;Hide to tray&lt;/b&gt;, the server keeps running in the background and you can restore the window from the system tray icon.</source>
        <translation>Если выбрать &lt;b&gt;Остановить&lt;/b&gt;, сервер прекратит работу и все клиенты будут отключены.&lt;br&gt;&lt;br&gt;Если выбрать &lt;b&gt;Скрыть в трей&lt;/b&gt;, сервер продолжит работу в фоне, окно можно вернуть через иконку в трее.</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="262"/>
        <source>Quit server</source>
        <translation>Остановить сервер</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="264"/>
        <source>Hide to tray</source>
        <translation>Скрыть в трей</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="269"/>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="312"/>
        <source>Select language</source>
        <translation>Выбор языка</translation>
    </message>
    <message>
        <location filename="../gui/MainWindow.cpp" line="313"/>
        <source>Interface language:</source>
        <translation>Язык интерфейса:</translation>
    </message>
</context>
<context>
    <name>messenger::server::gui::MessagesModel</name>
    <message>
        <location filename="../gui/MessagesModel.cpp" line="110"/>
        <source>(broadcast)</source>
        <translation>(общий чат)</translation>
    </message>
    <message>
        <location filename="../gui/MessagesModel.cpp" line="116"/>
        <source>edited</source>
        <translation>изменено</translation>
    </message>
    <message>
        <location filename="../gui/MessagesModel.cpp" line="117"/>
        <source>deleted</source>
        <translation>удалено</translation>
    </message>
    <message>
        <location filename="../gui/MessagesModel.cpp" line="151"/>
        <source>Time</source>
        <translation>Время</translation>
    </message>
    <message>
        <location filename="../gui/MessagesModel.cpp" line="152"/>
        <source>From</source>
        <translation>От</translation>
    </message>
    <message>
        <location filename="../gui/MessagesModel.cpp" line="153"/>
        <source>To / Type</source>
        <translation>Кому / Тип</translation>
    </message>
    <message>
        <location filename="../gui/MessagesModel.cpp" line="154"/>
        <source>Body</source>
        <translation>Текст</translation>
    </message>
    <message>
        <location filename="../gui/MessagesModel.cpp" line="155"/>
        <source>Flags</source>
        <translation>Флаги</translation>
    </message>
</context>
<context>
    <name>messenger::server::gui::MessagesTab</name>
    <message>
        <location filename="../gui/MessagesTab.cpp" line="180"/>
        <source>Type:</source>
        <translation>Тип:</translation>
    </message>
    <message>
        <location filename="../gui/MessagesTab.cpp" line="189"/>
        <source>All</source>
        <translation>Все</translation>
    </message>
    <message>
        <location filename="../gui/MessagesTab.cpp" line="190"/>
        <source>Broadcast only</source>
        <translation>Только общий чат</translation>
    </message>
    <message>
        <location filename="../gui/MessagesTab.cpp" line="191"/>
        <source>Private only</source>
        <translation>Только приватные</translation>
    </message>
    <message>
        <location filename="../gui/MessagesTab.cpp" line="181"/>
        <source>Sender id:</source>
        <translation>ID отправителя:</translation>
    </message>
    <message>
        <location filename="../gui/MessagesTab.cpp" line="182"/>
        <source>0 = all</source>
        <translation>0 = все</translation>
    </message>
    <message>
        <location filename="../gui/MessagesTab.cpp" line="183"/>
        <source>Apply</source>
        <translation>Применить</translation>
    </message>
    <message>
        <location filename="../gui/MessagesTab.cpp" line="184"/>
        <source>Load older</source>
        <translation>Загрузить старые</translation>
    </message>
    <message>
        <location filename="../gui/MessagesTab.cpp" line="185"/>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="../gui/MessagesTab.cpp" line="118"/>
        <location filename="../gui/MessagesTab.cpp" line="204"/>
        <source>Loaded %1 messages</source>
        <translation>Загружено: %1 сообщ.</translation>
    </message>
    <message>
        <location filename="../gui/MessagesTab.cpp" line="130"/>
        <source>No more older messages. Total: %1</source>
        <translation>Старее нет. Всего: %1</translation>
    </message>
    <message>
        <location filename="../gui/MessagesTab.cpp" line="133"/>
        <source>Loaded %1 more (total: %2)</source>
        <translation>Подгружено ещё %1 (всего: %2)</translation>
    </message>
    <message>
        <location filename="../gui/MessagesTab.cpp" line="156"/>
        <source>Loaded %1 messages (last: live update)</source>
        <translation>Загружено %1 сообщ. (последнее: live)</translation>
    </message>
    <message>
        <location filename="../gui/MessagesTab.cpp" line="162"/>
        <source>Loaded %1 messages (last: edit)</source>
        <translation>Загружено %1 сообщ. (последнее: изменение)</translation>
    </message>
    <message>
        <location filename="../gui/MessagesTab.cpp" line="168"/>
        <source>Loaded %1 messages (last: delete)</source>
        <translation>Загружено %1 сообщ. (последнее: удаление)</translation>
    </message>
</context>
<context>
    <name>messenger::server::gui::SettingsDialog</name>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="15"/>
        <source>Server settings</source>
        <translation>Настройки сервера</translation>
    </message>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="23"/>
        <source>Settings are loaded from: &lt;b&gt;%1&lt;/b&gt;&lt;br/&gt;To change values, edit this file and restart the server.</source>
        <translation>Настройки загружены из: &lt;b&gt;%1&lt;/b&gt;&lt;br/&gt;Чтобы изменить значения, отредактируйте файл и перезапустите сервер.</translation>
    </message>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="39"/>
        <source>Database</source>
        <translation>База данных</translation>
    </message>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="42"/>
        <source>Host:</source>
        <translation>Хост:</translation>
    </message>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="43"/>
        <source>Port:</source>
        <translation>Порт:</translation>
    </message>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="44"/>
        <source>Database:</source>
        <translation>База:</translation>
    </message>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="45"/>
        <source>User:</source>
        <translation>Пользователь:</translation>
    </message>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="46"/>
        <source>Password:</source>
        <translation>Пароль:</translation>
    </message>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="50"/>
        <source>Server</source>
        <translation>Сервер</translation>
    </message>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="53"/>
        <source>Listen host:</source>
        <translation>Хост прослушивания:</translation>
    </message>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="54"/>
        <source>Listen port:</source>
        <translation>Порт прослушивания:</translation>
    </message>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="55"/>
        <source>Max attachment (MB):</source>
        <translation>Макс. вложение (МБ):</translation>
    </message>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="60"/>
        <source>Logging</source>
        <translation>Логирование</translation>
    </message>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="63"/>
        <source>Log file:</source>
        <translation>Файл лога:</translation>
    </message>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="64"/>
        <source>Log level:</source>
        <translation>Уровень логирования:</translation>
    </message>
    <message>
        <location filename="../gui/SettingsDialog.cpp" line="68"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
</context>
<context>
    <name>messenger::server::gui::TrayIcon</name>
    <message>
        <location filename="../gui/TrayIcon.cpp" line="24"/>
        <source>Show window</source>
        <translation>Показать окно</translation>
    </message>
    <message>
        <location filename="../gui/TrayIcon.cpp" line="25"/>
        <source>Hide window</source>
        <translation>Скрыть окно</translation>
    </message>
    <message>
        <location filename="../gui/TrayIcon.cpp" line="27"/>
        <source>About...</source>
        <translation>О программе...</translation>
    </message>
    <message>
        <location filename="../gui/TrayIcon.cpp" line="29"/>
        <source>Quit</source>
        <translation>Выход</translation>
    </message>
    <message>
        <location filename="../gui/TrayIcon.cpp" line="40"/>
        <source>MessengerQt Server
Online: %1</source>
        <translation>MessengerQt Server
Онлайн: %1</translation>
    </message>
</context>
<context>
    <name>messenger::server::gui::UsersModel</name>
    <message>
        <location filename="../gui/UsersModel.cpp" line="75"/>
        <source>online</source>
        <translation>в сети</translation>
    </message>
    <message>
        <location filename="../gui/UsersModel.cpp" line="75"/>
        <source>offline</source>
        <translation>не в сети</translation>
    </message>
    <message>
        <location filename="../gui/UsersModel.cpp" line="78"/>
        <source>yes</source>
        <translation>да</translation>
    </message>
    <message>
        <location filename="../gui/UsersModel.cpp" line="79"/>
        <source>yes (%1)</source>
        <translation>да (%1)</translation>
    </message>
    <message>
        <location filename="../gui/UsersModel.cpp" line="80"/>
        <source>no</source>
        <translation>нет</translation>
    </message>
    <message>
        <location filename="../gui/UsersModel.cpp" line="119"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../gui/UsersModel.cpp" line="120"/>
        <source>Login</source>
        <translation>Логин</translation>
    </message>
    <message>
        <location filename="../gui/UsersModel.cpp" line="121"/>
        <source>Display name</source>
        <translation>Отображаемое имя</translation>
    </message>
    <message>
        <location filename="../gui/UsersModel.cpp" line="122"/>
        <source>Role</source>
        <translation>Роль</translation>
    </message>
    <message>
        <location filename="../gui/UsersModel.cpp" line="123"/>
        <source>Status</source>
        <translation>Статус</translation>
    </message>
    <message>
        <location filename="../gui/UsersModel.cpp" line="124"/>
        <source>Banned</source>
        <translation>Забанен</translation>
    </message>
    <message>
        <location filename="../gui/UsersModel.cpp" line="125"/>
        <source>Last seen</source>
        <translation>Последний визит</translation>
    </message>
</context>
<context>
    <name>messenger::server::gui::UsersTab</name>
    <message>
        <location filename="../gui/UsersTab.cpp" line="226"/>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="227"/>
        <source>Kick</source>
        <translation>Отключить</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="228"/>
        <source>Ban</source>
        <translation>Забанить</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="229"/>
        <source>Unban</source>
        <translation>Снять бан</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="113"/>
        <source>Total users: %1   |   Online: %2   |   Banned: %3</source>
        <translation>Всего: %1   |   Онлайн: %2   |   Забанено: %3</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="178"/>
        <source>Kicked</source>
        <translation>Отключён</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="179"/>
        <source>User &apos;%1&apos; has been disconnected.</source>
        <translation>Пользователь «%1» отключён.</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="181"/>
        <source>Kick failed</source>
        <translation>Ошибка отключения</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="182"/>
        <source>Cannot kick &apos;%1&apos; (perhaps user is offline?).</source>
        <translation>Не удалось отключить «%1» (возможно, не в сети).</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="193"/>
        <source>Banned</source>
        <translation>Забанен</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="194"/>
        <source>User &apos;%1&apos; is now blocked.</source>
        <translation>Пользователь «%1» заблокирован.</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="196"/>
        <source>Ban failed</source>
        <translation>Ошибка бана</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="197"/>
        <source>Cannot ban &apos;%1&apos;.</source>
        <translation>Не удалось забанить «%1».</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="205"/>
        <source>Unban user</source>
        <translation>Снять бан</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="206"/>
        <source>Remove ban from &apos;%1&apos;?</source>
        <translation>Снять бан с «%1»?</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="209"/>
        <source>Unbanned</source>
        <translation>Бан снят</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="210"/>
        <source>User &apos;%1&apos; is unblocked.</source>
        <translation>Пользователь «%1» разблокирован.</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="212"/>
        <source>Unban failed</source>
        <translation>Ошибка снятия бана</translation>
    </message>
    <message>
        <location filename="../gui/UsersTab.cpp" line="213"/>
        <source>Cannot unban &apos;%1&apos;.</source>
        <translation>Не удалось снять бан с «%1».</translation>
    </message>
</context>
</TS>
