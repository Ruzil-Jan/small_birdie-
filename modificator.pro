# Проверка операционной системы
win32 {
    # Проверка компилятора
    mingw {
        # Конфиг проекта для Windows с MinGW
        QT += core gui

        greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

        CONFIG += c++11
        CONFIG += debug

        # Вы можете заставить ваш код не компилироваться, если он использует устаревшие API.
        # Для этого раскомментируйте следующую строку.
        #DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # отключает все API, устаревшие до Qt 6.0.0

        # Указываем пути к папкам
        INCLUDEPATH += $$PWD/includes
       # RESOURCES += $$PWD/resources #а че делать если нет

        # Указываем пути к файлам
        SOURCES += \
            src/main.cpp \
            src/mainwindow.cpp

        HEADERS += \
            includes/mainwindow.h

        FORMS += \
            ui/mainwindow.ui

        # Настройка ресурсов (если есть файл ресурсов .qrc)
        # RESOURCES += resources/myresources.qrc

        # Правила по умолчанию для развертывания
        qnx: target.path = /tmp/$${TARGET}/bin
        else: unix:!android: target.path = /opt/$${TARGET}/bin
        !isEmpty(target.path): INSTALLS += target

    } else {
        error("Этот проект поддерживает только сборку с MinGW на Windows!")
    }
} else {
    error("Этот проект поддерживает только сборку на Windows!")
}
