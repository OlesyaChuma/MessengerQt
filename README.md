# MessengerQt

Сетевой мессенджер с клиент-серверной архитектурой на Qt 6 и PostgreSQL.
Итоговая аттестация по программе **«Профессия C++ разработчик»**.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![Qt 6.5](https://img.shields.io/badge/Qt-6.5-green)
![PostgreSQL](https://img.shields.io/badge/PostgreSQL-14%2B-blue)
![CMake](https://img.shields.io/badge/CMake-3.21%2B-orange)
![License: MIT](https://img.shields.io/badge/license-MIT-green)

---

## Содержание

- [Технологический стек](#технологический-стек)
- [Возможности](#возможности)
- [Скриншоты](#скриншоты)
- [Структура проекта](#структура-проекта)
- [Установка и сборка](#установка-и-сборка)
- [Запуск](#запуск)
- [Сетевой протокол](#сетевой-протокол)
- [Тестирование](#тестирование)
- [Документация](#документация)
- [Атрибуции](#атрибуции)
- [Лицензия](#лицензия)
- [Автор](#автор)

---

## Технологический стек

| Компонент | Технология |
|-----------|------------|
| Язык | C++17 |
| GUI-фреймворк | Qt 6.5 (Widgets) |
| База данных | PostgreSQL 14+ |
| Сборочная система | CMake 3.21+ |
| Компилятор | MSVC 2022 (x64) |
| IDE | Qt Creator |
| Тестирование | QTest |
| Шифрование паролей | SHA-256 + случайная 16-байтовая соль |
| Локализация | Qt Linguist (RU / EN) |

---

## Возможности

### Серверная часть

- **GUI администратора** с тремя вкладками: «Пользователи», «Сообщения», «Журнал подключений»
- **Управление пользователями**: отключение (kick) и блокировка (ban) с указанием причины
- **Просмотр всех сообщений**, включая приватные, с фильтрами по типу и отправителю
- **Live-обновление** ленты сообщений: новые/отредактированные/удалённые сообщения отображаются мгновенно
- **Журнал событий**: вход, выход, неудачные попытки авторизации, бан/разбан с фильтром по категориям
- **Системный трей**: иконка с уведомлениями о подключениях, диалог при закрытии (Quit / Hide to tray / Cancel)
- **Светлая и тёмная темы** оформления (Ctrl+T)
- **Локализация** интерфейса на русский и английский (View → Language)
- **Хранение пароля админа** в `config/server.ini`, не в коде

### Клиентская часть

- **Регистрация и вход** с проверкой подлинности через SHA-256 + соль
- **Запоминание логина** и параметров сервера через `QSettings`
- **Боковая панель пользователей** с круглыми аватарками (генерация инициалов и стабильного цвета по логину)
- **Online/offline индикаторы** в реальном времени
- **Поиск пользователей** по логину и имени
- **Чат с пузырями сообщений**: свои справа голубые, чужие слева серые, имя отправителя и время
- **# Общий чат** + приватные диалоги с любым пользователем
- **История сообщений** с автоскроллом и постраничной подгрузкой
- **Редактирование и удаление** своих сообщений с подтверждением и live-обновлением у всех собеседников
- **Эмодзи picker на основе [OpenMoji](https://openmoji.org/)** — 80 эмодзи в 5 категориях, единый вид у всех
- **Уведомления о новых сообщениях**: мигание иконки в панели задач, системный звук, счётчик непрочитанных в заголовке окна
- **Светлая и тёмная темы** оформления (Ctrl+T)
- **Локализация** интерфейса на русский и английский

---

## Скриншоты

### Клиент

| | |
|---|---|
| ![Окно входа](docs/screenshots/client/ru/01_login_light_ru.png) | ![Главное окно](docs/screenshots/client/ru/03_main_light_ru.png) |
| Окно входа клиента (светлая тема) | Главное окно — список пользователей и чат |
| ![Тёмная тема](docs/screenshots/client/en/04_main_dark_en.png) | ![Меню Вид](docs/screenshots/client/ru/08_menu_view_dark_ru.png) |
| Тёмная тема оформления | Меню «Вид» — переключение тем и языков |
| ![Поиск пользователей](docs/screenshots/client/en/05_search_dark_en.png) | ![Уведомление о сообщении](docs/screenshots/client/en/15_flashing_message.png) |
| Поиск пользователей | Уведомление в панели задач |
| ![Редактирование](docs/screenshots/client/ru/13_edit_light_ru.png) | ![Удаление](docs/screenshots/client/ru/16_delete_message_light_ru.png) |
| Редактирование сообщения | Подтверждение удаления |
| ![Статус edited/deleted](docs/screenshots/client/en/17_del_edit_status_dark_en.png) | ![Picker эмодзи](docs/screenshots/client/ru/18_emoji_light_ru.png) |
| Статус «изменено» и «удалено» | Эмодзи picker (OpenMoji) |
| ![Эмодзи в чате](docs/screenshots/client/ru/20_emoji_wind_light_ru.png) | ![О программе](docs/screenshots/client/ru/09_reference_dark_ru.png) |
| Эмодзи в окне чата | Диалог «О программе» с атрибуцией OpenMoji |

### Сервер

| | |
|---|---|
| ![Окно входа](docs/screenshots/server/ru/14_login_dialog_server_ru.png) | ![Меню Вид](docs/screenshots/server/ru/24_view_ru.png) |
| Окно входа администратора | Меню «Вид» |
| ![Пользователи светлая](docs/screenshots/server/ru/15_users_light_ru.png) | ![Пользователи тёмная](docs/screenshots/server/ru/16_users_dark_ru.png) |
| Список пользователей (светлая тема) | Список пользователей (тёмная тема) |
| ![Сообщения](docs/screenshots/server/ru/17_messages_light_ru.png) | ![Сообщения тёмная](docs/screenshots/server/ru/18_messages_dark_ru.png) |
| Все сообщения, в том числе приватные | Сообщения (тёмная тема) |
| ![Статус изменено/удалено](docs/screenshots/server/ru/26_flags_light_ru.png) | ![Журнал подключений](docs/screenshots/server/ru/19_connection_log_light_ru.png) |
| Колонка «Статус»: изменено/удалено | Журнал подключений |
| ![Журнал тёмная](docs/screenshots/server/ru/20_connection_log_dark_ru.png) | ![Настройки](docs/screenshots/server/ru/21_settings_dialog_ru.png) |
| Журнал подключений (тёмная тема) | Диалог настроек |
| ![Системный трей](docs/screenshots/server/en/09_system_tray_en.png) | ![О программе](docs/screenshots/server/ru/23_about_ru.png) |
| Системный трей с уведомлениями | Диалог «О программе» |

---

## Структура проекта
```
MessengerQt/
├── Common/              общие модели, протокол, кодек фреймов, хеширование
├── ChatServer/          серверное приложение
│   ├── core/            ядро: Database, AuthService, ChatServer, ClientSession
│   ├── gui/             GUI: вкладки, диалоги, меню, темы, переводы
│   ├── resources/       QSS-темы, иконки
│   └── translations/    .ts / .qm для RU/EN
├── ChatClient/          клиентское приложение
│   ├── core/            ChatClientCore (сетевое ядро без UI)
│   ├── gui/             LoginWindow, MainWindow, UsersPanel, ChatPanel,
│   │                    MessageBubble, MessagesView, ChatInputWidget,
│   │                    EmojiPicker, AvatarRenderer, ThemeManager,
│   │                    TranslationManager
│   ├── resources/       темы, иконки, эмодзи OpenMoji (80 PNG)
│   └── translations/    .ts / .qm для RU/EN
├── tests/               6 наборов QTest: Common, Database, Auth,
│                        Protocol, FrameCodec, ClientServer (integration)
├── sql/                 001_init.sql — схема PostgreSQL
├── cmake/               WinDeployQt.cmake — деплой Qt-зависимостей
├── config/              server.ini.example
├── docs/                документация и скриншоты
└── update_translations.bat   batch для lupdate + lrelease           техническая документация
```

---

## Установка и сборка

### Требования

- **Qt 6.5+** (Core, Gui, Widgets, Network, Sql, Test) — установить через Qt Online Installer
- **PostgreSQL 14+** локально или удалённо
- **MSVC 2022** (Visual Studio Build Tools)
- **CMake 3.21+**
- **Qt Creator** (рекомендуется) или CLion

### Подготовка базы данных

1. Создайте пользователя и БД в PostgreSQL:

```sql
CREATE USER messenger_user WITH PASSWORD 'messenger_pass_2026';
CREATE DATABASE messenger_db OWNER messenger_user
    ENCODING 'UTF8' TEMPLATE template0;
GRANT ALL PRIVILEGES ON DATABASE messenger_db TO messenger_user;
```

2. Примените схему:

```bash
psql -U messenger_user -d messenger_db -f sql/001_init.sql
```

или откройте `sql/001_init.sql` в DBeaver и выполните Alt+X.

### Сборка через Qt Creator

1. **File → Open File or Project** — выберите `CMakeLists.txt` в корне проекта.
2. Выберите kit: **Desktop Qt 6.5.x MSVC2019 64bit**.
3. Дождитесь конфигурации CMake (статус внизу).
4. **Build → Build All Projects** (Ctrl+B).
5. Бинарники появятся в `build/<kit-name>/bin/`.

### Сборка из командной строки

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt6/6.5.11/msvc2019_64"
cmake --build build --config Debug
```

### Локализация (после правки `.ts` файлов)

```bash
update_translations.bat
```

Скрипт прогоняет `lupdate` и `lrelease` для сервера и клиента.

---

## Запуск

### Шаг 1. Запуск сервера

1. Скопируйте `config/server.ini.example` в `build/<kit>/bin/config/server.ini` и проверьте параметры подключения к БД.
2. Запустите `ChatServer.exe`.
3. В открывшемся окне войдите как администратор:
   - **Login:** `admin`
   - **Password:** `admin123`

После входа откроется главное окно администратора со вкладками.

### Шаг 2. Запуск клиента

1. Запустите `ChatClient.exe`.
2. Зарегистрируйтесь (кнопка «Создать аккаунт») или войдите существующим логином.
3. По умолчанию подключение к `localhost:54000` (можно изменить через «Дополнительно»).

### Тестовые учётные записи

В БД при первом запуске создаются:
- `admin / admin123` — администратор
- `alice / secretA` — обычный пользователь
- `bob / secretB` — обычный пользователь

---

## Сетевой протокол

JSON-фреймы поверх TCP с длино-префиксным фреймингом:
```
[uint32 BE: длина]  [UTF-8 JSON: { "cmd": "...", "payload": {...}, "request_id": N }]
```

### Команды клиент → сервер

`register`, `login`, `logout`, `send_message`, `edit_message`, `delete_message`, `get_history`, `get_users`

### Push-события сервер → клиент

`new_message`, `message_edited`, `message_deleted`, `user_online`, `user_offline`, `kicked`, `banned`

---

## Тестирование

В проекте 6 наборов автотестов на QTest:

| Тест | Что проверяет |
|------|----------------|
| `test_common` | Сериализация моделей, утилиты, форматирование |
| `test_protocol` | JSON-протокол, кодирование/декодирование команд |
| `test_framecodec` | Length-prefix фрейминг, обработка частичных пакетов |
| `test_password_hasher` | SHA-256 + соль, проверка стабильности |
| `test_database` | CRUD-операции на in-memory SQLite |
| `test_auth_service` | Регистрация, аутентификация, бан |
| `test_clientserver` | Интеграционный: реальный TCP, два клиента |

Запуск всех тестов из Qt Creator: переключите конфигурацию на тестовый таргет → Run.

Из командной строки:

```bash
ctest --test-dir build --output-on-failure
```

---

## Атрибуции

- **Эмодзи:** [OpenMoji](https://openmoji.org/) — open-source emoji and icon project, лицензия [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/).
- **Иконки приложения:** собственные SVG.

---

## Лицензия

Проект распространяется под лицензией **MIT** — см. файл [LICENSE](LICENSE).

---

## Автор

**Чумаченко О. А.**

Итоговая аттестация по программе «Профессия "C++ разработчик"», 2026 г.

GitHub: [@OlesyaChuma](https://github.com/OlesyaChuma)
