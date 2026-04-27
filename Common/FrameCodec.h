#pragma once

#include <QByteArray>
#include <QList>

namespace messenger {

// Кодек длино-префиксных фреймов поверх произвольного байтового потока.
//
// Формат фрейма:
//     [4 байта: длина payload как uint32 BE] [payload]
//
// Зачем нужен:
// TCP — это поток, не пакеты. recv может вернуть несколько сообщений
// слипшимися или одно сообщение по частям. Кодек собирает это правильно.

class FrameCodec {
public:
    // Максимальный допустимый размер одного фрейма.
    // Больше = разрыв соединения с подозрением на мусорные данные.
    static constexpr int kMaxFrameSize = 16 * 1024 * 1024; // 16 MiB

    FrameCodec() = default;

    // Упаковать payload во фрейм (добавляет 4-байтовый префикс длины).
    static QByteArray encode(const QByteArray& payload);

    // Скормить кодеку очередную порцию из сокета.
    // Возвращает все полные фреймы, которые удалось извлечь.
    // Хвост (неполный фрейм) сохраняется внутри для следующего вызова.
    QList<QByteArray> feed(const QByteArray& chunk);

    // Очистить буфер (например, при разрыве соединения).
    void reset();

    // Текущий размер внутреннего буфера (для тестов и диагностики).
    int bufferedBytes() const { return _buffer.size(); }

    // Признак переполнения / поломанного потока.
    bool hasError() const { return _error; }

private:
    QByteArray _buffer;
    bool _error = false;
};

} // namespace messenger