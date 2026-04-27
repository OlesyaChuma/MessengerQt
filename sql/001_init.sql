-- Инициализация схемы БД для MessengerQt.
-- Запускать в БД messenger_db от имени messenger_user (или владельца БД).

CREATE TABLE IF NOT EXISTS users (
    id              SERIAL PRIMARY KEY,
    login           VARCHAR(64)  UNIQUE NOT NULL,
    display_name    VARCHAR(128) NOT NULL,
    password_hash   CHAR(64)     NOT NULL,
    password_salt   CHAR(32)     NOT NULL,
    role            VARCHAR(16)  NOT NULL DEFAULT 'user'
                    CHECK (role IN ('user','admin')),
    is_banned       BOOLEAN      NOT NULL DEFAULT FALSE,
    ban_reason      TEXT,
    banned_until    TIMESTAMPTZ,
    banned_by       INTEGER      REFERENCES users(id),
    created_at      TIMESTAMPTZ  NOT NULL DEFAULT NOW(),
    last_seen       TIMESTAMPTZ
);

CREATE INDEX IF NOT EXISTS idx_users_login ON users(login);

CREATE TABLE IF NOT EXISTS messages (
    id              BIGSERIAL    PRIMARY KEY,
    sender_id       INTEGER      NOT NULL REFERENCES users(id),
    receiver_id     INTEGER      REFERENCES users(id),
    is_broadcast    BOOLEAN      NOT NULL,
    body            TEXT,
    created_at      TIMESTAMPTZ  NOT NULL DEFAULT NOW(),
    edited_at       TIMESTAMPTZ,
    deleted_at      TIMESTAMPTZ,
    CHECK (is_broadcast = (receiver_id IS NULL))
);

CREATE INDEX IF NOT EXISTS idx_messages_created   ON messages(created_at DESC);
CREATE INDEX IF NOT EXISTS idx_messages_sender    ON messages(sender_id);
CREATE INDEX IF NOT EXISTS idx_messages_pair      ON messages(sender_id, receiver_id);
CREATE INDEX IF NOT EXISTS idx_messages_broadcast ON messages(is_broadcast) WHERE is_broadcast;

CREATE TABLE IF NOT EXISTS attachments (
    id          BIGSERIAL    PRIMARY KEY,
    message_id  BIGINT       NOT NULL REFERENCES messages(id) ON DELETE CASCADE,
    filename    VARCHAR(255) NOT NULL,
    mime_type   VARCHAR(100) NOT NULL,
    size_bytes  INTEGER      NOT NULL CHECK (size_bytes > 0 AND size_bytes <= 10485760),
    data        BYTEA        NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_attachments_msg ON attachments(message_id);

CREATE TABLE IF NOT EXISTS connection_log (
    id           BIGSERIAL    PRIMARY KEY,
    user_id      INTEGER      REFERENCES users(id),
    login_cache  VARCHAR(64),
    ip_address   VARCHAR(45),
    event_type   VARCHAR(20)  NOT NULL
                 CHECK (event_type IN
                    ('login','logout','kick','ban','unban','failed_login','register')),
    details      TEXT,
    occurred_at  TIMESTAMPTZ  NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_conn_log_user ON connection_log(user_id);
CREATE INDEX IF NOT EXISTS idx_conn_log_time ON connection_log(occurred_at DESC);