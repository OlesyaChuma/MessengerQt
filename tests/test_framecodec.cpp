#include <QtTest/QtTest>

#include "FrameCodec.h"

using namespace messenger;

class TestFrameCodec : public QObject {
    Q_OBJECT
private slots:
    void encodeDecodeSingle() {
        const QByteArray payload = "Hello, world!";
        const QByteArray frame = FrameCodec::encode(payload);

        FrameCodec codec;
        const auto frames = codec.feed(frame);

        QCOMPARE(frames.size(), 1);
        QCOMPARE(frames.first(), payload);
        QCOMPARE(codec.bufferedBytes(), 0);
    }

    void multipleFramesInOneChunk() {
        FrameCodec codec;
        QByteArray combined;
        combined += FrameCodec::encode("first");
        combined += FrameCodec::encode("second");
        combined += FrameCodec::encode("third");

        const auto frames = codec.feed(combined);
        QCOMPARE(frames.size(), 3);
        QCOMPARE(frames[0], QByteArray("first"));
        QCOMPARE(frames[1], QByteArray("second"));
        QCOMPARE(frames[2], QByteArray("third"));
    }

    void splitAcrossChunks() {
        FrameCodec codec;
        const QByteArray full = FrameCodec::encode("abcdefghij");

        // Подаём по одному байту
        QList<QByteArray> collected;
        for (int i = 0; i < full.size(); ++i) {
            const auto frames = codec.feed(full.mid(i, 1));
            collected += frames;
        }

        QCOMPARE(collected.size(), 1);
        QCOMPARE(collected.first(), QByteArray("abcdefghij"));
    }

    void halfFrameThenRest() {
        FrameCodec codec;
        const QByteArray full = FrameCodec::encode("payload-test");

        QCOMPARE(codec.feed(full.left(5)).size(), 0);
        QVERIFY(codec.bufferedBytes() > 0);

        const auto frames = codec.feed(full.mid(5));
        QCOMPARE(frames.size(), 1);
        QCOMPARE(frames.first(), QByteArray("payload-test"));
        QCOMPARE(codec.bufferedBytes(), 0);
    }

    void emptyPayload() {
        FrameCodec codec;
        const QByteArray frame = FrameCodec::encode(QByteArray());
        QCOMPARE(frame.size(), 4);

        const auto frames = codec.feed(frame);
        QCOMPARE(frames.size(), 1);
        QCOMPARE(frames.first().size(), 0);
    }

    void oversizedFrameTriggersError() {
        FrameCodec codec;
        // Подкладываем фейковый префикс с длиной больше лимита
        QByteArray bad(4, 0);
        const quint32 huge = 100u * 1024u * 1024u; // 100 MB > kMaxFrameSize
        bad[0] = static_cast<char>((huge >> 24) & 0xFF);
        bad[1] = static_cast<char>((huge >> 16) & 0xFF);
        bad[2] = static_cast<char>((huge >>  8) & 0xFF);
        bad[3] = static_cast<char>( huge        & 0xFF);

        codec.feed(bad);
        QVERIFY(codec.hasError());
    }
};

QTEST_APPLESS_MAIN(TestFrameCodec)
#include "test_framecodec.moc"