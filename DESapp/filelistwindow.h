#ifndef FILELISTWINDOW_H
#define FILELISTWINDOW_H

#include <QMainWindow>

namespace Ui {
class FileListWindow;
}

class FileListWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FileListWindow(const QString &folderPath, const QString &columnTitle, QWidget *parent = nullptr);
    ~FileListWindow();

signals:
    void itemDoubleClicked(const QString &key, const QString &content, bool isEncrypted);

private slots:
    void onDeleteButtonClicked();

private:
    Ui::FileListWindow *ui;
    void loadFiles(const QString &folderPath);
    QString folderPath;
};

#endif // FILELISTWINDOW_H
