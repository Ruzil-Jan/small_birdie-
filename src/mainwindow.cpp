#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>  // Для дебаг-сообщении на консоль

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , timer(new QTimer(this))
    , warningShown(false)
    , fileNotFoundWarningShown(false)
{
    ui->setupUi(this);
    setFixedSize(758, 248); // Фиксировать размер окна
    ui->spinBox->setMinimum(500);
    ui->spinBox->setMaximum(10000);
    connect(timer, &QTimer::timeout, this, [this]() {
        QString fileMask = ui->Mask_lineEdit->text();
        QString savePath = ui->lineEditText->text();
        uint64_t xorValue = ui->lineEdit_xor->text().toULongLong(nullptr, 16);
        bool deleteFiles = ui->checkBox->isChecked();
        qDebug() << "Таймер сработал. Запуск обработки файлов...";
        processFiles(savePath, fileMask, xorValue, deleteFiles, overwriteFiles);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_4_triggered()
{
    qDebug() << "Закрытие программы. Пользователь выбрал завершение.";
    QApplication::beep();
    QMessageBox::information(this, "Ахтунг", "Закрытие программы. Программа завершается.");
    QApplication::quit();
}

/// Первый блок функции
void MainWindow::modifyFile(const QString &filePath, uint64_t xorValue) {
    qDebug() << "Попытка модификации файла:" << filePath;
    QFile file(filePath);

    // Попробуем открыть файл для чтения и записи
    if (!file.open(QIODevice::ReadWrite)) {
        qWarning() << "Не удалось открыть файл:" << filePath;
        QMessageBox::warning(nullptr, "Ошибка", "Не удалось открыть файл: " + filePath);
        return;
    }

    // Проверка на пустой файл
    if (file.size() == 0) {
        qWarning() << "Файл пуст или не может быть открыт:" << filePath;
        QMessageBox::warning(nullptr, "Ошибка", "Файл пуст или не может быть открыт: " + filePath);
        file.close();
        return;
    }

    // Чтение данных из файла
    QByteArray data = file.readAll();
    if (data.isEmpty()) {
        qWarning() << "Не удалось прочитать данные из файла:" << filePath;
        QMessageBox::warning(nullptr, "Ошибка", "Не удалось прочитать данные из файла: " + filePath);
        file.close();
        return;
    }

    // Перемещаем указатель на начало файла
    file.seek(0);
    uint64_t datasize = data.size(); // int к uint64_t

    // Выполняем XOR с 8-байтным значением
    for (unsigned int i = 0; i < datasize; i += sizeof(uint64_t)) {
        if (i + sizeof(uint64_t) <= datasize) {
            uint64_t *dataPart = reinterpret_cast<uint64_t*>(data.data() + i);
            *dataPart ^= xorValue;
        }
    }

    // Записываем измененные данные обратно в файл
    if (file.write(data) == -1) {
        qWarning() << "Не удалось записать данные в файл:" << filePath;
        QMessageBox::warning(nullptr, "Ошибка", "Не удалось записать данные в файл: " + filePath);
    }

    file.close();
}
/// Конец первого блока

/// Второй блок функции
void MainWindow::processFiles(const QString &directoryPath, const QString &fileMask, uint64_t xorValue, bool deleteFiles, bool overwriteFiles) {
    qDebug() << "Запуск обработки файлов в директории:" << directoryPath << "с маской:" << fileMask;
    QDir dir(directoryPath);
    QStringList files = dir.entryList(QStringList() << fileMask, QDir::Files);

    // Проверка на наличие файлов, соответствующих маске
    if (files.isEmpty()) {
        if (!warningShown) {
            qWarning() << "Нет файлов, соответствующих маске:" << fileMask;
            QMessageBox::warning(this, "Ошибка", "Нет файлов, соответствующих маске: " + fileMask);
            warningShown = true; // Установить флаг, что предупреждение показано
        }
        return; // Прекращаем выполнение функции, так как нет файлов для обработки
    }

    warningShown = false; // Сбросить флаг, так как файлы найдены

    bool fileNotFound = false;

    for (const QString &fileName : files) {
        QString filePath = dir.absoluteFilePath(fileName);

        // Проверка существования файла
        if (!QFile::exists(filePath)) {
            if (!fileNotFoundWarningShown) {
                qWarning() << "Файл не существует:" << filePath;
                QMessageBox::warning(this, "Ошибка", "Файл не существует: " + filePath);
                fileNotFoundWarningShown = true; // Установить флаг, что предупреждение показано
            }
            fileNotFound = true; // Установить флаг, что был найден несуществующий файл
            continue; // Пропустить этот файл
        }

        fileNotFoundWarningShown = false; // Сбросить флаг, так как файл существует

        // Операция XOR
        qDebug() << "Обработка файла:" << filePath;
        modifyFile(filePath, xorValue);

        // Обработка параметра overwriteFiles
        if (!overwriteFiles) {
            qDebug() << "Перемещение файла, так как перезапись отключена.";
            int counter = 1;
            QString newFilePath = filePath;

            while (QFile::exists(newFilePath)) {
                QString baseName = QFileInfo(fileName).completeBaseName();
                QString extension = QFileInfo(fileName).suffix();
                newFilePath = dir.absoluteFilePath(baseName + "_" + QString::number(counter) + "." + extension);
                counter++;
            }

            QFile::rename(filePath, newFilePath); // Переименовать файл
        }

        // Удаление исходного файла, если это требуется
        if (deleteFiles) {
            qDebug() << "Удаление исходного файла:" << filePath;
            QFile::remove(filePath);
        }
    }

    if (fileNotFound) {
        // В случае, если хотя бы один файл не был найден
        qWarning() << "Некоторые файлы не были найдены.";
        QMessageBox::warning(this, "Ошибка", "Некоторые файлы не были найдены.");
    }
}
/// Конец второго блока функции

void MainWindow::on_pushButton_clicked()
{
    qDebug() << "Пользователь выбрал папку через диалог.";
    QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку", "");
    if (!dir.isEmpty()) {
        ui->lineEditText->setText(dir);
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    qDebug() << "Кнопка нажата. Выполняем обработку файлов.";
    buttonClickCount++;
    ui->pushButton_2->setText("Выпускайте бычка!\n Количество нажатий на кнопку:\n " + QString::number(buttonClickCount) + " раз");

    QString fileMask = ui->Mask_lineEdit->text();
    QString savePath = ui->lineEditText->text();
    QString xorString = ui->lineEdit_xor->text();

    // Проверка на пустое поле
    if (xorString.isEmpty()) {
        qWarning() << "Поле XOR пустое.";
        QMessageBox::warning(this, "Ошибка", "Поле XOR не может быть пустым.");
        return;
    }

    // Проверка на корректность формата значения XOR
    bool ok;
    uint64_t xorValue = xorString.toULongLong(&ok, 16); // Пытаемся конвертировать строку в число
    if (!ok) {
        qWarning() << "Некорректное шестнадцатеричное значение в поле XOR:" << xorString;
        QMessageBox::warning(this, "Ошибка", "Введите корректное шестнадцатеричное значение в поле XOR (например, 0xFFFFFFFFFFFFFFFF).");
        return;
    }

    bool deleteFiles = ui->checkBox->isChecked();
    bool runOnce = (ui->comboBox_2->currentIndex() == 1); // Проверка для разового запуска
    pollInterval = ui->spinBox->value();
    overwriteFiles = (ui->comboBox->currentIndex() == 0); // Если 0 - перезапись

    if (runOnce) {
        // Разовый запуск
        qDebug() << "Разовый запуск обработки файлов.";
        processFiles(savePath, fileMask, xorValue, deleteFiles, overwriteFiles);
    } else {
        // Многократный запуск через таймер
        qDebug() << "Многократный запуск через таймер.";
        if (timer->isActive()) {
            timer->stop(); // Останавливаем таймер перед запуском, чтобы избежать дублирования
        }

        connect(timer, &QTimer::timeout, this, [=]() {
            qDebug() << "Таймер сработал. Запуск обработки файлов...";
            processFiles(savePath, fileMask, xorValue, deleteFiles, overwriteFiles);
        });

        timer->start(pollInterval); // Запуск таймера с указанным интервалом
    }
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    qDebug() << "Интервал опроса изменён на:" << arg1;
    pollInterval = arg1;
    if (timer->isActive()) {
        timer->setInterval(pollInterval);
    }
}

void MainWindow::on_comboBox_2_currentIndexChanged(int index)
{
    qDebug() << "Выбран индекс в comboBox_2:" << index;
    if (index == 0) { // Если выбран режим таймера
        qDebug() << "Режим таймера активирован.";
        if (!timer->isActive()) {
            timer->start(pollInterval);
        }
    } else if (index == 1) { // Если выбран разовый запуск
        qDebug() << "Разовый запуск активирован.";
        if (timer->isActive()) {
            timer->stop();
        }
    }
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    qDebug() << "Выбран индекс в comboBox: сли 0 - перезапись 1 - Модификация" << index;
    overwriteFiles = (index == 0); // Если 0 - перезапись
}

void MainWindow::on_action_2_triggered()
{
    QString str  = "Порядок работы программы:\n"
                     "После запуска настраивается пользователем:\n"
                     "а) Маска входных файлов, например .txt, testFile.bin\n"
                     "б) Настройка необходимости удалять входные файлы или нет\n"
                     "в) Путь для сохранения результирующих файлов\n"
                     "г) Действия при повторении имени выходного файла: перезапись или\n"
                     "   модификация, например, счетчик к имени файла\n"
                     "д) Работа по таймеру или разовый запуск\n"
                     "е) Периодичность опроса наличия входного файла (таймер)\n"
                     "ж) Значение 8 байт для бинарной операции модификации файла\n"
                     "Функциональность: модифицирует входные файлы, например операция XOR с 8-байтной переменной, введенной с формы (Пункт 1, г)\n"
                     "Защита от «дурака»: если входной файл не закрыт - не трогать его.\n\n"
                     "Среда разработки QT Creator, язык программирования С++, операционная система Windows, компилятор MinGW.";
    QMessageBox::information(this, "Информация по задаче", str);
    qDebug() << "Вызвана функция on_action_2_triggered.";

}

