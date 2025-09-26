#include "filelistwindow.h"
#include "ui_filelistwindow.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>

FileListWindow::FileListWindow(const QString &folderPath, const QString &columnTitle, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FileListWindow)
    , folderPath(folderPath)
{
    ui->setupUi(this);
    // Đặt tiêu đề cột động
    ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(columnTitle));
    connect(ui->tableWidget, &QTableWidget::cellDoubleClicked, this, [=](int row, int column) {
        if (row >= 0 && row < ui->tableWidget->rowCount()) {
            QString key = ui->tableWidget->item(row, 0)->text();
            QString content = ui->tableWidget->item(row, 1)->text();
            bool isEncrypted = (columnTitle == "Ciphertext");
            emit itemDoubleClicked(key, content, isEncrypted);
        }
    });
    connect(ui->deleteButton, &QPushButton::clicked, this, &FileListWindow::onDeleteButtonClicked);
    loadFiles(folderPath);
}

FileListWindow::~FileListWindow()
{
    delete ui;
}

void FileListWindow::loadFiles(const QString &folderPath)
{
    QDir directory(folderPath);
    if (!directory.exists()) {
        qDebug() << "Folder does not exist:" << folderPath;
        return;
    }

    QStringList fileList = directory.entryList(QStringList() << "*.txt", QDir::Files);
    ui->tableWidget->setRowCount(fileList.size());

    int row = 0;
    for (const QString &fileName : fileList) {
        QString filePath = directory.filePath(fileName);
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString key = in.readLine().trimmed();
            QString content = in.readLine().trimmed();
            file.close();

            QTableWidgetItem *keyItem = new QTableWidgetItem(key);
            QTableWidgetItem *contentItem = new QTableWidgetItem(content);
            keyItem->setData(Qt::UserRole, fileName); // Lưu tên file vào UserRole
            ui->tableWidget->setItem(row, 0, keyItem);
            ui->tableWidget->setItem(row, 1, contentItem);
            row++;
        }
    }
}

void FileListWindow::onDeleteButtonClicked()
{
    int selectedRow = ui->tableWidget->currentRow();
    if (selectedRow < 0) {
        QMessageBox::warning(this, tr("Warning"), tr("Hãy chọn 1 dòng để xóa "));
        return;
    }

    QTableWidgetItem *item = ui->tableWidget->item(selectedRow, 0);
    if (!item) {
        QMessageBox::warning(this, tr("Error"), tr("Could not determine file name."));
        return;
    }

    QString fileName = item->data(Qt::UserRole).toString(); // Lấy tên file từ UserRole
    if (fileName.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Could not determine file name."));
        return;
    }

    QString filePath = folderPath + "/" + fileName;
    QFile file(filePath);
    if (file.exists()) {
        if (file.remove()) {
            ui->tableWidget->removeRow(selectedRow);
            QMessageBox::information(this, tr("Success"), tr("Xóa thành công "));
        } else {
            qDebug() << "Delete failed. Error:" << file.errorString();
            QMessageBox::warning(this, tr("Error"), tr("Không xóa được ").arg(file.errorString()));
        }
    } else {
        qDebug() << "File does not exist:" << filePath;
        QMessageBox::warning(this, tr("Error"), tr("File không tồn tại "));
    }
}
