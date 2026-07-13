// MainWindow.cpp
// Qt 主窗口实现：校园题材轮回 Roguelike 回合制 RPG 的界面、存档、排课、商店、地窟与战斗逻辑。
#include "MainWindow.h"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSettings>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <algorithm>
#include <memory>

namespace {
const int kSlotCount = 4;
const int kInventoryLimit = 40;
const int kDays = 7;
const int kHalfDays = 2;

QStringList normalActions() {
    return {"大物课", "语文课", "外语课", "化学课", "高数课", "专业课", "体育课", "不上课", "打工"};
}

QStringList weekendActions() {
    return {"不上课", "打工", "补习班"};
}

int actionVigorCost(const QString& action) {
    if (action == "不上课") return -15;
    if (action == "体育课" || action == "打工") return 18;
    if (action == "补习班") return 8;
    return 10;
}

int actionGoldChange(const QString& action, int workBonus) {
    if (action == "打工") return 45 + workBonus;
    if (action == "补习班") return -35;
    return 0;
}
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("校园RPG冒险游戏 - 轮回Roguelike Qt版");
    resize(1280, 820);
    setupData();
    stack = new QStackedWidget(this);
    setCentralWidget(stack);
    setupSavePage();
    setupGamePage();
    connectActions();
    setStyleSheet(
        "QMainWindow{background:#f4f6f8;color:#202833;}"
        "QLabel{font-size:14px;}"
        "QPushButton{padding:8px 12px;border:1px solid #9aa8b8;border-radius:5px;background:#ffffff;}"
        "QPushButton:hover{background:#eaf2ff;}"
        "QPushButton:disabled{color:#9aa3ad;background:#eef1f5;}"
        "QTabWidget::pane{border:1px solid #c9d3df;background:#ffffff;}"
        "QTabBar::tab{padding:8px 14px;background:#e3e9f1;border:1px solid #c9d3df;}"
        "QTabBar::tab:selected{background:#ffffff;font-weight:600;}"
        "QListWidget,QTextEdit,QTableWidget{background:#ffffff;border:1px solid #c9d3df;border-radius:5px;padding:5px;}"
    );
    loadSlotMeta();
    showSavePage();
}

void MainWindow::setupData() {
    professions.clear();
    professions.push_back(std::make_unique<StudentProfession>());
    professions.push_back(std::make_unique<IceMageProfession>());
    professions.push_back(std::make_unique<PaladinProfession>());
    professions.push_back(std::make_unique<BlesserProfession>());
    professions.push_back(std::make_unique<BloodWarriorProfession>());
    professions.push_back(std::make_unique<MagicianProfession>());

    angelGoods = {
        {"肾上腺素", ItemType::Medicine, "药品", 120, 0, "战斗中使用，立即回复少量生命并提升本回合攻击。", 35, 0, 0, 8, 0, 0, 0, "", true},
        {"速回药", ItemType::Medicine, "药品", 90, 0, "战斗中使用，快速回复50生命。", 50, 0, 0, 0, 0, 0, 0, "", true},
        {"回血药", ItemType::Medicine, "药品", 70, 0, "战斗中使用，回复30%最大生命。", 0, 0, 0, 0, 0, 0, 0, "", true},
        {"回蓝药", ItemType::Medicine, "药品", 70, 0, "战斗中使用，回复50蓝量。", 0, 50, 0, 0, 0, 0, 0, "", true},
        {"绷带", ItemType::Medicine, "药品", 45, 0, "战斗中使用，回复25生命。", 25, 0, 0, 0, 0, 0, 0, "", true},
        {"阿司匹林", ItemType::Medicine, "药品", 80, 0, "战斗中使用，清除眩晕、迟缓等可清除Debuff。", 15, 0, 0, 0, 0, 0, 0, "", true},
        {"巨无霸牛肉饭", ItemType::Food, "食品", 75, 0, "非战斗使用，回复大量体力和生命。", 45, 0, 30, 0, 0, 0, 0, "", false},
        {"咖喱猪排饭", ItemType::Food, "食品", 65, 0, "非战斗使用，回复生命和体力。", 35, 0, 22, 0, 0, 0, 0, "", false},
        {"校园面包", ItemType::Food, "食品", 25, 0, "非战斗使用，回复少量体力。", 10, 0, 15, 0, 0, 0, 0, "", false},
        {"特制海鲜粥", ItemType::Food, "食品", 85, 0, "非战斗使用，回复生命和蓝量。", 35, 25, 12, 0, 0, 0, 0, "", false},
        {"妈妈秘制蛋花汤", ItemType::Food, "食品", 90, 0, "非战斗使用，回复全队少量生命。", 30, 0, 15, 0, 0, 0, 0, "", false},
        {"热腾腾的肉包子", ItemType::Food, "食品", 35, 0, "非战斗使用，回复生命。", 25, 0, 8, 0, 0, 0, 0, "", false},
        {"魔法教授的手冲咖啡", ItemType::Food, "食品", 80, 0, "非战斗使用，回复蓝量。", 0, 45, 5, 0, 0, 0, 0, "", false},
        {"波子汽水", ItemType::Food, "食品", 30, 0, "非战斗使用，回复体力和蓝量。", 0, 18, 12, 0, 0, 0, 0, "", false},
        {"罐装浓茶", ItemType::Food, "食品", 45, 0, "非战斗使用，回复蓝量并提神。", 0, 30, 6, 0, 0, 0, 0, "", false},
        {"活力少年", ItemType::Growth, "养成", 280, 0, "永久增加每周初始活力。", 0, 0, 0, 0, 0, 0, 0, "", false},
        {"家财万贯", ItemType::Growth, "养成", 320, 0, "永久增加每一次轮回的初始金币。", 0, 0, 0, 0, 0, 0, 0, "", false},
        {"高效打工人", ItemType::Growth, "养成", 260, 0, "永久增加打工所得金币，包括寒假打工。", 0, 0, 0, 0, 0, 0, 0, "", false}
    };

    tasks = {
        {1, "角色命名", "创建第一名角色并命名。", "创建角色", "任意", 1, 0, 0, 100, 0, ""},
        {2, "天使商店购物", "在进入地窟前完成一次天使商店购物。", "天使购物", "任意", 1, 0, 0, 0, 20, ""},
        {3, "恶魔商店购物", "在地窟恶魔商店完成一次购物。", "恶魔购物", "任意", 1, 0, 0, 0, 0, "第一层学生木剑"},
        {4, "装备穿戴", "为任意角色穿戴一件装备。", "穿戴装备", "任意", 1, 0, 0, 0, 15, ""},
        {5, "编队", "调整一次上阵阵型或站位。", "编队", "任意", 1, 0, 0, 0, 15, ""},
        {6, "人生第一个宝箱房", "开启地窟中的第一个宝箱房。", "宝箱", "任意", 1, 0, 0, 80, 10, ""},
        {7, "人生第一次战斗", "完成地窟中的第一次战斗。", "战斗", "任意", 1, 0, 0, 100, 10, ""},
        {8, "通关第一层", "通过神秘地窟第一层。", "通层", "1", 1, 0, 0, 120, 15, ""},
        {9, "通关第二层", "通过神秘地窟第二层。", "通层", "2", 1, 0, 0, 150, 20, ""},
        {10, "通关第三层", "通过神秘地窟第三层。", "通层", "3", 1, 0, 0, 180, 25, ""},
        {11, "通关第四层", "通过神秘地窟第四层。", "通层", "4", 1, 0, 0, 210, 30, ""},
        {12, "通关第五层", "通过神秘地窟第五层。", "通层", "5", 1, 0, 0, 240, 35, ""},
        {13, "通关第六层", "通过神秘地窟第六层。", "通层", "6", 1, 0, 0, 270, 40, ""},
        {14, "打败最终BOSS逃离轮回", "打通七层地窟最终BOSS，打破诅咒。", "最终BOSS", "终焉挂科龙", 1, 0, 0, 1000, 100, ""}
    };
}

void MainWindow::setupSavePage() {
    savePage = new QWidget(this);
    auto* layout = new QVBoxLayout(savePage);
    auto* title = new QLabel("校园RPG冒险游戏\n选择存档开始轮回", savePage);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:28px;font-weight:700;padding:18px;");
    saveHintLabel = new QLabel("总共 4 个存档位。空白存档可创建角色，已有存档可读取或删除。", savePage);
    saveHintLabel->setAlignment(Qt::AlignCenter);
    slotGrid = new QGridLayout();
    layout->addWidget(title);
    layout->addWidget(saveHintLabel);
    layout->addLayout(slotGrid);
    layout->addStretch();
    stack->addWidget(savePage);
}

void MainWindow::setupGamePage() {
    gamePage = new QWidget(this);
    auto* root = new QVBoxLayout(gamePage);
    tabs = new QTabWidget(gamePage);
    root->addWidget(tabs, 1);

    auto* overviewPage = new QWidget(gamePage);
    auto* overviewLayout = new QVBoxLayout(overviewPage);
    overviewLabel = new QLabel(overviewPage);
    overviewLabel->setWordWrap(true);
    logText = new QTextEdit(overviewPage);
    logText->setReadOnly(true);
    auto* saveBtn = new QPushButton("保存当前存档", overviewPage);
    overviewLayout->addWidget(overviewLabel);
    overviewLayout->addWidget(saveBtn);
    overviewLayout->addWidget(logText, 1);
    tabs->addTab(overviewPage, "轮回总览");
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::saveGame);

    auto* schedulePage = new QWidget(gamePage);
    auto* scheduleLayout = new QVBoxLayout(schedulePage);
    schedulePreviewLabel = new QLabel(schedulePage);
    scheduleTable = new QTableWidget(kDays, kHalfDays, schedulePage);
    scheduleTable->setHorizontalHeaderLabels({"上午", "下午"});
    scheduleTable->setVerticalHeaderLabels({"周一", "周二", "周三", "周四", "周五", "周六", "周日"});
    auto* runSemesterBtn = new QPushButton("执行本学期20周课表", schedulePage);
    auto* restBtn = new QPushButton("寒假休息（提升全体血量）", schedulePage);
    auto* workBtn = new QPushButton("寒假打工（获得额外金币）", schedulePage);
    scheduleLayout->addWidget(schedulePreviewLabel);
    scheduleLayout->addWidget(scheduleTable, 1);
    scheduleLayout->addWidget(runSemesterBtn);
    scheduleLayout->addWidget(restBtn);
    scheduleLayout->addWidget(workBtn);
    tabs->addTab(schedulePage, "排课");
    connect(runSemesterBtn, &QPushButton::clicked, this, &MainWindow::runSemester);
    connect(restBtn, &QPushButton::clicked, this, &MainWindow::winterRest);
    connect(workBtn, &QPushButton::clicked, this, &MainWindow::winterWork);
    rebuildScheduleTable();

    auto* rolePage = new QWidget(gamePage);
    auto* roleLayout = new QVBoxLayout(rolePage);
    characterList = new QListWidget(rolePage);
    auto* formation1 = new QPushButton("阵型：前排1 后排2", rolePage);
    auto* formation2 = new QPushButton("阵型：前排2 后排1", rolePage);
    auto* upBtn = new QPushButton("选中角色上移站位", rolePage);
    auto* downBtn = new QPushButton("选中角色下移站位", rolePage);
    auto* equipBtn = new QPushButton("给选中角色穿戴背包选中装备", rolePage);
    roleLayout->addWidget(characterList, 1);
    roleLayout->addWidget(formation1);
    roleLayout->addWidget(formation2);
    roleLayout->addWidget(upBtn);
    roleLayout->addWidget(downBtn);
    roleLayout->addWidget(equipBtn);
    tabs->addTab(rolePage, "角色编队");
    connect(formation1, &QPushButton::clicked, this, &MainWindow::setFormationOneFront);
    connect(formation2, &QPushButton::clicked, this, &MainWindow::setFormationTwoFront);
    connect(upBtn, &QPushButton::clicked, this, &MainWindow::moveRoleUp);
    connect(downBtn, &QPushButton::clicked, this, &MainWindow::moveRoleDown);
    connect(equipBtn, &QPushButton::clicked, this, &MainWindow::equipSelectedItem);

    auto* bagPage = new QWidget(gamePage);
    auto* bagLayout = new QVBoxLayout(bagPage);
    inventoryList = new QListWidget(bagPage);
    auto* useBtn = new QPushButton("使用选中物品", bagPage);
    auto* discardBtn = new QPushButton("丢弃选中物品", bagPage);
    bagLayout->addWidget(inventoryList, 1);
    bagLayout->addWidget(useBtn);
    bagLayout->addWidget(discardBtn);
    tabs->addTab(bagPage, "背包");
    connect(useBtn, &QPushButton::clicked, this, &MainWindow::useInventoryItem);
    connect(discardBtn, &QPushButton::clicked, this, &MainWindow::discardInventoryItem);

    auto* taskPage = new QWidget(gamePage);
    auto* taskLayout = new QVBoxLayout(taskPage);
    taskList = new QListWidget(taskPage);
    auto* acceptBtn = new QPushButton("接受选中任务", taskPage);
    auto* claimBtn = new QPushButton("领取选中奖励", taskPage);
    taskLayout->addWidget(taskList, 1);
    taskLayout->addWidget(acceptBtn);
    taskLayout->addWidget(claimBtn);
    tabs->addTab(taskPage, "任务");
    connect(acceptBtn, &QPushButton::clicked, this, &MainWindow::acceptTask);
    connect(claimBtn, &QPushButton::clicked, this, &MainWindow::claimTask);

    auto* angelPage = new QWidget(gamePage);
    auto* angelLayout = new QVBoxLayout(angelPage);
    angelShopList = new QListWidget(angelPage);
    auto* enterAngelBtn = new QPushButton("暑假进入天使商店", angelPage);
    auto* buyAngelBtn = new QPushButton("购买选中天使商品", angelPage);
    auto* enterDungeonBtn = new QPushButton("购买完成，进入7层神秘地窟（离开后金币清空）", angelPage);
    angelLayout->addWidget(angelShopList, 1);
    angelLayout->addWidget(enterAngelBtn);
    angelLayout->addWidget(buyAngelBtn);
    angelLayout->addWidget(enterDungeonBtn);
    tabs->addTab(angelPage, "天使商店");
    connect(enterAngelBtn, &QPushButton::clicked, this, &MainWindow::enterAngelShop);
    connect(buyAngelBtn, &QPushButton::clicked, this, &MainWindow::buyAngelItem);
    connect(enterDungeonBtn, &QPushButton::clicked, this, &MainWindow::enterDungeon);

    auto* dungeonPage = new QWidget(gamePage);
    auto* dungeonLayout = new QVBoxLayout(dungeonPage);
    dungeonLabel = new QLabel(dungeonPage);
    dungeonLabel->setWordWrap(true);
    dungeonRoomList = new QListWidget(dungeonPage);
    demonShopList = new QListWidget(dungeonPage);
    auto* exploreBtn = new QPushButton("进入/探索选中房间", dungeonPage);
    auto* buyDemonBtn = new QPushButton("购买选中恶魔商品", dungeonPage);
    auto* sellDemonBtn = new QPushButton("出售背包装备给恶魔商店（半价）", dungeonPage);
    fightRoundBtn = new QPushButton("战斗：我方全体行动一轮", dungeonPage);
    battleMedicineBtn = new QPushButton("战斗：使用背包选中药品", dungeonPage);
    dungeonLayout->addWidget(dungeonLabel);
    dungeonLayout->addWidget(dungeonRoomList, 1);
    dungeonLayout->addWidget(demonShopList);
    dungeonLayout->addWidget(exploreBtn);
    dungeonLayout->addWidget(buyDemonBtn);
    dungeonLayout->addWidget(sellDemonBtn);
    dungeonLayout->addWidget(fightRoundBtn);
    dungeonLayout->addWidget(battleMedicineBtn);
    tabs->addTab(dungeonPage, "地窟/恶魔商店");
    connect(exploreBtn, &QPushButton::clicked, this, &MainWindow::exploreRoom);
    connect(buyDemonBtn, &QPushButton::clicked, this, &MainWindow::buyDemonItem);
    connect(sellDemonBtn, &QPushButton::clicked, this, &MainWindow::sellEquipmentToDemon);
    connect(fightRoundBtn, &QPushButton::clicked, this, &MainWindow::fightOneRound);
    connect(battleMedicineBtn, &QPushButton::clicked, this, &MainWindow::useBattleMedicine);

    auto* codexPage = new QWidget(gamePage);
    auto* codexLayout = new QVBoxLayout(codexPage);
    codexList = new QListWidget(codexPage);
    codexLayout->addWidget(codexList, 1);
    tabs->addTab(codexPage, "轮回游记");

    stack->addWidget(gamePage);
}

void MainWindow::rebuildScheduleTable() {
    for (int day = 0; day < kDays; ++day) {
        for (int half = 0; half < kHalfDays; ++half) {
            auto* combo = new QComboBox(scheduleTable);
            combo->addItems(day < 5 ? normalActions() : weekendActions());
            scheduleTable->setCellWidget(day, half, combo);
            connect(combo, &QComboBox::currentTextChanged, this, &MainWindow::refreshSchedulePreview);
        }
    }
}

void MainWindow::connectActions() {
    refreshSaveSlots();
}

void MainWindow::showSavePage() {
    phase = GamePhase::SaveSelect;
    stack->setCurrentWidget(savePage);
    refreshSaveSlots();
}

void MainWindow::showGamePage() {
    stack->setCurrentWidget(gamePage);
    refreshAll();
}

void MainWindow::refreshAll() {
    refreshOverview();
    refreshSchedulePreview();
    refreshCharacters();
    refreshInventory();
    refreshTasks();
    refreshAngelShop();
    refreshDemonShop();
    refreshDungeon();
    refreshCodex();
}

void MainWindow::refreshSaveSlots() {
    while (QLayoutItem* item = slotGrid->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    loadSlotMeta();
    for (int i = 1; i <= kSlotCount; ++i) {
        QString label = saveNames.value(QString::number(i), "空白存档");
        auto* btn = new QPushButton(QString("存档%1\n%2").arg(i).arg(label), savePage);
        btn->setMinimumHeight(110);
        btn->setProperty("slot", i);
        btn->setStyleSheet("font-size:18px;text-align:center;");
        connect(btn, &QPushButton::clicked, this, &MainWindow::slotClicked);
        slotGrid->addWidget(btn, (i - 1) / 2, (i - 1) % 2);
    }
}

void MainWindow::refreshOverview() {
    if (!overviewLabel) return;
    overviewLabel->setText(QString(
        "阶段：%1\n存档位：%2\n轮回次数：%3\n学期：%4\n金币：%5\n恶魔币：%6\n"
        "永久养成：每周初始活力+%7，每轮初始金币+%8，打工收益+%9，初始恶魔币+%10\n"
        "折扣：天使商店-%11%，恶魔商店-%12%\n上阵人数：%13/3，阵型：%14\n背包：%15/%16")
        .arg(phaseText()).arg(currentSlot).arg(loopCount).arg(semester)
        .arg(gold).arg(demonCoin).arg(initialVigorBonus).arg(initialGoldBonus)
        .arg(workGoldBonus).arg(demonFriendBonus).arg(angelDiscount)
        .arg(demonDiscount).arg(party.size()).arg(formationType == 1 ? "前排1 后排2" : "前排2 后排1")
        .arg(inventory.size()).arg(kInventoryLimit));
}

void MainWindow::refreshSchedulePreview() {
    if (!schedulePreviewLabel || !scheduleTable) return;
    int vigor = 100 + initialVigorBonus;
    int profit = 0;
    QStringList details;
    for (int day = 0; day < kDays; ++day) {
        for (int half = 0; half < kHalfDays; ++half) {
            auto* combo = qobject_cast<QComboBox*>(scheduleTable->cellWidget(day, half));
            QString action = combo ? combo->currentText() : "不上课";
            vigor -= actionVigorCost(action);
            profit += actionGoldChange(action, workGoldBonus);
            if (action == "补习班") {
                profit -= 35;
                details << "周末补习班：本半天可额外安排课程，课程效果减半";
            }
        }
    }
    schedulePreviewLabel->setText(QString("本周预计盈利金币：%1 | 本周剩余活力：%2 | 前五天可安排所有行动，周末只能不上课/打工/补习班。%3")
        .arg(profit).arg(vigor).arg(details.isEmpty() ? "" : "\n" + details.join("\n")));
}

void MainWindow::refreshCharacters() {
    if (!characterList) return;
    characterList->clear();
    for (int i = 0; i < party.size(); ++i) {
        QString position;
        if (formationType == 1) {
            position = (i == 0) ? "前排" : "后排";
        } else {
            position = (i < 2) ? "前排" : "后排";
        }
        characterList->addItem(QString("%1 [%2]\n%3\n技能：\n%4")
            .arg(roleText(party[i])).arg(position).arg(party[i].equipment.isEmpty() ? "装备：无" : "装备：" + QStringList(party[i].equipment.values()).join("、"))
            .arg(skillsText(party[i])));
    }
}

void MainWindow::refreshInventory() {
    if (!inventoryList) return;
    inventoryList->clear();
    for (int i = 0; i < inventory.size(); ++i) {
        inventoryList->addItem(QString("%1. %2").arg(i + 1).arg(inventoryText(inventory[i])));
    }
}

void MainWindow::refreshTasks() {
    if (!taskList) return;
    taskList->clear();
    for (const auto& task : tasks) {
        taskList->addItem(QString("[%1] %2. %3 | %4 | 条件：%5 %6 %7/%8 | 奖励：金币%9 恶魔币%10 %11")
            .arg(taskStatusText(task.status)).arg(task.id).arg(task.name).arg(task.description)
            .arg(task.conditionType).arg(task.target).arg(task.progress).arg(task.need)
            .arg(task.rewardGold).arg(task.rewardDemonCoin).arg(task.rewardItem));
    }
}

void MainWindow::refreshAngelShop() {
    if (!angelShopList) return;
    angelShopList->clear();
    for (int i = 0; i < angelGoods.size(); ++i) {
        int price = std::max(angelGoods[i].price * (100 - std::min(50, angelDiscount)) / 100, angelGoods[i].price / 2);
        angelShopList->addItem(QString("%1. [%2/%3] %4 | 价格：%5金币 | %6")
            .arg(i + 1).arg(angelGoods[i].category).arg(itemTypeText(angelGoods[i].type))
            .arg(angelGoods[i].name).arg(price).arg(angelGoods[i].effect));
    }
}

void MainWindow::refreshDemonShop() {
    if (!demonShopList) return;
    demonShopList->clear();
    if (demonGoods.isEmpty()) {
        demonShopList->addItem("恶魔商店尚未刷新：进入地窟每层第一个房间固定刷新。");
        return;
    }
    for (int i = 0; i < demonGoods.size(); ++i) {
        int price = std::max(demonGoods[i].demonPrice * (100 - std::min(50, demonDiscount)) / 100, demonGoods[i].demonPrice / 2);
        demonShopList->addItem(QString("%1. [%2/%3] %4 | 价格：%5恶魔币 | %6")
            .arg(i + 1).arg(demonGoods[i].category).arg(itemTypeText(demonGoods[i].type))
            .arg(demonGoods[i].name).arg(price).arg(demonGoods[i].effect));
    }
}

void MainWindow::refreshDungeon() {
    if (!dungeonRoomList || !dungeonLabel) return;
    dungeonRoomList->clear();
    dungeonLabel->setText(QString("地窟层数：%1/7 | 当前房间：%2 | 战斗状态：%3 | 恶魔币：%4")
        .arg(dungeonLayer).arg(currentRoom + 1).arg(inBattle ? "战斗中" : "非战斗").arg(demonCoin));
    for (int i = 0; i < rooms.size(); ++i) {
        QString type;
        if (rooms[i].type == RoomType::DemonShop) type = "恶魔商店";
        if (rooms[i].type == RoomType::Battle) type = "普通战斗";
        if (rooms[i].type == RoomType::EliteBattle) type = "精英战斗+下一层通道";
        if (rooms[i].type == RoomType::Chest) type = "宝箱房";
        if (rooms[i].type == RoomType::Boss) type = "最终BOSS";
        dungeonRoomList->addItem(QString("房间%1：%2 | %3").arg(i + 1).arg(type).arg(rooms[i].cleared ? "已完成" : "未完成"));
    }
    fightRoundBtn->setEnabled(inBattle);
    battleMedicineBtn->setEnabled(inBattle);
}

void MainWindow::refreshCodex() {
    if (!codexList) return;
    codexList->clear();
    QStringList names = codex.keys();
    names.sort();
    for (const QString& name : names) {
        if (!encountered.value(name, false)) {
            codexList->addItem(QString("%1：？？？").arg(name));
            continue;
        }
        const auto& e = codex[name];
        codexList->addItem(QString("%1 | 第%2层%3 | HP%4 攻%5 防%6 法抗%7 | 技能：%8")
            .arg(e.name).arg(e.layer).arg(e.kind).arg(e.maxHp).arg(e.attack).arg(e.defense).arg(e.resist).arg(e.skills));
    }
}

void MainWindow::appendLog(const QString& text) {
    if (logText) logText->append(text);
}

void MainWindow::slotClicked() {
    auto* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    int slot = btn->property("slot").toInt();
    selectedSlot = slot;
    if (saveNames.contains(QString::number(slot))) {
        QMessageBox box(this);
        box.setWindowTitle("存档操作");
        box.setText(QString("存档%1：%2").arg(slot).arg(saveNames.value(QString::number(slot))));
        auto* loadBtn = box.addButton("读取存档", QMessageBox::AcceptRole);
        auto* deleteBtn = box.addButton("删除存档", QMessageBox::DestructiveRole);
        box.addButton("取消", QMessageBox::RejectRole);
        box.exec();
        if (box.clickedButton() == loadBtn) {
            if (loadGame(slot)) showGamePage();
        } else if (box.clickedButton() == deleteBtn) {
            deleteSelectedSlot();
        }
    } else {
        createInSelectedSlot();
    }
}

void MainWindow::createInSelectedSlot() {
    if (selectedSlot < 1 || selectedSlot > kSlotCount) return;
    QStringList names;
    for (const auto& p : professions) names << QString::fromStdString(p->name());
    bool ok = false;
    QString prof = QInputDialog::getItem(this, "选择职业", "请选择 6 个预设职业之一：", names, 0, false, &ok);
    if (!ok || prof.isEmpty()) return;
    QString roleName = QInputDialog::getText(this, "角色命名", "请输入自定义角色名：", QLineEdit::Normal, "", &ok).trimmed();
    if (!ok || roleName.isEmpty()) return;
    currentSlot = selectedSlot;
    resetGameForNewRun(false);
    createCharacter(prof, roleName);
    phase = GamePhase::SchoolFirst;
    addTaskProgress("创建角色", "任意", 1);
    writeGame(currentSlot);
    showGamePage();
    appendLog(QString("创建角色：%1（%2），进入第一学期排课。").arg(roleName).arg(prof));
}

void MainWindow::deleteSelectedSlot() {
    if (selectedSlot < 1 || selectedSlot > kSlotCount) return;
    if (QMessageBox::question(this, "确认删除", QString("确定删除存档%1吗？").arg(selectedSlot)) != QMessageBox::Yes) return;
    deleteGameFile(selectedSlot);
    refreshSaveSlots();
}

void MainWindow::saveGame() {
    if (currentSlot < 1 || currentSlot > kSlotCount) {
        QMessageBox::warning(this, "无法保存", "当前没有有效存档位。");
        return;
    }
    writeGame(currentSlot);
    QMessageBox::information(this, "保存成功", QString("已保存到存档%1。").arg(currentSlot));
}

void MainWindow::resetGameForNewRun(bool keepGrowth) {
    int oldInitialGold = keepGrowth ? initialGoldBonus : 0;
    int oldInitialVigor = keepGrowth ? initialVigorBonus : 0;
    int oldWorkBonus = keepGrowth ? workGoldBonus : 0;
    int oldDemonFriend = keepGrowth ? demonFriendBonus : 0;
    int oldLoop = keepGrowth ? loopCount + 1 : 1;
    party.clear();
    inventory.clear();
    rooms.clear();
    battleEnemies.clear();
    demonGoods.clear();
    inBattle = false;
    loopCount = oldLoop;
    semester = 1;
    week = 1;
    gold = 100 + oldInitialGold;
    demonCoin = 0;
    initialGoldBonus = oldInitialGold;
    initialVigorBonus = oldInitialVigor;
    workGoldBonus = oldWorkBonus;
    demonFriendBonus = oldDemonFriend;
    angelDiscount = 0;
    demonDiscount = 0;
    formationType = 1;
    dungeonLayer = 0;
    currentRoom = 0;
    firstAngelShopBought = false;
    firstDemonShopBought = false;
    equippedOnce = false;
    formationChanged = false;
    phase = GamePhase::SchoolFirst;
    setupData();
}

void MainWindow::createCharacter(const QString& professionName, const QString& roleName) {
    auto* prof = professionByName(professionName);
    if (!prof) return;
    CharacterData role;
    role.name = roleName;
    role.profession = professionName;
    applyProfessionStats(role, *prof);
    party.push_back(role);
}

void MainWindow::applyProfessionStats(CharacterData& role, const Profession& profession) {
    ProfessionStats stats = profession.baseStats();
    role.maxHp = stats.hp;
    role.hp = stats.hp;
    role.maxMp = stats.mp;
    role.mp = stats.mp;
    role.vigor = stats.vigor;
    role.physicalAttack = stats.physicalAttack;
    role.magicAttack = stats.magicAttack;
    role.physicalDefense = stats.physicalDefense;
    role.magicResistance = stats.magicResistance;
}

void MainWindow::loadSlotMeta() {
    saveNames.clear();
    for (int i = 1; i <= kSlotCount; ++i) {
        QFile file(saveFilePath(i));
        if (!file.exists()) continue;
        QSettings s(saveFilePath(i), QSettings::IniFormat);
        QString name = s.value("meta/displayName").toString();
        if (!name.isEmpty()) saveNames[QString::number(i)] = name;
    }
}

QString MainWindow::saveFilePath(int slot) const {
    QDir dir(QCoreApplication::applicationDirPath());
    if (!dir.exists("saves")) dir.mkpath("saves");
    return dir.filePath(QString("saves/slot_%1.ini").arg(slot));
}

QString MainWindow::currentSaveFilePath() const {
    return saveFilePath(currentSlot);
}

bool MainWindow::loadGame(int slot) {
    QFile file(saveFilePath(slot));
    if (!file.exists()) {
        QMessageBox::warning(this, "读取失败", "存档不存在，请新建角色。");
        return false;
    }
    QSettings s(saveFilePath(slot), QSettings::IniFormat);
    if (!s.contains("meta/displayName")) {
        QMessageBox::warning(this, "读取失败", "存档损坏或缺少角色信息，请新建角色。");
        return false;
    }
    currentSlot = slot;
    setupData();
    loopCount = s.value("state/loopCount", 1).toInt();
    semester = s.value("state/semester", 1).toInt();
    week = s.value("state/week", 1).toInt();
    gold = s.value("state/gold", 100).toInt();
    demonCoin = s.value("state/demonCoin", 0).toInt();
    initialGoldBonus = s.value("state/initialGoldBonus", 0).toInt();
    initialVigorBonus = s.value("state/initialVigorBonus", 0).toInt();
    workGoldBonus = s.value("state/workGoldBonus", 0).toInt();
    demonFriendBonus = s.value("state/demonFriendBonus", 0).toInt();
    angelDiscount = s.value("state/angelDiscount", 0).toInt();
    demonDiscount = s.value("state/demonDiscount", 0).toInt();
    formationType = s.value("state/formationType", 1).toInt();
    dungeonLayer = s.value("state/dungeonLayer", 0).toInt();
    currentRoom = s.value("state/currentRoom", 0).toInt();
    phase = static_cast<GamePhase>(s.value("state/phase", int(GamePhase::SchoolFirst)).toInt());
    deserializeCharacters(s.value("data/characters").toString());
    deserializeInventory(s.value("data/inventory").toString());
    deserializeTasks(s.value("data/tasks").toString());
    deserializeCodex(s.value("data/codex").toString());
    firstAngelShopBought = s.value("flags/firstAngelShopBought", false).toBool();
    firstDemonShopBought = s.value("flags/firstDemonShopBought", false).toBool();
    equippedOnce = s.value("flags/equippedOnce", false).toBool();
    formationChanged = s.value("flags/formationChanged", false).toBool();
    if (dungeonLayer > 0) buildDungeonLayer(dungeonLayer);
    appendLog(QString("读取存档%1成功。").arg(slot));
    return true;
}

void MainWindow::writeGame(int slot) {
    QSettings s(saveFilePath(slot), QSettings::IniFormat);
    QString displayName = party.isEmpty() ? "未命名队伍" : party.first().name;
    s.setValue("meta/displayName", displayName);
    s.setValue("state/loopCount", loopCount);
    s.setValue("state/semester", semester);
    s.setValue("state/week", week);
    s.setValue("state/gold", gold);
    s.setValue("state/demonCoin", demonCoin);
    s.setValue("state/initialGoldBonus", initialGoldBonus);
    s.setValue("state/initialVigorBonus", initialVigorBonus);
    s.setValue("state/workGoldBonus", workGoldBonus);
    s.setValue("state/demonFriendBonus", demonFriendBonus);
    s.setValue("state/angelDiscount", angelDiscount);
    s.setValue("state/demonDiscount", demonDiscount);
    s.setValue("state/formationType", formationType);
    s.setValue("state/dungeonLayer", dungeonLayer);
    s.setValue("state/currentRoom", currentRoom);
    s.setValue("state/phase", int(phase));
    s.setValue("data/characters", serializeCharacters());
    s.setValue("data/inventory", serializeInventory());
    s.setValue("data/tasks", serializeTasks());
    s.setValue("data/codex", serializeCodex());
    s.setValue("flags/firstAngelShopBought", firstAngelShopBought);
    s.setValue("flags/firstDemonShopBought", firstDemonShopBought);
    s.setValue("flags/equippedOnce", equippedOnce);
    s.setValue("flags/formationChanged", formationChanged);
    s.sync();
    loadSlotMeta();
}

void MainWindow::deleteGameFile(int slot) {
    QFile::remove(saveFilePath(slot));
    if (currentSlot == slot) currentSlot = -1;
}

void MainWindow::runSemester() {
    if (phase != GamePhase::SchoolFirst && phase != GamePhase::SchoolSecond) {
        QMessageBox::warning(this, "不能执行课表", "当前阶段不是上学学期。");
        return;
    }
    int weeklyProfit = 0;
    int weeklyVigor = 100 + initialVigorBonus;
    for (int day = 0; day < kDays; ++day) {
        for (int half = 0; half < kHalfDays; ++half) {
            auto* combo = qobject_cast<QComboBox*>(scheduleTable->cellWidget(day, half));
            QString action = combo ? combo->currentText() : "不上课";
            weeklyVigor -= actionVigorCost(action);
            weeklyProfit += actionGoldChange(action, workGoldBonus);
            double effectScale = 1.0;
            if (action == "补习班") {
                weeklyProfit -= 35;
                effectScale = 0.5;
                action = "专业课";
            }
            for (auto& role : party) {
                if (action == "大物课") {
                    role.physicalAttack += int(1 * effectScale);
                    role.physicalDefense += int(1 * effectScale);
                } else if (action == "语文课") {
                    angelDiscount = std::min(50, angelDiscount + int(1 * effectScale));
                } else if (action == "外语课") {
                    demonDiscount = std::min(50, demonDiscount + int(1 * effectScale));
                } else if (action == "化学课") {
                    role.magicResistance += int(2 * effectScale);
                } else if (action == "高数课") {
                    role.physicalDefense += int(2 * effectScale);
                } else if (action == "专业课") {
                    role.physicalAttack += int(2 * effectScale);
                    role.magicAttack += int(2 * effectScale);
                } else if (action == "体育课") {
                    role.maxHp += int(4 * effectScale);
                    role.hp = role.maxHp;
                    role.vigor += int(3 * effectScale);
                } else if (action == "不上课") {
                    role.vigor += 2;
                }
            }
        }
    }
    int totalProfit = weeklyProfit * 20;
    gold += totalProfit;
    for (auto& role : party) addExp(role, 60 * 20);
    appendLog(QString("第%1学期结束：20周课表执行完毕，金币变化%2，本周剩余活力模板为%3。")
        .arg(semester).arg(totalProfit).arg(weeklyVigor));
    if (semester == 1) {
        phase = GamePhase::Winter;
        QMessageBox::information(this, "寒假到来", "第一学期结束，请选择寒假休息或寒假打工。");
    } else {
        phase = GamePhase::AngelShop;
        QMessageBox::information(this, "暑假诅咒触发", "第二学期结束，所有角色被传送至天使商店。购买完成后将进入7层神秘地窟。");
    }
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::winterRest() {
    if (phase != GamePhase::Winter) {
        QMessageBox::warning(this, "阶段错误", "只有第一学期结束后的寒假可以选择休息。");
        return;
    }
    for (auto& role : party) {
        role.maxHp += 25;
        role.hp = role.maxHp;
    }
    semester = 2;
    phase = GamePhase::SchoolSecond;
    appendLog("寒假休息：全体最大生命值提升25，进入第二学期排课。");
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::winterWork() {
    if (phase != GamePhase::Winter) {
        QMessageBox::warning(this, "阶段错误", "只有第一学期结束后的寒假可以选择打工。");
        return;
    }
    int gain = 420 + workGoldBonus * 8;
    gold += gain;
    semester = 2;
    phase = GamePhase::SchoolSecond;
    appendLog(QString("寒假打工：获得额外金币%1，进入第二学期排课。").arg(gain));
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::enterAngelShop() {
    if (phase != GamePhase::AngelShop) {
        QMessageBox::warning(this, "尚未到达暑假", "需要完成第二学期后才能进入天使商店。");
        return;
    }
    appendLog("进入天使商店：可用一学年与寒假打工所得金币购买药品、食品、养成类物品。");
    refreshAll();
}

void MainWindow::buyAngelItem() {
    if (phase != GamePhase::AngelShop) {
        QMessageBox::warning(this, "无法购买", "天使商店只在暑假进入地窟前开放。");
        return;
    }
    int row = angelShopList->currentRow();
    if (row < 0 || row >= angelGoods.size()) return;
    ItemData item = angelGoods[row];
    int price = std::max(item.price * (100 - std::min(50, angelDiscount)) / 100, item.price / 2);
    if (gold < price) {
        QMessageBox::warning(this, "金币不足", "金币不足，购买失败。");
        return;
    }
    gold -= price;
    if (item.name == "活力少年") initialVigorBonus += 10;
    else if (item.name == "家财万贯") initialGoldBonus += 80;
    else if (item.name == "高效打工人") workGoldBonus += 12;
    else addItem(item);
    firstAngelShopBought = true;
    addTaskProgress("天使购物", "任意", 1);
    appendLog(QString("天使商店购买：%1，剩余金币%2。").arg(item.name).arg(gold));
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::buyDemonItem() {
    if (phase != GamePhase::Dungeon || dungeonLayer <= 0) {
        QMessageBox::warning(this, "无法购买", "恶魔商店只在地窟内开放。");
        return;
    }
    int row = demonShopList->currentRow();
    if (row < 0 || row >= demonGoods.size()) return;
    ItemData item = demonGoods[row];
    int price = std::max(item.demonPrice * (100 - std::min(50, demonDiscount)) / 100, item.demonPrice / 2);
    if (demonCoin < price) {
        QMessageBox::warning(this, "恶魔币不足", "恶魔币不足，购买失败。");
        return;
    }
    demonCoin -= price;
    if (item.name == "恶魔之友") demonFriendBonus += 8;
    else addItem(item);
    firstDemonShopBought = true;
    addTaskProgress("恶魔购物", "任意", 1);
    appendLog(QString("恶魔商店购买：%1，剩余恶魔币%2。").arg(item.name).arg(demonCoin));
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::sellEquipmentToDemon() {
    int row = inventoryList->currentRow();
    if (row < 0 || row >= inventory.size()) return;
    if (inventory[row].type != ItemType::Equipment) {
        QMessageBox::warning(this, "无法出售", "恶魔商店只收购装备。");
        return;
    }
    int value = std::max(1, inventory[row].demonPrice / 2);
    QString name = inventory[row].name;
    inventory.removeAt(row);
    demonCoin += value;
    appendLog(QString("出售装备：%1，获得恶魔币%2。").arg(name).arg(value));
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::addExp(CharacterData& role, int amount) {
    role.exp += amount;
    while (role.exp >= role.expNeed) {
        role.exp -= role.expNeed;
        role.level++;
        role.expNeed = int(role.expNeed * 1.35) + 40;
        role.maxHp += 12;
        role.maxMp += 6;
        role.physicalAttack += 3;
        role.magicAttack += 3;
        role.physicalDefense += 2;
        role.magicResistance += 2;
        role.hp = role.maxHp;
        role.mp = role.maxMp;
        appendLog(QString("%1 升级到 Lv.%2，生命/蓝量回满，属性提升。").arg(role.name).arg(role.level));
    }
}

void MainWindow::addItem(const ItemData& item) {
    if (inventory.size() >= kInventoryLimit) {
        QMessageBox::warning(this, "背包已满", "背包满后无法拾取。");
        return;
    }
    inventory.push_back(item);
}

void MainWindow::useInventoryItem() {
    if (inBattle) {
        useBattleMedicine();
        return;
    }
    int row = inventoryList->currentRow();
    if (row < 0 || row >= inventory.size() || party.isEmpty()) return;
    ItemData item = inventory[row];
    if (item.type == ItemType::Equipment || item.type == ItemType::Growth) {
        QMessageBox::information(this, "提示", "装备请在角色编队页面穿戴，养成物品购买时立即生效。");
        return;
    }
    CharacterData& role = party[0];
    if (item.hpRecover > 0) role.hp = std::min(role.maxHp, role.hp + item.hpRecover);
    if (item.mpRecover > 0) role.mp = std::min(role.maxMp, role.mp + item.mpRecover);
    if (item.staminaRecover > 0) role.vigor += item.staminaRecover;
    inventory.removeAt(row);
    appendLog(QString("非战斗使用物品：%1。").arg(item.name));
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::discardInventoryItem() {
    int row = inventoryList->currentRow();
    if (row < 0 || row >= inventory.size()) return;
    QString name = inventory[row].name;
    inventory.removeAt(row);
    appendLog(QString("丢弃物品：%1。").arg(name));
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::equipSelectedItem() {
    int roleRow = characterList->currentRow();
    int itemRow = inventoryList->currentRow();
    if (roleRow < 0 || roleRow >= party.size() || itemRow < 0 || itemRow >= inventory.size()) {
        QMessageBox::warning(this, "请选择", "需要在角色列表选角色，并在背包列表选装备。");
        return;
    }
    ItemData item = inventory[itemRow];
    if (item.type != ItemType::Equipment) {
        QMessageBox::warning(this, "不是装备", "每个角色可穿4件不同类型装备：物攻、法攻、物防、法抗。");
        return;
    }
    CharacterData& role = party[roleRow];
    QString old = role.equipment.value(item.equipSlot);
    if (!old.isEmpty()) addItem(itemByName(old));
    role.equipment[item.equipSlot] = item.name;
    role.physicalAttack += item.attackBonus;
    role.magicAttack += item.magicBonus;
    role.physicalDefense += item.defenseBonus;
    role.magicResistance += item.resistBonus;
    inventory.removeAt(itemRow);
    equippedOnce = true;
    addTaskProgress("穿戴装备", "任意", 1);
    appendLog(QString("%1 穿戴装备：%2。").arg(role.name).arg(item.name));
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::acceptTask() {
    int row = taskList->currentRow();
    if (row < 0 || row >= tasks.size()) return;
    if (tasks[row].status != 0) {
        QMessageBox::information(this, "任务状态", "只能接受未接任务。");
        return;
    }
    tasks[row].status = 1;
    checkTaskCompletion();
    refreshTasks();
}

void MainWindow::claimTask() {
    int row = taskList->currentRow();
    if (row < 0 || row >= tasks.size()) return;
    if (tasks[row].status != 2) {
        QMessageBox::information(this, "任务状态", "只有已完成未领奖任务可以领取。");
        return;
    }
    gold += tasks[row].rewardGold;
    demonCoin += tasks[row].rewardDemonCoin;
    if (!tasks[row].rewardItem.isEmpty()) addItem(itemByName(tasks[row].rewardItem));
    tasks[row].status = 3;
    appendLog(QString("领取任务奖励：%1。").arg(tasks[row].name));
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::addTaskProgress(const QString& type, const QString& target, int amount) {
    for (auto& task : tasks) {
        if (task.status == 0 && task.id <= 5) task.status = 1;
        if (task.status != 1) continue;
        bool typeMatch = task.conditionType == type;
        bool targetMatch = task.target == target || task.target == "任意" || target == "任意";
        if (typeMatch && targetMatch) task.progress = std::min(task.need, task.progress + amount);
    }
    checkTaskCompletion();
}

void MainWindow::checkTaskCompletion() {
    for (auto& task : tasks) {
        if (task.status == 1 && task.progress >= task.need) {
            task.status = 2;
            appendLog(QString("任务完成：%1，可以领取奖励。").arg(task.name));
        }
    }
}

void MainWindow::enterDungeon() {
    if (phase != GamePhase::AngelShop) {
        QMessageBox::warning(this, "无法进入", "需要完成第二学期并进入天使商店后才能进入地窟。");
        return;
    }
    gold = 0;
    demonCoin += demonFriendBonus;
    phase = GamePhase::Dungeon;
    dungeonLayer = 1;
    currentRoom = 0;
    buildDungeonLayer(1);
    appendLog("离开天使商店，所有金币清空。进入7层神秘地窟。");
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::buildDungeonLayer(int layer) {
    rooms.clear();
    demonGoods.clear();
    auto consumables = QVector<ItemData>{
        {"手雷", ItemType::Consumable, "消耗品", 0, 18, "对敌方全体造成固定物理伤害。", 0, 0, 0, 18, 0, 0, 0, "", true},
        {"魔法手雷", ItemType::Consumable, "消耗品", 0, 20, "对敌方全体造成固定法术伤害。", 0, 0, 0, 0, 22, 0, 0, "", true},
        {"冰冻手雷", ItemType::Consumable, "消耗品", 0, 28, "对敌方全体造成伤害并附加冻结。", 0, 0, 0, 0, 18, 0, 0, "", true},
        {"神圣药水", ItemType::Consumable, "消耗品", 0, 24, "清除我方可清除Debuff并回复生命。", 45, 0, 0, 0, 0, 0, 0, "", true},
        {"狂暴药水", ItemType::Consumable, "消耗品", 0, 26, "提升使用者接下来3回合攻击力。", 0, 0, 0, 12, 12, 0, 0, "", true},
        {"护佑药水", ItemType::Consumable, "消耗品", 0, 24, "提升使用者接下来3回合防御和法抗。", 0, 0, 0, 0, 0, 12, 12, "", true},
        {"恶魔之友", ItemType::Growth, "养成", 0, 75, "永久增加进入地窟时获得的初始恶魔币数量。", 0, 0, 0, 0, 0, 0, 0, "", false}
    };
    demonGoods += consumables;
    demonGoods += makeLayerEquipments(layer);

    if (layer == 7) {
        rooms.push_back({RoomType::DemonShop, false, {}});
        rooms.push_back({RoomType::Boss, false, makeEnemyGroup(7, false, true)});
    } else {
        rooms.push_back({RoomType::DemonShop, false, {}});
        QVector<RoomType> rest = {RoomType::Chest, RoomType::Battle, RoomType::Battle, RoomType::Battle, RoomType::Battle, RoomType::EliteBattle};
        std::shuffle(rest.begin(), rest.end(), *QRandomGenerator::global());
        for (RoomType type : rest) {
            bool elite = type == RoomType::EliteBattle;
            QVector<EnemyData> enemies = (type == RoomType::Chest) ? QVector<EnemyData>{} : makeEnemyGroup(layer, elite, false);
            rooms.push_back({type, false, enemies});
        }
    }
    for (const auto& room : rooms) {
        for (const auto& enemy : room.enemies) {
            codex[enemy.name] = enemy;
            if (!encountered.contains(enemy.name)) encountered[enemy.name] = false;
        }
    }
}

QVector<MainWindow::EnemyData> MainWindow::makeEnemyGroup(int layer, bool elite, bool boss) const {
    if (boss) {
        return {{"终焉挂科龙", "首领", 7, 900, 900, 72, 45, 38, 36, 600, 120, false, true, true, 0, 0, 0, "终焉吐息：攻击全体并优先压低后排"}};
    }
    int hp = 70 + layer * 28;
    int atk = 12 + layer * 7;
    int def = 4 + layer * 3;
    QVector<EnemyData> group = {
        {QString("第%1层课堂游魂").arg(layer), "小怪", layer, hp, hp, atk, 0, def, def, 30 + layer * 12, 6 + layer * 3, false, false, false, 0, 0, 0, "粉笔投掷：普通攻击"},
        {QString("第%1层迟到巡查员").arg(layer), "小怪", layer, hp + 20, hp + 20, atk + 4, 0, def + 2, def, 35 + layer * 12, 7 + layer * 3, false, false, true, 0, 0, 0, "越过前排攻击后排"},
        {QString("第%1层作业怨念").arg(layer), "小怪", layer, hp - 10, hp - 10, atk, atk + 8, def, def + 3, 32 + layer * 12, 8 + layer * 3, false, false, false, 0, 0, 0, "精神压迫：法术伤害"}
    };
    if (elite) {
        group[0].name = QString("第%1层精英教导主任").arg(layer);
        group[0].kind = "精英";
        group[0].maxHp += 120;
        group[0].hp = group[0].maxHp;
        group[0].attack += 18;
        group[0].defense += 8;
        group[0].exp += 100;
        group[0].demonCoin += 35;
        group[0].elite = true;
        group[0].skills = "纪律压制：高伤害并有概率眩晕";
    }
    return group;
}

QVector<MainWindow::ItemData> MainWindow::makeLayerEquipments(int layer) const {
    int base = 10 + layer * 5;
    return {
        {QString("第%1层学生木剑").arg(layer), ItemType::Equipment, "装备", 0, 18 + layer * 4, "物攻装备，提升物理攻击。", 0, 0, 0, base, 0, 0, 0, "物攻", false},
        {QString("第%1层魔法粉笔").arg(layer), ItemType::Equipment, "装备", 0, 18 + layer * 4, "法攻装备，提升法术攻击。", 0, 0, 0, 0, base, 0, 0, "法攻", false},
        {QString("第%1层纪律护甲").arg(layer), ItemType::Equipment, "装备", 0, 18 + layer * 4, "物防装备，提升物理防御。", 0, 0, 0, 0, 0, base, 0, "物防", false},
        {QString("第%1层实验护符").arg(layer), ItemType::Equipment, "装备", 0, 18 + layer * 4, "法抗装备，提升法术抗性。", 0, 0, 0, 0, 0, 0, base, "法抗", false}
    };
}

void MainWindow::exploreRoom() {
    if (phase != GamePhase::Dungeon || dungeonLayer <= 0) {
        QMessageBox::warning(this, "无法探索", "请先从天使商店进入地窟。");
        return;
    }
    int row = dungeonRoomList->currentRow();
    if (row < 0 || row >= rooms.size()) row = currentRoom;
    currentRoom = row;
    RoomData& room = rooms[row];
    if (room.cleared) {
        QMessageBox::information(this, "已完成", "该房间已经完成。");
        return;
    }
    for (auto& role : party) role.vigor = std::max(0, role.vigor - 5);
    if (room.type == RoomType::DemonShop) {
        room.cleared = true;
        appendLog(QString("进入第%1层恶魔商店，交易和进入下一房间消耗体力。").arg(dungeonLayer));
    } else if (room.type == RoomType::Chest) {
        room.cleared = true;
        addTaskProgress("宝箱", "任意", 1);
        QVector<ItemData> equips = makeLayerEquipments(dungeonLayer);
        addItem(equips[QRandomGenerator::global()->bounded(equips.size())]);
        addItem(angelGoods[QRandomGenerator::global()->bounded(0, 6)]);
        for (auto& role : party) addExp(role, 80 + dungeonLayer * 25);
        appendLog(QString("开启第%1层宝箱房：获得随机物资、装备和经验。").arg(dungeonLayer));
    } else {
        startBattle(room.enemies, room.type == RoomType::EliteBattle, room.type == RoomType::Boss);
    }
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::startBattle(const QVector<EnemyData>& enemies, bool elite, bool boss) {
    battleEnemies = enemies;
    for (auto& e : battleEnemies) {
        e.hp = e.maxHp;
        encountered[e.name] = true;
    }
    inBattle = true;
    QString kind = boss ? "最终BOSS" : (elite ? "精英战斗" : "普通战斗");
    appendLog(QString("进入%1：%2。平均等级决定先后手，等级相同我方先手。").arg(kind).arg(battleEnemies.first().name));
}

void MainWindow::fightOneRound() {
    if (!inBattle) return;
    double playerAvg = 0;
    for (const auto& r : party) playerAvg += r.level;
    playerAvg = party.isEmpty() ? 0 : playerAvg / party.size();
    double enemyAvg = dungeonLayer + (battleEnemies.first().elite ? 1.5 : 0) + (battleEnemies.first().boss ? 4 : 0);
    bool playerFirst = playerAvg >= enemyAvg;

    auto playerTurn = [this]() {
        for (auto& role : party) {
            if (role.hp <= 0) continue;
            if (role.battleStun > 0) {
                role.battleStun--;
                appendLog(QString("%1 处于眩晕，无法行动。").arg(role.name));
                continue;
            }
            int enemyIndex = firstAliveEnemy();
            if (enemyIndex < 0) break;
            auto* prof = professionByName(role.profession);
            QString skillName = prof ? QString::fromStdString(prof->skills().front().name) : "普通攻击";
            int dmg = damage(std::max(role.physicalAttack, role.magicAttack), battleEnemies[enemyIndex].defense, 1.0);
            battleEnemies[enemyIndex].hp = std::max(0, battleEnemies[enemyIndex].hp - dmg);
            appendLog(QString("%1 使用 %2 攻击 %3，造成%4伤害。").arg(role.name).arg(skillName).arg(battleEnemies[enemyIndex].name).arg(dmg));
        }
    };

    if (playerFirst) {
        playerTurn();
        endBattleIfNeeded();
        if (inBattle) enemyTurn();
    } else {
        enemyTurn();
        endBattleIfNeeded();
        if (inBattle) playerTurn();
    }
    endBattleIfNeeded();
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::enemyTurn() {
    for (auto& enemy : battleEnemies) {
        if (enemy.hp <= 0) continue;
        if (enemy.frozen > 0) {
            enemy.frozen--;
            appendLog(QString("%1被冻结，跳过行动。").arg(enemy.name));
            continue;
        }
        if (enemy.slow > 0 && QRandomGenerator::global()->bounded(100) < 50) {
            enemy.slow--;
            appendLog(QString("%1受到迟缓影响，本回合无法行动。").arg(enemy.name));
            continue;
        }
        int target = firstAlivePlayer();
        for (int i = 0; i < party.size(); ++i) {
            if (party[i].tauntRounds > 0 && party[i].hp > 0) target = i;
        }
        if (enemy.backlineAttack && party.size() > 1) {
            for (int i = party.size() - 1; i >= 0; --i) {
                if (party[i].hp > 0) {
                    target = i;
                    break;
                }
            }
        }
        if (target < 0) return;
        int dmg = damage(enemy.attack + enemy.magicAttack / 2, party[target].physicalDefense, enemy.boss ? 1.25 : 1.0);
        party[target].hp = std::max(0, party[target].hp - dmg);
        appendLog(QString("%1 攻击 %2，造成%3伤害。").arg(enemy.name).arg(party[target].name).arg(dmg));
    }
    for (auto& role : party) {
        if (role.tauntRounds > 0) role.tauntRounds--;
    }
}

void MainWindow::endBattleIfNeeded() {
    if (aliveEnemyCount() == 0) {
        inBattle = false;
        RoomData& room = rooms[currentRoom];
        room.cleared = true;
        bool elite = room.type == RoomType::EliteBattle;
        bool boss = room.type == RoomType::Boss;
        int totalExp = 0;
        int totalCoin = 0;
        for (const auto& enemy : battleEnemies) {
            totalExp += enemy.exp;
            totalCoin += enemy.demonCoin;
        }
        demonCoin += totalCoin;
        for (auto& role : party) addExp(role, totalExp);
        QVector<ItemData> equips = makeLayerEquipments(elite ? std::min(7, dungeonLayer + 1) : dungeonLayer);
        int dropCount = elite ? 3 : 3;
        for (int i = 0; i < dropCount; ++i) addItem(equips[QRandomGenerator::global()->bounded(equips.size())]);
        addTaskProgress("战斗", "任意", 1);
        if (boss) {
            addTaskProgress("最终BOSS", "终焉挂科龙", 1);
            phase = GamePhase::Ending;
            QMessageBox::information(this, "通关结局", "打破诅咒，所有角色逃离轮回。");
        } else if (elite) {
            addTaskProgress("通层", QString::number(dungeonLayer), 1);
            if (dungeonLayer < 7) {
                dungeonLayer++;
                currentRoom = 0;
                buildDungeonLayer(dungeonLayer);
                appendLog(QString("精英房连接下一层通道，进入第%1层。").arg(dungeonLayer));
            }
        }
        appendLog(QString("战斗胜利：获得经验%1，恶魔币%2，掉落3件装备。").arg(totalExp).arg(totalCoin));
    } else if (alivePlayerCount() == 0) {
        inBattle = false;
        QMessageBox::warning(this, "地窟团灭", "地窟全体死亡，返回开学，保留进入地窟前的养成属性开启下一次轮回。");
        nextLoopAfterDeath();
    }
}

void MainWindow::nextLoopAfterDeath() {
    QVector<CharacterData> oldParty = party;
    int keepGold = initialGoldBonus;
    int keepVigor = initialVigorBonus;
    int keepWork = workGoldBonus;
    int keepDemon = demonFriendBonus;
    resetGameForNewRun(true);
    initialGoldBonus = keepGold;
    initialVigorBonus = keepVigor;
    workGoldBonus = keepWork;
    demonFriendBonus = keepDemon;
    for (auto role : oldParty) {
        role.hp = role.maxHp;
        role.mp = role.maxMp;
        role.battleStun = 0;
        role.tauntRounds = 0;
        party.push_back(role);
    }
    appendLog(QString("第%1次轮回开始：保留角色养成属性，恶魔币清空。").arg(loopCount));
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::useBattleMedicine() {
    if (!inBattle) return;
    int row = inventoryList->currentRow();
    if (row < 0 || row >= inventory.size()) return;
    ItemData item = inventory[row];
    if (!item.battleUsable || item.type == ItemType::Food) {
        QMessageBox::warning(this, "无法使用", "食品无法在战斗中使用，只有药品/战斗消耗品可用。");
        return;
    }
    int roleIndex = firstAlivePlayer();
    if (roleIndex < 0) return;
    CharacterData& role = party[roleIndex];
    if (item.name == "阿司匹林" || item.name == "神圣药水") {
        role.battleStun = 0;
    }
    role.hp = std::min(role.maxHp, role.hp + (item.hpRecover > 0 ? item.hpRecover : role.maxHp * 30 / 100));
    role.mp = std::min(role.maxMp, role.mp + item.mpRecover);
    role.physicalAttack += item.attackBonus;
    role.magicAttack += item.magicBonus;
    role.physicalDefense += item.defenseBonus;
    role.magicResistance += item.resistBonus;
    if (item.name.contains("手雷")) {
        for (auto& enemy : battleEnemies) {
            if (enemy.hp > 0) enemy.hp = std::max(0, enemy.hp - std::max(item.attackBonus, item.magicBonus));
            if (item.name == "冰冻手雷") enemy.frozen = 1;
        }
    }
    inventory.removeAt(row);
    appendLog(QString("%1 战斗中使用 %2，行动结束。").arg(role.name).arg(item.name));
    endBattleIfNeeded();
    if (inBattle) enemyTurn();
    endBattleIfNeeded();
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::setFormationOneFront() {
    formationType = 1;
    formationChanged = true;
    addTaskProgress("编队", "任意", 1);
    refreshAll();
}

void MainWindow::setFormationTwoFront() {
    formationType = 2;
    formationChanged = true;
    addTaskProgress("编队", "任意", 1);
    refreshAll();
}

void MainWindow::moveRoleUp() {
    int row = characterList->currentRow();
    if (row > 0 && row < party.size()) {
        std::swap(party[row], party[row - 1]);
        formationChanged = true;
        addTaskProgress("编队", "任意", 1);
        refreshAll();
    }
}

void MainWindow::moveRoleDown() {
    int row = characterList->currentRow();
    if (row >= 0 && row + 1 < party.size()) {
        std::swap(party[row], party[row + 1]);
        formationChanged = true;
        addTaskProgress("编队", "任意", 1);
        refreshAll();
    }
}

int MainWindow::alivePlayerCount() const {
    int count = 0;
    for (const auto& role : party) if (role.hp > 0) count++;
    return count;
}

int MainWindow::aliveEnemyCount() const {
    int count = 0;
    for (const auto& enemy : battleEnemies) if (enemy.hp > 0) count++;
    return count;
}

int MainWindow::firstAliveEnemy() const {
    for (int i = 0; i < battleEnemies.size(); ++i) if (battleEnemies[i].hp > 0) return i;
    return -1;
}

int MainWindow::firstAlivePlayer() const {
    for (int i = 0; i < party.size(); ++i) if (party[i].hp > 0) return i;
    return -1;
}

int MainWindow::damage(int attack, int defense, double scale) const {
    return std::max(1, int((attack - defense) * scale));
}

QString MainWindow::itemTypeText(ItemType type) const {
    switch (type) {
    case ItemType::Medicine: return "药品";
    case ItemType::Food: return "食品";
    case ItemType::Growth: return "养成";
    case ItemType::Consumable: return "消耗品";
    case ItemType::Equipment: return "装备";
    }
    return "未知";
}

QString MainWindow::phaseText() const {
    switch (phase) {
    case GamePhase::SaveSelect: return "存档选择";
    case GamePhase::SchoolFirst: return "第一学期排课";
    case GamePhase::Winter: return "寒假选择";
    case GamePhase::SchoolSecond: return "第二学期排课";
    case GamePhase::AngelShop: return "暑假天使商店";
    case GamePhase::Dungeon: return "神秘地窟";
    case GamePhase::Ending: return "已通关结局";
    }
    return "未知";
}

QString MainWindow::taskStatusText(int status) const {
    if (status == 0) return "未接";
    if (status == 1) return "已接未完成";
    if (status == 2) return "已完成未领奖";
    return "已领奖";
}

QString MainWindow::roleText(const CharacterData& role) const {
    return QString("%1（%2）Lv.%3 EXP:%4/%5 HP:%6/%7 MP:%8/%9 物攻:%10 法攻:%11 物防:%12 法抗:%13 活力:%14")
        .arg(role.name).arg(role.profession).arg(role.level).arg(role.exp).arg(role.expNeed)
        .arg(role.hp).arg(role.maxHp).arg(role.mp).arg(role.maxMp)
        .arg(role.physicalAttack).arg(role.magicAttack).arg(role.physicalDefense).arg(role.magicResistance).arg(role.vigor);
}

QString MainWindow::skillsText(const CharacterData& role) const {
    auto* prof = professionByName(role.profession);
    if (!prof) return "无";
    QStringList lines;
    for (const auto& skill : prof->skills()) {
        QString lock = role.level >= skill.unlockLevel ? "已解锁" : QString("Lv.%1解锁").arg(skill.unlockLevel);
        lines << QString("%1 [%2] MP%3 HP%4% | %5")
            .arg(QString::fromStdString(skill.name)).arg(lock).arg(skill.mpCost).arg(skill.hpCostPercent)
            .arg(QString::fromStdString(skill.description));
    }
    return lines.join("\n");
}

QString MainWindow::inventoryText(const ItemData& item) const {
    QString slot = item.equipSlot.isEmpty() ? "" : QString(" | 槽位：%1").arg(item.equipSlot);
    QString price = item.demonPrice > 0 ? QString(" | 恶魔币价：%1").arg(item.demonPrice) : QString(" | 金币价：%1").arg(item.price);
    return QString("[%1/%2] %3%4%5 | %6")
        .arg(item.category).arg(itemTypeText(item.type)).arg(item.name).arg(slot).arg(price).arg(item.effect);
}

MainWindow::ItemData MainWindow::itemByName(const QString& name) const {
    for (const auto& item : angelGoods) if (item.name == name) return item;
    for (const auto& item : demonGoods) if (item.name == name) return item;
    for (int layer = 1; layer <= 7; ++layer) {
        for (const auto& item : makeLayerEquipments(layer)) if (item.name == name) return item;
    }
    if (name == "第一层学生木剑") return makeLayerEquipments(1).first();
    return {"未知物品", ItemType::Food, "未知", 0, 0, "存档中存在但配置表未找到的物品。", 0, 0, 0, 0, 0, 0, 0, "", false};
}

Profession* MainWindow::professionByName(const QString& name) const {
    for (const auto& prof : professions) {
        if (QString::fromStdString(prof->name()) == name) return prof.get();
    }
    return nullptr;
}

QString MainWindow::serializeInventory() const {
    QStringList names;
    for (const auto& item : inventory) names << item.name;
    return names.join("||");
}

void MainWindow::deserializeInventory(const QString& text) {
    inventory.clear();
    if (text.isEmpty()) return;
    for (const QString& name : text.split("||", Qt::SkipEmptyParts)) inventory.push_back(itemByName(name));
}

QString MainWindow::serializeCharacters() const {
    QStringList rows;
    for (const auto& role : party) {
        QStringList equipRows;
        for (auto it = role.equipment.begin(); it != role.equipment.end(); ++it) {
            equipRows << it.key() + "=" + it.value();
        }
        rows << QStringList{
            role.name, role.profession, QString::number(role.level), QString::number(role.exp), QString::number(role.expNeed),
            QString::number(role.hp), QString::number(role.maxHp), QString::number(role.mp), QString::number(role.maxMp),
            QString::number(role.vigor), QString::number(role.physicalAttack), QString::number(role.magicAttack),
            QString::number(role.physicalDefense), QString::number(role.magicResistance), equipRows.join(",")
        }.join("|");
    }
    return rows.join("\n");
}

void MainWindow::deserializeCharacters(const QString& text) {
    party.clear();
    for (const QString& row : text.split("\n", Qt::SkipEmptyParts)) {
        QStringList p = row.split("|");
        if (p.size() < 14) continue;
        CharacterData role;
        role.name = p.value(0);
        role.profession = p.value(1);
        role.level = p.value(2).toInt();
        role.exp = p.value(3).toInt();
        role.expNeed = p.value(4).toInt();
        role.hp = p.value(5).toInt();
        role.maxHp = p.value(6).toInt();
        role.mp = p.value(7).toInt();
        role.maxMp = p.value(8).toInt();
        role.vigor = p.value(9).toInt();
        role.physicalAttack = p.value(10).toInt();
        role.magicAttack = p.value(11).toInt();
        role.physicalDefense = p.value(12).toInt();
        role.magicResistance = p.value(13).toInt();
        for (const QString& equip : p.value(14).split(",", Qt::SkipEmptyParts)) {
            QStringList kv = equip.split("=");
            if (kv.size() == 2) role.equipment[kv[0]] = kv[1];
        }
        party.push_back(role);
    }
}

QString MainWindow::serializeTasks() const {
    QStringList rows;
    for (const auto& task : tasks) {
        rows << QString("%1,%2,%3").arg(task.id).arg(task.progress).arg(task.status);
    }
    return rows.join("|");
}

void MainWindow::deserializeTasks(const QString& text) {
    if (text.isEmpty()) return;
    QMap<int, QStringList> saved;
    for (const QString& row : text.split("|", Qt::SkipEmptyParts)) {
        QStringList p = row.split(",");
        if (p.size() == 3) saved[p[0].toInt()] = p;
    }
    for (auto& task : tasks) {
        if (!saved.contains(task.id)) continue;
        task.progress = saved[task.id][1].toInt();
        task.status = saved[task.id][2].toInt();
    }
}

QString MainWindow::serializeCodex() const {
    QStringList names;
    for (auto it = encountered.begin(); it != encountered.end(); ++it) {
        if (it.value()) names << it.key();
    }
    return names.join("||");
}

void MainWindow::deserializeCodex(const QString& text) {
    for (const QString& name : text.split("||", Qt::SkipEmptyParts)) encountered[name] = true;
}
