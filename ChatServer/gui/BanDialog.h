#pragma once

#include <QDialog>

class QLineEdit;

namespace messenger::server::gui {

// Маленький диалог: ввод причины бана/кика.
class BanDialog : public QDialog {
    Q_OBJECT
public:
    enum Mode { Ban, Kick };

    BanDialog(Mode mode, const QString& userLogin, QWidget* parent = nullptr);

    QString reason() const;

private:
    Mode _mode;
    QLineEdit* _reasonEdit = nullptr;
};

} // namespace messenger::server::gui