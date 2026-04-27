#include <QApplication>
#include <QLabel>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QLabel label(QStringLiteral("MessengerQt — сервер (Пакет 1, заглушка)"));
    label.setAlignment(Qt::AlignCenter);
    label.setMinimumSize(480, 160);
    label.show();
    return app.exec();
}