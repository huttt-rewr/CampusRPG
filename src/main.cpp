// main.cpp
// 程序入口：创建游戏管理对象并启动主流程。
#include "GameManager.h"
#include <iostream>

int main() {
    try {
        GameManager game;
        game.run();
    } catch (const std::exception& ex) {
        std::cout << "程序发生异常：" << ex.what() << "\n";
        return 1;
    }
    return 0;
}
