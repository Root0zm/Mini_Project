#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <vector>
#include <string>
#include <bitset>
#include <algorithm>
#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QByteArray>
#include <QStyleFactory>
#include <random>
#include <chrono>
#include "filelistwindow.h"
#include <QCryptographicHash>
using namespace std;

// Khai báo nguyên mẫu hàm
string DES_round(const string &pt_bin, const vector<string> &rkb);

// Hàm hỗ trợ
string bin_from_bytes(const QByteArray &bytes) {
    string bin;
    bin.reserve(bytes.size() * 8);
    for (int i = 0; i < bytes.size(); ++i) {
        bin += bitset<8>(static_cast<unsigned char>(bytes[i])).to_string();
    }
    return bin;
}

QByteArray bytes_from_bin(const string &bin) {
    QByteArray bytes;
    for (size_t i = 0; i < bin.size(); i += 8) {
        string byte_str = bin.substr(i, 8);
        unsigned long val = bitset<8>(byte_str).to_ulong();
        bytes.append(static_cast<char>(val));
    }
    return bytes;
}

string xor_str(const string &a, const string &b) {
    string result;
    result.reserve(a.size());
    for (size_t i = 0; i < a.size(); ++i) {
        result += (a[i] == b[i] ? '0' : '1');
    }
    return result;
}

string permute(const string &in, const vector<int> &arr) {
    string out;
    out.reserve(arr.size());
    for (int pos : arr) {
        out += in[pos - 1];
    }
    return out;
}

string shift_left(const string &in, int shifts) {
    string s = in;
    while (shifts--) {
        char c = s.front();
        s.erase(s.begin());
        s.push_back(c);
    }
    return s;
}

// Bảng hoán vị và S-box
static const vector<int> initial_perm = {58,50,42,34,26,18,10,2,60,52,44,36,28,20,12,4,62,54,46,38,30,22,14,6,64,56,48,40,32,24,16,8,57,49,41,33,25,17,9,1,59,51,43,35,27,19,11,3,61,53,45,37,29,21,13,5,63,55,47,39,31,23,15,7};
static const vector<int> exp_d = {32,1,2,3,4,5,4,5,6,7,8,9,8,9,10,11,12,13,12,13,14,15,16,17,16,17,18,19,20,21,20,21,22,23,24,25,24,25,26,27,28,29,28,29,30,31,32,1};
static const vector<int> per = {16,7,20,21,29,12,28,17,1,15,23,26,5,18,31,10,2,8,24,14,32,27,3,9,19,13,30,6,22,11,4,25};
static const int sbox[8][4][16] = {
    {{14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},{0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},{4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},{15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}},
    {{15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},{3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},{0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},{13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}},
    {{10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},{13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},{13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},{1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}},
    {{7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},{13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},{10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},{3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}},
    {{2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},{14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},{4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},{11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}},
    {{12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},{10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},{9,14,15,5,2,0,12,3,7,0,4,10,1,13,11,6},{4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}},
    {{4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},{13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},{1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},{6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}},
    {{13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},{1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},{7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},{2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}}
};
static const vector<int> final_perm = {40,8,48,16,56,24,64,32,39,7,47,15,55,23,63,31,38,6,46,14,54,22,62,30,37,5,45,13,53,21,61,29,36,4,44,12,52,20,60,28,35,3,43,11,51,19,59,27,34,2,42,10,50,18,58,26,33,1,41,9,49,17,57,25};
static const vector<int> keyp = {57,49,41,33,25,17,9,1,58,50,42,34,26,18,10,2,59,51,43,35,27,19,11,3,60,52,44,36,63,55,47,39,31,23,15,7,62,54,46,38,30,22,14,6,61,53,45,37,29,21,13,5,28,20,12,4};
static const vector<int> shift_table = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
static const vector<int> key_comp = {14,17,11,24,1,5,3,28,15,6,21,10,23,19,12,4,26,8,16,7,27,20,13,2,41,52,31,37,47,55,30,40,51,45,33,48,44,49,39,56,34,53,46,42,50,36,29,32};

void generate_round_keys(const string& key_bin, vector<string>& rkb) {
    string permuted_key = permute(key_bin, keyp);
    string L = permuted_key.substr(0, 28);
    string R = permuted_key.substr(28, 28);

    rkb.clear();

    for (int i = 0; i < 16; ++i) {
        L = shift_left(L, shift_table[i]);
        R = shift_left(R, shift_table[i]);
        string combined = L + R;
        string round_key = permute(combined, key_comp);
        rkb.push_back(round_key);
    }
}

string DES_round(const string &pt_bin, const vector<string> &rkb) {
    string pt = permute(pt_bin, initial_perm);
    string L = pt.substr(0, 32);
    string R = pt.substr(32, 32);

    for (int i = 0; i < 16; ++i) {
        string R_exp = permute(R, exp_d);
        string xor_x = xor_str(R_exp, rkb[i]);

        string sbox_out;
        for (int j = 0; j < 8; ++j) {
            int row = (xor_x[j*6] - '0')*2 + (xor_x[j*6+5] - '0');
            int col = (xor_x[j*6+1] - '0')*8 + (xor_x[j*6+2] - '0')*4 + (xor_x[j*6+3] - '0')*2 + (xor_x[j*6+4] - '0');
            sbox_out += bitset<4>(sbox[j][row][col]).to_string();
        }

        sbox_out = permute(sbox_out, per);
        string tmp = xor_str(L, sbox_out);
        L = tmp;

        if (i != 15) swap(L, R);
    }

    string combined = L + R;
    return permute(combined, final_perm);
}

// Mã hóa chuỗi ASCII sang Base64 bằng CBC
QString MainWindow::encrypt(const QString &plaintext, const QString &key) {
    QString adjustedKey = key;
    if (adjustedKey.length() > 9) {
        QMessageBox::information(this, tr("Cảnh báo"), tr("Chỉ khóa chỉ có 8 ký tự. Khóa sẽ được cắt thành 8 ký tự đầu tiên."));
        adjustedKey = adjustedKey.left(8);
    } else if (adjustedKey.length() > 8) {
        adjustedKey = adjustedKey.left(8);
    } else if (adjustedKey.length() < 8) {
        adjustedKey = adjustedKey.leftJustified(8, '\0');
    }

    QByteArray keyBytes = adjustedKey.toLatin1();
    string key_bin = bin_from_bytes(keyBytes);

    vector<string> rkb;
    generate_round_keys(key_bin, rkb);

    QByteArray ivBytes = QByteArray(8, 0); // IV 8 byte null
    QByteArray inputBytes = plaintext.toLatin1();

    int padding = 8 - (inputBytes.size() % 8);
    if (padding == 0) padding = 8;
    inputBytes.append(QByteArray(padding, padding));

    QByteArray ciphertextBytes;
    for (int i = 0; i < inputBytes.size(); i += 8) {
        QByteArray block = inputBytes.mid(i, 8);
        string block_bin = bin_from_bytes(block);
        string iv_bin = bin_from_bytes(ivBytes);
        string xored_bin = xor_str(block_bin, iv_bin);
        string encrypted_bin = DES_round(xored_bin, rkb);
        QByteArray encryptedBlock = bytes_from_bin(encrypted_bin);
        ciphertextBytes.append(encryptedBlock);
        ivBytes = encryptedBlock;
    }

    return QString(ciphertextBytes.toBase64());
}

// Giải mã chuỗi Base64 bằng CBC
QString MainWindow::decrypt(const QString &ciphertextBase64, const QString &key) {
    QString adjustedKey = key;
    if (adjustedKey.length() > 9) {
        QMessageBox::information(this, tr("Cảnh báo"), tr("Chỉ khóa chỉ có 8 ký tự. Khóa sẽ được cắt thành 8 ký tự đầu tiên."));
        adjustedKey = adjustedKey.left(8);
    } else if (adjustedKey.length() > 8) {
        adjustedKey = adjustedKey.left(8);
    } else if (adjustedKey.length() < 8) {
        adjustedKey = adjustedKey.leftJustified(8, '\0');
    }

    QByteArray keyBytes = adjustedKey.toLatin1();
    string key_bin = bin_from_bytes(keyBytes);

    vector<string> rkb;
    generate_round_keys(key_bin, rkb);
    reverse(rkb.begin(), rkb.end());

    QByteArray ivBytes = QByteArray(8, 0); // IV 8 byte null
    QByteArray ciphertextBytes = QByteArray::fromBase64(ciphertextBase64.toLatin1());

    QByteArray plaintextBytes;
    for (int i = 0; i < ciphertextBytes.size(); i += 8) {
        QByteArray block = ciphertextBytes.mid(i, 8);
        string block_bin = bin_from_bytes(block);
        string iv_bin = bin_from_bytes(ivBytes);
        string decrypted_bin = DES_round(block_bin, rkb);
        string xored_bin = xor_str(decrypted_bin, iv_bin);
        QByteArray xoredBlock = bytes_from_bin(xored_bin);
        plaintextBytes.append(xoredBlock);
        ivBytes = block;
    }

    int padding = static_cast<unsigned char>(plaintextBytes.back());
    if (padding > 0 && padding <= 8) {
        plaintextBytes.chop(padding);
    }

    return QString(plaintextBytes);
}

// Hàm tính hash SHA-256 của file
QByteArray calculateFileHash(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (hash.addData(&file)) {
        return hash.result();
    }
    return QByteArray();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Ngắt kết nối tự động cho các nút
    disconnect(ui->openPlaintText, &QPushButton::clicked, 0, 0);
    disconnect(ui->save1, &QPushButton::clicked, 0, 0);

    disconnect(ui->openCypherText, &QPushButton::clicked, 0, 0);
    disconnect(ui->save2, &QPushButton::clicked, 0, 0);

    disconnect(ui->Reset, &QPushButton::clicked, 0, 0);
    disconnect(ui->Reset2, &QPushButton::clicked, 0, 0);
    disconnect(ui->swap, &QPushButton::clicked, 0, 0);

    disconnect(ui->autoKey, &QPushButton::clicked, 0, 0);
    disconnect(ui->myEncryptButton, &QPushButton::clicked, 0, 0);
    disconnect(ui->myDecryptButton, &QPushButton::clicked, 0, 0);

    // Kết nối nút mã hóa
    connect(ui->myEncryptButton, &QPushButton::clicked, this, [=]() {
        QString plaintext = ui->myTextEditInput->toPlainText();
        QString key = ui->myKeyInput->text();
        QString result = encrypt(plaintext, key);
        ui->myTextEditOutput->setText(result);
    });

    // Kết nối nút giải mã
    connect(ui->myDecryptButton, &QPushButton::clicked, this, [=]() {
        QString ciphertext = ui->myTextEditInput2->toPlainText();
        QString key = ui->myKeyInput_2->text();
        QString result = decrypt(ciphertext, key);
        ui->myTextEditOutput2->setText(result);
        originalTextOutput2 = result;
        originalKey2 = key;
    });

    // Kết nối nút tạo key tự động
    connect(ui->autoKey, &QPushButton::clicked, this, &MainWindow::on_autoKey_clicked, Qt::UniqueConnection);

    // Kết nối các nút khác
    connect(ui->openPlaintText, &QPushButton::clicked, this, &MainWindow::on_openPlaintText_clicked, Qt::UniqueConnection);
    connect(ui->save1, &QPushButton::clicked, this, &MainWindow::on_save1_clicked, Qt::UniqueConnection);

    connect(ui->openCypherText, &QPushButton::clicked, this, &MainWindow::on_openCypherText_clicked, Qt::UniqueConnection);
    connect(ui->save2, &QPushButton::clicked, this, &MainWindow::on_save2_clicked, Qt::UniqueConnection);

    connect(ui->Reset, &QPushButton::clicked, this, &MainWindow::on_reset_clicked, Qt::UniqueConnection);
    connect(ui->Reset2, &QPushButton::clicked, this, &MainWindow::on_reset2_clicked, Qt::UniqueConnection);
    connect(ui->swap, &QPushButton::clicked, this, &MainWindow::on_swap_clicked, Qt::UniqueConnection);

    // Kết nối cảnh báo khi thay đổi bản rõ và khóa trong giải mã
    connect(ui->myTextEditOutput2, &QTextEdit::textChanged, this, &MainWindow::on_textOutput2_changed);
    connect(ui->myKeyInput_2, &QLineEdit::textChanged, this, &MainWindow::on_keyInput2_changed);

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::WindowText, Qt::black);
    lightPalette.setColor(QPalette::Base, QColor(245, 245, 245));
    lightPalette.setColor(QPalette::AlternateBase, QColor(233, 231, 228));
    lightPalette.setColor(QPalette::ToolTipBase, Qt::white);
    lightPalette.setColor(QPalette::ToolTipText, Qt::black);
    lightPalette.setColor(QPalette::Text, Qt::black);
    lightPalette.setColor(QPalette::Button, QColor(230, 230, 230));
    lightPalette.setColor(QPalette::ButtonText, Qt::black);
    lightPalette.setColor(QPalette::BrightText, Qt::red);
    lightPalette.setColor(QPalette::Highlight, QColor(76, 163, 224));
    lightPalette.setColor(QPalette::HighlightedText, Qt::white);

    qApp->setPalette(lightPalette);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Slot cho nút tạo key tự động
void MainWindow::on_autoKey_clicked() {
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 gen(static_cast<unsigned>(seed));
    std::uniform_int_distribution<> dis(33, 126);

    QString key;
    key.reserve(8);
    for (int i = 0; i < 8; ++i) {
        key.append(QChar(dis(gen)));
    }
    ui->myKeyInput->setText(key); // Chỉ cập nhật key cho phần mã hóa
}

// Slot cho nút mở file plaintext (mã hóa)
void MainWindow::on_openPlaintText_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Mở file văn bản"), "", tr("File văn bản (*.txt)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString plaintext = in.readAll().trimmed();
            file.close();

            // Tìm file key tương ứng, nếu không có thì để trống mà không báo lỗi
            QString keyFileName = fileName.left(fileName.lastIndexOf('.')) + ".key";
            QFile keyFile(keyFileName);
            QString key;
            if (keyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream keyIn(&keyFile);
                key = keyIn.readAll().trimmed();
                keyFile.close();
            }
            if (key.length() > 9) {
                QMessageBox::information(this, tr("Cảnh báo"), tr("Chỉ khóa chỉ có 8 ký tự. Khóa sẽ được cắt thành 8 ký tự đầu tiên."));
                key = key.left(8);
            } else if (key.length() > 8) {
                key = key.left(8);
            } else if (key.length() < 8) {
                key = key.leftJustified(8, '\0');
            }
            ui->myKeyInput->setText(key);
            ui->myTextEditInput->setText(plaintext);

            // Kiểm tra file hash cho .txt và .key riêng biệt
            QString txtHashFileName = fileName.left(fileName.lastIndexOf('.')) + ".txt.hash";
            QString keyHashFileName = fileName.left(fileName.lastIndexOf('.')) + ".key.hash";
            bool fileModified = false;
            bool keyModified = false;

            QFile txtHashFile(txtHashFileName);
            if (txtHashFile.open(QIODevice::ReadOnly)) {
                QTextStream txtHashIn(&txtHashFile);
                QByteArray storedTxtHash = QByteArray::fromHex(txtHashIn.readAll().trimmed().toLatin1());
                txtHashFile.close();

                QByteArray currentTxtHash = calculateFileHash(fileName);
                if (!currentTxtHash.isEmpty() && currentTxtHash != storedTxtHash) {
                    fileModified = true;
                }
            }

            if (!keyFileName.isEmpty() && QFile(keyFileName).exists()) {
                QFile keyHashFile(keyHashFileName);
                if (keyHashFile.open(QIODevice::ReadOnly)) {
                    QTextStream keyHashIn(&keyHashFile);
                    QByteArray storedKeyHash = QByteArray::fromHex(keyHashIn.readAll().trimmed().toLatin1());
                    keyHashFile.close();

                    QByteArray currentKeyHash = calculateFileHash(keyFileName);
                    if (!currentKeyHash.isEmpty() && currentKeyHash != storedKeyHash) {
                        keyModified = true;
                    }
                }
            }

            if (fileModified) {
                QMessageBox::warning(this, tr("Cảnh báo"), tr("File dữ liệu đã bị chỉnh sửa!"));
            }
            if (keyModified) {
                QMessageBox::warning(this, tr("Cảnh báo"), tr("File khóa đã bị chỉnh sửa!"));
            }
        } else {
            QMessageBox::warning(this, tr("Lỗi"), tr("Không thể mở file!"));
        }
    }
}

// Slot cho nút lưu bản mã (mã hóa)
void MainWindow::on_save1_clicked() {
    QString ciphertext = ui->myTextEditOutput->toPlainText();
    if (!ciphertext.isEmpty()) {
        QString folderPath = QDir::currentPath() + "/EncryptedFiles";
        QDir dir(folderPath);
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                QMessageBox::warning(this, tr("Lỗi"), tr("Không thể tạo thư mục 'EncryptedFiles'. Kiểm tra quyền truy cập hoặc đường dẫn."));
                return;
            }
        }
        QString fileName = QFileDialog::getSaveFileName(this, tr("Lưu file bản mã"), dir.path(), tr("File văn bản (*.txt)"));
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << ciphertext;
                file.close();

                // Lưu file key
                QString keyFileName = fileName.left(fileName.lastIndexOf('.')) + ".key";
                QFile keyFile(keyFileName);
                if (keyFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream keyOut(&keyFile);
                    keyOut << ui->myKeyInput->text();
                    keyFile.close();
                } else {
                    QMessageBox::warning(this, tr("Lỗi"), tr("Không thể lưu file key (%1).").arg(keyFileName));
                }

                // Tính và lưu hash riêng cho .txt và .key
                QByteArray txtHash = calculateFileHash(fileName);
                QString txtHashFileName = fileName.left(fileName.lastIndexOf('.')) + ".txt.hash";
                QFile txtHashFile(txtHashFileName);
                if (txtHashFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream txtHashOut(&txtHashFile);
                    txtHashOut << txtHash.toHex();
                    txtHashFile.close();
                } else {
                    QMessageBox::warning(this, tr("Lỗi"), tr("Không thể lưu file hash cho dữ liệu (%1).").arg(txtHashFileName));
                }

                if (!keyFileName.isEmpty() && QFile(keyFileName).exists()) {
                    QByteArray keyHash = calculateFileHash(keyFileName);
                    QString keyHashFileName = fileName.left(fileName.lastIndexOf('.')) + ".key.hash";
                    QFile keyHashFile(keyHashFileName);
                    if (keyHashFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QTextStream keyHashOut(&keyHashFile);
                        keyHashOut << keyHash.toHex();
                        keyHashFile.close();
                    } else {
                        QMessageBox::warning(this, tr("Lỗi"), tr("Không thể lưu file hash cho key (%1).").arg(keyHashFileName));
                    }
                }
            } else {
                QMessageBox::warning(this, tr("Lỗi"), tr("Không lưu được file bản mã!"));
            }
        }
    } else {
        QMessageBox::warning(this, tr("Lỗi"), tr("Không có gì để lưu!"));
    }
}

// Slot cho nút mở folder bản mã (mã hóa)
void MainWindow::on_openCypher_clicked() {
    QString folderPath = QDir::currentPath() + "/EncryptedFiles";
    QDir dir(folderPath);
    if (!dir.exists()) {
        if (dir.mkpath(".")) {
            QMessageBox::information(this, tr("Thành công"), tr("Thư mục 'EncryptedFiles' đã được tạo thành công."));
        } else {
            QMessageBox::warning(this, tr("Lỗi"), tr("Không thể tạo thư mục 'EncryptedFiles'. Kiểm tra quyền truy cập hoặc đường dẫn."));
            return;
        }
    }
    FileListWindow *window = new FileListWindow(folderPath, "Ciphertext", this);
    connect(window, &FileListWindow::itemDoubleClicked, this, &MainWindow::handleItemDoubleClicked);
    window->show();
}

// Slot cho nút mở file ciphertext (giải mã)
void MainWindow::on_openCypherText_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Mở file văn bản"), "", tr("File văn bản (*.txt)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString ciphertext = in.readAll().trimmed();
            file.close();

            // Tìm file key tương ứng
            QString keyFileName = fileName.left(fileName.lastIndexOf('.')) + ".key";
            QFile keyFile(keyFileName);
            QString key;
            if (keyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream keyIn(&keyFile);
                key = keyIn.readAll().trimmed();
                keyFile.close();
            }
            if (key.length() > 9) {
                QMessageBox::information(this, tr("Cảnh báo"), tr("Chỉ khóa chỉ có 8 ký tự. Khóa sẽ được cắt thành 8 ký tự đầu tiên."));
                key = key.left(8);
            } else if (key.length() > 8) {
                key = key.left(8);
            } else if (key.length() < 8) {
                key = key.leftJustified(8, '\0');
            }
            ui->myKeyInput_2->setText(key);
            ui->myTextEditInput2->setText(ciphertext);

            // Kiểm tra file hash cho .txt và .key riêng biệt
            QString txtHashFileName = fileName.left(fileName.lastIndexOf('.')) + ".txt.hash";
            QString keyHashFileName = fileName.left(fileName.lastIndexOf('.')) + ".key.hash";
            bool fileModified = false;
            bool keyModified = false;

            QFile txtHashFile(txtHashFileName);
            if (txtHashFile.open(QIODevice::ReadOnly)) {
                QTextStream txtHashIn(&txtHashFile);
                QByteArray storedTxtHash = QByteArray::fromHex(txtHashIn.readAll().trimmed().toLatin1());
                txtHashFile.close();

                QByteArray currentTxtHash = calculateFileHash(fileName);
                if (!currentTxtHash.isEmpty() && currentTxtHash != storedTxtHash) {
                    fileModified = true;
                }
            }

            if (!keyFileName.isEmpty() && QFile(keyFileName).exists()) {
                QFile keyHashFile(keyHashFileName);
                if (keyHashFile.open(QIODevice::ReadOnly)) {
                    QTextStream keyHashIn(&keyHashFile);
                    QByteArray storedKeyHash = QByteArray::fromHex(keyHashIn.readAll().trimmed().toLatin1());
                    keyHashFile.close();

                    QByteArray currentKeyHash = calculateFileHash(keyFileName);
                    if (!currentKeyHash.isEmpty() && currentKeyHash != storedKeyHash) {
                        keyModified = true;
                    }
                }
            }

            if (fileModified) {
                QMessageBox::warning(this, tr("Cảnh báo"), tr("File dữ liệu đã bị chỉnh sửa!"));
            }
            if (keyModified) {
                QMessageBox::warning(this, tr("Cảnh báo"), tr("File  đã bị chỉnh sửa!"));
            }
        } else {
            QMessageBox::warning(this, tr("Lỗi"), tr("Không thể mở file!"));
        }
    }
}

// Slot cho nút lưu bản rõ (giải mã)
void MainWindow::on_save2_clicked() {
    QString plaintext = ui->myTextEditOutput2->toPlainText();
    if (!plaintext.isEmpty()) {
        QString folderPath = QDir::currentPath() + "/DecryptedFiles";
        QDir dir(folderPath);
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                QMessageBox::warning(this, tr("Lỗi"), tr("Không thể tạo thư mục 'DecryptedFiles'. Kiểm tra quyền truy cập hoặc đường dẫn."));
                return;
            }
        }
        QString fileName = QFileDialog::getSaveFileName(this, tr("Lưu file"), dir.path(), tr("File văn bản (*.txt)"));
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << plaintext;
                file.close();
            } else {
                QMessageBox::warning(this, tr("Lỗi"), tr("Không lưu được file!"));
            }
        }
    } else {
        QMessageBox::warning(this, tr("Lỗi"), tr("Không có gì để lưu!"));
    }
}

// Slot cho nút mở folder bản rõ (giải mã)
void MainWindow::on_openPlaint_clicked() {
    QString folderPath = QDir::currentPath() + "/DecryptedFiles";
    QDir dir(folderPath);
    if (!dir.exists()) {
        if (dir.mkpath(".")) {
            QMessageBox::information(this, tr("Thành công"), tr("Thư mục 'DecryptedFiles' đã được tạo thành công."));
        } else {
            QMessageBox::warning(this, tr("Lỗi"), tr("Không thể tạo thư mục 'DecryptedFiles'. Kiểm tra quyền truy cập hoặc đường dẫn."));
            return;
        }
    }
    FileListWindow *window = new FileListWindow(folderPath, "Plaintext", this);
    connect(window, &FileListWindow::itemDoubleClicked, this, &MainWindow::handleItemDoubleClicked);
    window->show();
}

// Slot cho nút Reset (mã hóa)
void MainWindow::on_reset_clicked() {
    ui->myTextEditInput->clear();
    ui->myKeyInput->clear();
    ui->myTextEditOutput->clear();
}

// Slot cho nút Reset2 (giải mã)
void MainWindow::on_reset2_clicked() {
    ui->myTextEditInput2->clear();
    ui->myKeyInput_2->clear();
    ui->myTextEditOutput2->clear();
    originalTextOutput2.clear();
    originalKey2.clear();
}

// Slot cho nút Swap (chuyển bản mã từ mã hóa sang giải mã)
void MainWindow::on_swap_clicked() {
    QString ciphertext = ui->myTextEditOutput->toPlainText();
    if (!ciphertext.isEmpty()) {
        ui->myTextEditInput2->setText(ciphertext);
        ui->myKeyInput_2->setText(ui->myKeyInput->text());
    } else {
        QMessageBox::warning(this, tr("Lỗi"), tr("Không có bản mã để chuyển!"));
    }
}

// Slot cho cảnh báo khi thay đổi khóa trong giải mã
void MainWindow::on_keyInput2_changed() {
    QString currentKey = ui->myKeyInput_2->text();
    if (!originalKey2.isEmpty() && currentKey != originalKey2) {
        QMessageBox::warning(this, tr("Cảnh báo"), tr("Khóa đã bị sửa đổi"));
        originalKey2 = currentKey;
    }
}

// Slot cho cảnh báo khi thay đổi bản rõ trong giải mã
void MainWindow::on_textOutput2_changed() {
    QString currentText = ui->myTextEditOutput2->toPlainText();
    if (!originalTextOutput2.isEmpty() && currentText != originalTextOutput2) {
        QMessageBox::warning(this, tr("Cảnh báo"), tr("Bản mã đã bị sửa đổi"));
        originalTextOutput2 = currentText;
    }
}

void MainWindow::handleItemDoubleClicked(const QString &key, const QString &content, bool isEncrypted) {
    QString adjustedKey = key;
    if (adjustedKey.length() > 9) {
        QMessageBox::information(this, tr("Cảnh báo"), tr("Chỉ khóa chỉ có 8 ký tự. Khóa sẽ được cắt thành 8 ký tự đầu tiên."));
        adjustedKey = adjustedKey.left(8);
    } else if (adjustedKey.length() > 8) {
        adjustedKey = adjustedKey.left(8);
    } else if (adjustedKey.length() < 8) {
        adjustedKey = adjustedKey.leftJustified(8, '\0');
    }
    if (isEncrypted) {
        ui->myKeyInput_2->setText(adjustedKey);
        ui->myTextEditInput2->setText(content);
    } else {
        ui->myKeyInput->setText(adjustedKey);
        ui->myTextEditInput->setText(content);
    }
}
