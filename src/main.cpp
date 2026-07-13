// main.cpp
// Qt 程序入口：启动校园 RPG 图形窗口。
#include "MainWindow.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("CampusRPG");
    MainWindow window;
    window.show();
    return app.exec();
}
