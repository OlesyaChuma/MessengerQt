#include "FrameCodec.h"

#include <QtEndian>

namespace messenger {

QByteArray FrameCodec::encode(const QByteArray& payload) {
    QByteArray frame;
    frame.resize(4 + payload.size());

    const quint32 length = static_cast<quint32>(payload.size());
    qToBigEndian(length, reinterpret_cast<uchar*>(frame.data()));

    std::memcpy(frame.data() + 4, payload.constData(), payload.size());
    return frame;
}

QList<QByteArray> FrameCodec::feed(const QByteArray& chunk) {
    QList<QByteArray> result;

    if (_error) return result;

    _buffer.append(chunk);

    while (_buffer.size() >= 4) {
        const quint32 length = qFromBigEndian<quint32>(
            reinterpret_cast<const uchar*>(_buffer.constData()));

        if (length > static_cast<quint32>(kMaxFrameSize)) {
            // Поток повреждён — больше не пытаемся ничего разобрать
            _error = true;
            _buffer.clear();
            return result;
        }

        const int totalNeeded = 4 + static_cast<int>(length);
        if (_buffer.size() < totalNeeded) {
            // Полный фрейм ещё не пришёл, ждём следующий chunk
            break;
        }

        result.append(_buffer.mid(4, static_cast<int>(length)));
        _buffer.remove(0, totalNeeded);
    }

    return result;
}

void FrameCodec::reset() {
    _buffer.clear();
    _error = false;
}

} // namespace messenger