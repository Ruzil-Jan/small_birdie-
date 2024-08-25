#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_action_4_triggered();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_spinBox_valueChanged(int arg1);
    void on_comboBox_2_currentIndexChanged(int index);
    void on_comboBox_currentIndexChanged(int index);
    void processFiles(const QString &directoryPath, const QString &fileMask, uint64_t xorValue, bool deleteFiles, bool overwriteFiles);
    void modifyFile(const QString &filePath, uint64_t xorValue);

    void on_action_2_triggered();

private:
    Ui::MainWindow *ui;
    int pollInterval; // Интервал
    QTimer *timer;  // Для таймера
    bool overwriteFiles; // Режим презаписи
    int buttonClickCount = 0; // Для каунтера кнопки)))
    bool warningShown;  // Флаг для варна
    bool fileNotFoundWarningShown; // Флаг для варна на несуществующивй файл
};
#endif // MAINWINDOW_H
