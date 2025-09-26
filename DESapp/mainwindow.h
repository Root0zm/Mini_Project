#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_autoKey_clicked();
    void on_openPlaintText_clicked();
    void on_save1_clicked();
    void on_openCypher_clicked();
    void on_openCypherText_clicked();
    void on_save2_clicked();
    void on_openPlaint_clicked();
    void on_reset_clicked();
    void on_reset2_clicked();
    void on_swap_clicked();
    void on_keyInput2_changed();
    void on_textOutput2_changed();

private:
    Ui::MainWindow *ui;
    QString originalKey2;
    QString originalTextOutput2;

    QString encrypt(const QString &plaintext, const QString &key);
    QString decrypt(const QString &ciphertextBase64, const QString &key);
    void handleItemDoubleClicked(const QString &key, const QString &content, bool isEncrypted);
};

#endif // MAINWINDOW_H
