#include <QtTest/QtTest>

class TestSmoke : public QObject {
    Q_OBJECT
private slots:
    void trivial() {
        QCOMPARE(2 + 2, 4);
    }
};

QTEST_APPLESS_MAIN(TestSmoke)
#include "test_smoke.moc"