#include "MainWindow.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSettings>
#include <QSplitter>
#include <QTableWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <algorithm>

namespace {

const QStringList kRoles = QStringList({"学生", "冰法师", "圣骑士", "祈福者", "血战士", "魔术师"});
const QStringList kWeekDays = QStringList({"周一", "周二", "周三", "周四", "周五", "周六", "周日"});
const QStringList kHalfDays = QStringList({"上午", "下午"});
const QStringList kActions = QStringList({
    "大物课", "语文课", "外语课", "化学课", "高数课", "专业课", "体育课",
    "不上课", "打工", "补习班+大物课", "补习班+语文课", "补习班+外语课",
    "补习班+化学课", "补习班+高数课", "补习班+专业课", "补习班+体育课"
});
const QStringList kWeekendActions = QStringList({
    "不上课", "打工", "补习班+大物课", "补习班+语文课", "补习班+外语课",
    "补习班+化学课", "补习班+高数课", "补习班+专业课", "补习班+体育课"
});
const QStringList kFloorNames = QStringList({
    "", "废弃体育馆", "倒悬图书馆", "镜面戏剧社", "失序化学实验室",
    "占卜社旧址", "校长室前厅", "真理篡改者王座"
});

int clampInt(int value, int low, int high) {
    return std::max(low, std::min(value, high));
}

QString yesNo(bool value) {
    return value ? "是" : "否";
}

QString itemLine(const MainWindow::ItemData& item) {
    return QString("%1 [%2] %3币 - %4")
        .arg(item.name, item.type)
        .arg(item.price)
        .arg(item.desc);
}

}  // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("校园RPG冒险游戏 - 轮回地窟版");
    setMinimumSize(1180, 760);
    resize(1280, 820);
    initializeCatalogs();
    clearGameState();
    setupUi();
    applyStyle();
    refreshAll();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    tabs = new QTabWidget(this);
    setCentralWidget(tabs);
    setupSaveTab();
    setupRoleTab();
    setupScheduleTab();
    setupAngelShopTab();
    setupDungeonTab();
    setupFormationTab();
    setupInventoryTab();
    setupJournalTab();
    setupQuestTab();
    tabs->setCurrentIndex(0);
}

void MainWindow::setupSaveTab() {
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 24, 28, 24);
    auto* title = new QLabel("存档选择", page);
    title->setObjectName("PageTitle");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);
    auto* hint = new QLabel("最多 4 个独立存档。空槽新建时会依次为六名角色命名；已有存档可以读取、保存或删除。", page);
    hint->setWordWrap(true);
    hint->setAlignment(Qt::AlignCenter);
    layout->addWidget(hint);
    saveList = new QListWidget(page);
    saveList->setAlternatingRowColors(true);
    layout->addWidget(saveList, 1);
    auto* row = new QHBoxLayout();
    newSaveBtn = new QPushButton("新建存档", page);
    loadSaveBtn = new QPushButton("读取存档", page);
    saveCurrentBtn = new QPushButton("保存当前", page);
    deleteSaveBtn = new QPushButton("删除存档", page);
    row->addWidget(newSaveBtn);
    row->addWidget(loadSaveBtn);
    row->addWidget(saveCurrentBtn);
    row->addWidget(deleteSaveBtn);
    layout->addLayout(row);
    connect(newSaveBtn, &QPushButton::clicked, this, &MainWindow::onNewSave);
    connect(loadSaveBtn, &QPushButton::clicked, this, &MainWindow::onLoadSave);
    connect(saveCurrentBtn, &QPushButton::clicked, this, &MainWindow::onSaveCurrent);
    connect(deleteSaveBtn, &QPushButton::clicked, this, &MainWindow::onDeleteSave);
    connect(saveList, &QListWidget::itemDoubleClicked, this, [this]() {
        if (saveList->currentRow() >= 0 && saveExists(saveList->currentRow() + 1)) onLoadSave();
        else onNewSave();
    });
    tabs->addTab(page, "存档");
}

void MainWindow::setupRoleTab() {
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);
    roleTable = new QTableWidget(page);
    roleTable->setColumnCount(11);
    roleTable->setHorizontalHeaderLabels(QStringList({"姓名", "职业", "等级", "经验", "生命", "蓝量", "活力", "物攻", "法攻", "物防", "法抗"}));
    roleTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    roleTable->verticalHeader()->setVisible(false);
    roleTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    roleTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(roleTable, 2);
    skillText = new QTextEdit(page);
    skillText->setReadOnly(true);
    layout->addWidget(skillText, 1);
    tabs->addTab(page, "角色");
}

void MainWindow::setupScheduleTab() {
    auto* page = new QWidget(this);
    auto* root = new QVBoxLayout(page);
    auto* top = new QHBoxLayout();
    top->addWidget(new QLabel("排课角色：", page));
    scheduleCharacterBox = new QComboBox(page);
    top->addWidget(scheduleCharacterBox, 1);
    auto* applyBtn = new QPushButton("执行 20 周学期", page);
    auto* restBtn = new QPushButton("寒假休息", page);
    auto* workBtn = new QPushButton("寒假打工", page);
    top->addWidget(applyBtn);
    top->addWidget(restBtn);
    top->addWidget(workBtn);
    root->addLayout(top);
    scheduleTable = new QTableWidget(7, 2, page);
    scheduleTable->setHorizontalHeaderLabels(kHalfDays);
    scheduleTable->setVerticalHeaderLabels(kWeekDays);
    scheduleTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    scheduleTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    root->addWidget(scheduleTable, 1);
    scheduleSummaryLabel = new QLabel(page);
    scheduleSummaryLabel->setObjectName("StatusPill");
    scheduleSummaryLabel->setWordWrap(true);
    root->addWidget(scheduleSummaryLabel);
    connect(scheduleCharacterBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        loadScheduleToTable(index);
        refreshScheduleSummary();
    });
    connect(applyBtn, &QPushButton::clicked, this, &MainWindow::onApplySchedule);
    connect(restBtn, &QPushButton::clicked, this, &MainWindow::onWinterRest);
    connect(workBtn, &QPushButton::clicked, this, &MainWindow::onWinterWork);
    tabs->addTab(page, "课表");
}

void MainWindow::setupAngelShopTab() {
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);
    angelGoldLabel = new QLabel(page);
    angelGoldLabel->setObjectName("StatusPill");
    layout->addWidget(angelGoldLabel);
    angelShopList = new QListWidget(page);
    angelShopList->setAlternatingRowColors(true);
    layout->addWidget(angelShopList, 1);
    auto* row = new QHBoxLayout();
    auto* buyBtn = new QPushButton("购买选中物品", page);
    auto* leaveBtn = new QPushButton("离开商店并清空金币", page);
    row->addWidget(buyBtn);
    row->addWidget(leaveBtn);
    layout->addLayout(row);
    connect(buyBtn, &QPushButton::clicked, this, &MainWindow::onBuyAngelItem);
    connect(leaveBtn, &QPushButton::clicked, this, [this]() {
        if (!hasGame) return;
        gold = 0;
        completeQuest("angel");
        appendLog("离开天使商店，未花完的打工金币被清空。");
        refreshAll();
    });
    tabs->addTab(page, "天使商店");
}

void MainWindow::setupDungeonTab() {
    auto* page = new QWidget(this);
    auto* root = new QVBoxLayout(page);
    dungeonStateLabel = new QLabel(page);
    dungeonStateLabel->setObjectName("StatusPill");
    dungeonStateLabel->setWordWrap(true);
    root->addWidget(dungeonStateLabel);
    auto* splitter = new QSplitter(Qt::Horizontal, page);
    dungeonLog = new QTextEdit(splitter);
    dungeonLog->setReadOnly(true);
    demonShopList = new QListWidget(splitter);
    demonShopList->setAlternatingRowColors(true);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);
    root->addWidget(splitter, 1);
    auto* row = new QHBoxLayout();
    enterDungeonBtn = new QPushButton("进入地窟", page);
    nextRoomBtn = new QPushButton("探索下一房间", page);
    treasureBtn = new QPushButton("开启宝箱", page);
    demonShopBtn = new QPushButton("恶魔交易说明", page);
    auto* buyDemonBtn = new QPushButton("购买恶魔商品", page);
    row->addWidget(enterDungeonBtn);
    row->addWidget(nextRoomBtn);
    row->addWidget(treasureBtn);
    row->addWidget(demonShopBtn);
    row->addWidget(buyDemonBtn);
    root->addLayout(row);
    connect(enterDungeonBtn, &QPushButton::clicked, this, &MainWindow::onEnterDungeon);
    connect(nextRoomBtn, &QPushButton::clicked, this, &MainWindow::onNextRoom);
    connect(treasureBtn, &QPushButton::clicked, this, &MainWindow::onOpenTreasure);
    connect(demonShopBtn, &QPushButton::clicked, this, &MainWindow::onDemonShop);
    connect(buyDemonBtn, &QPushButton::clicked, this, &MainWindow::onBuyDemonItem);
    tabs->addTab(page, "地窟");
}

void MainWindow::setupFormationTab() {
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);
    formationBox = new QComboBox(page);
    formationBox->addItems(QStringList({"前排 1 人 + 后排 2 人", "前排 2 人 + 后排 1 人"}));
    layout->addWidget(formationBox);
    formationList = new QListWidget(page);
    formationList->setAlternatingRowColors(true);
    layout->addWidget(formationList, 1);
    auto* row = new QHBoxLayout();
    auto* toggleBtn = new QPushButton("上阵/下阵", page);
    auto* applyBtn = new QPushButton("应用阵型", page);
    row->addWidget(toggleBtn);
    row->addWidget(applyBtn);
    layout->addLayout(row);
    connect(toggleBtn, &QPushButton::clicked, this, &MainWindow::onToggleActive);
    connect(applyBtn, &QPushButton::clicked, this, &MainWindow::onSetFormation);
    tabs->addTab(page, "编队");
}

void MainWindow::setupInventoryTab() {
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);
    inventorySummaryLabel = new QLabel(page);
    inventorySummaryLabel->setObjectName("StatusPill");
    layout->addWidget(inventorySummaryLabel);
    inventoryList = new QListWidget(page);
    inventoryList->setAlternatingRowColors(true);
    layout->addWidget(inventoryList, 1);
    auto* row = new QHBoxLayout();
    auto* useBtn = new QPushButton("使用选中物品", page);
    auto* dropBtn = new QPushButton("丢弃选中物品", page);
    row->addWidget(useBtn);
    row->addWidget(dropBtn);
    layout->addLayout(row);
    connect(useBtn, &QPushButton::clicked, this, &MainWindow::onUseInventoryItem);
    connect(dropBtn, &QPushButton::clicked, this, &MainWindow::onDropInventoryItem);
    tabs->addTab(page, "背包");
}

void MainWindow::setupJournalTab() {
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);
    journalText = new QTextEdit(page);
    journalText->setReadOnly(true);
    layout->addWidget(journalText);
    tabs->addTab(page, "轮回游记");
}

void MainWindow::setupQuestTab() {
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);
    questList = new QListWidget(page);
    layout->addWidget(questList);
    tabs->addTab(page, "任务");
}

void MainWindow::applyStyle() {
    qApp->setStyleSheet(
        "QMainWindow{background:#f5f7fb;color:#1f2933;}"
        "QTabWidget::pane{border:1px solid #cfd7e6;background:white;}"
        "QTabBar::tab{padding:9px 14px;background:#e7ecf5;border:1px solid #cfd7e6;}"
        "QTabBar::tab:selected{background:white;border-bottom-color:white;font-weight:600;}"
        "QLabel#PageTitle{font-size:24px;font-weight:700;color:#182433;}"
        "QLabel#StatusPill{background:#eef6ff;border:1px solid #bad7ff;border-radius:6px;padding:9px;color:#17324d;}"
        "QPushButton{padding:8px 12px;border:1px solid #9fb0c5;border-radius:6px;background:#ffffff;}"
        "QPushButton:hover{background:#edf4ff;}"
        "QListWidget,QTableWidget,QTextEdit,QComboBox{background:#ffffff;border:1px solid #cfd7e6;border-radius:6px;padding:4px;}"
        "QHeaderView::section{background:#edf1f7;padding:6px;border:0;border-right:1px solid #cfd7e6;}");
}

void MainWindow::initializeCatalogs() {
    angelCatalog = {
        {"肾上腺素", "药品", "战斗中瞬间回复大量生命，但之后会结算副作用。", 120, true},
        {"速回药", "药品", "立即回复生命和蓝量。", 90, true},
        {"回血药", "药品", "按百分比回复生命。", 55, true},
        {"回蓝药", "药品", "按百分比回复蓝量。", 55, true},
        {"绷带", "药品", "三回合内持续回复生命。", 45, true},
        {"阿司匹林", "药品", "三回合内持续回复蓝量。", 45, true},
        {"巨无霸牛肉饭", "食品", "非战斗使用，回复大量活力。", 70, false},
        {"咖喱猪排饭", "食品", "非战斗使用，回复中量活力。", 55, false},
        {"校园面包", "食品", "非战斗使用，回复少量活力。", 25, false},
        {"特制海鲜粥", "食品", "非战斗使用，回复大量生命并回复少量活力。", 80, false},
        {"妈妈秘制蛋花汤", "食品", "非战斗使用，回复中量生命和少量活力。", 60, false},
        {"热腾腾的肉包子", "食品", "非战斗使用，回复少量生命和活力。", 35, false},
        {"魔法教授的手冲咖啡", "食品", "非战斗使用，回复大量蓝量和少量活力。", 75, false},
        {"波子汽水", "食品", "非战斗使用，回复中量蓝量和少量活力。", 45, false},
        {"罐装浓茶", "食品", "非战斗使用，回复少量蓝量和活力。", 35, false},
        {"活力少年", "养成", "永久增加全员每周初始活力。", 180, false},
        {"家财万贯", "养成", "每次轮回获得更多初始金币。", 220, false},
        {"高效打工人", "养成", "提高打工和寒假打工收益。", 200, false}
    };
    demonCatalog = {
        {"手雷", "恶魔消耗品", "造成物理伤害并降低目标物防。", 60, true},
        {"魔法手雷", "恶魔消耗品", "造成法术伤害并降低目标法抗。", 60, true},
        {"冰冻手雷", "恶魔消耗品", "造成法术伤害并冰冻目标。", 75, true},
        {"神圣药水", "恶魔消耗品", "清除负面状态并短暂抵抗 debuff。", 90, true},
        {"狂暴药水", "恶魔消耗品", "三回合提升基础攻击力。", 85, true},
        {"护佑药水", "恶魔消耗品", "三回合提升防御和法抗。", 85, true},
        {"恶魔之友", "养成", "进入地窟时获得更多初始恶魔币。", 180, false}
    };
}

void MainWindow::createDefaultSchedule() {
    schedules.clear();
    for (int i = 0; i < 6; ++i) {
        QStringList one;
        for (int cell = 0; cell < 14; ++cell) one << (cell >= 10 ? "不上课" : "专业课");
        schedules << one;
    }
}

void MainWindow::createInitialParty(const QStringList& names) {
    party.clear();
    for (int i = 0; i < kRoles.size(); ++i) {
        CharacterData c;
        c.name = names.value(i).trimmed().isEmpty() ? kRoles[i] : names[i].trimmed();
        c.role = kRoles[i];
        c.active = i < 3;
        c.position = i == 0 ? "前排" : "后排";
        if (c.role == "学生") {
            c.desc = "擅长学习，战斗基础较弱，但成长收益高。";
            c.maxHp = 120; c.maxMp = 35; c.maxStamina = 115; c.physAtk = 12; c.magicAtk = 8; c.physDef = 7; c.magicRes = 7;
            c.skills = QStringList({"含怒一击：125%物理伤害，消耗生命", "吃我一棒：100%物理伤害", "扫膛腿：50%物理群伤，消耗生命"});
        } else if (c.role == "冰法师") {
            c.desc = "低生命、高法攻、高蓝量，擅长冰冻与减速。";
            c.maxHp = 82; c.maxMp = 130; c.physAtk = 6; c.magicAtk = 20; c.physDef = 4; c.magicRes = 12;
            c.skills = QStringList({"冰锥术：100%法术伤害并回蓝", "冰冻术：80%法伤并冰冻（5级）", "凛冬赐福：攻击附带迟缓（10级）", "凛冬领域：群伤与控制（15级）", "凛冬降临：强力全场冻结（20级）"});
        } else if (c.role == "圣骑士") {
            c.desc = "高生命和防御，负责保护队友。";
            c.maxHp = 160; c.maxMp = 75; c.maxStamina = 110; c.physAtk = 10; c.magicAtk = 8; c.physDef = 16; c.magicRes = 14;
            c.skills = QStringList({"正义审判：100%物理伤害", "庇护坚壁：提升自身防御（5级）", "舍己：吸引攻击（10级）", "致盲：令敌方无法攻击（15级）", "圣光领域：真实伤害与破防（20级）"});
        } else if (c.role == "祈福者") {
            c.desc = "治疗者，生命较低，负责回复与解除负面状态。";
            c.maxHp = 90; c.maxMp = 145; c.physAtk = 5; c.magicAtk = 16; c.physDef = 5; c.magicRes = 14;
            c.skills = QStringList({"祈福：回复队友生命", "祈愿：回复队友蓝量（5级）", "祈甘霖：群体治疗（10级）", "祈无忧：转移 debuff（15级）", "祈命：濒死满血保护（20级）"});
        } else if (c.role == "血战士") {
            c.desc = "高攻击高生命低防御，通过燃血换取爆发。";
            c.maxHp = 150; c.maxMp = 45; c.maxStamina = 110; c.physAtk = 19; c.magicAtk = 5; c.physDef = 6; c.magicRes = 5;
            c.skills = QStringList({"燃血：消耗生命造成120%物理伤害", "沸血：获得额外行动（5级）", "窃血：攻击并吸血（10级）", "暴血：高额群伤并眩晕（15级）", "无我：低血终极爆发（20级）"});
        } else {
            c.desc = "高攻击、低防御、低生命，擅长诡术控制。";
            c.maxHp = 86; c.maxMp = 120; c.physAtk = 16; c.magicAtk = 18; c.physDef = 4; c.magicRes = 8;
            c.skills = QStringList({"火焰魔术：随机目标持续灼烧", "镜面魔术：为队友制造护盾（5级）", "箱中魔术：困住敌人（10级）", "障眼魔术：双方停滞（15级）", "同命巫术：伤害结算反噬（20级）"});
        }
        c.hp = c.maxHp;
        c.mp = c.maxMp;
        c.stamina = c.maxStamina;
        party << c;
    }
    createDefaultSchedule();
}

void MainWindow::clearGameState() {
    party.clear();
    preDungeonParty.clear();
    inventory.clear();
    preDungeonInventory.clear();
    knownEnemies.clear();
    completedQuests.clear();
    createDefaultSchedule();
    activeSaveSlot = -1;
    gold = 0;
    demonCoins = 0;
    loopCount = 1;
    termCount = 1;
    currentFloor = 0;
    currentRoom = 0;
    eliteRoom = 6;
    chestRoom = 3;
    formationMode = 0;
    hasGame = false;
    inDungeon = false;
    preDungeonSnapshotValid = false;
}

void MainWindow::refreshAll() {
    refreshSaveList();
    refreshRoleTable();
    refreshScheduleSummary();
    refreshAngelShop();
    refreshDungeonView();
    refreshFormationView();
    refreshInventoryList();
    refreshJournal();
    refreshQuestList();
}

void MainWindow::saveScheduleFromTable() {
    if (!scheduleTable || !scheduleCharacterBox) return;
    int index = scheduleCharacterBox->currentIndex();
    if (index < 0 || index >= schedules.size()) return;
    schedules[index].clear();
    for (int r = 0; r < 7; ++r) {
        for (int c = 0; c < 2; ++c) {
            auto* combo = qobject_cast<QComboBox*>(scheduleTable->cellWidget(r, c));
            schedules[index] << (combo ? combo->currentText() : "不上课");
        }
    }
}

void MainWindow::loadScheduleToTable(int characterIndex) {
    if (!scheduleTable) return;
    scheduleTable->blockSignals(true);
    for (int r = 0; r < 7; ++r) {
        for (int c = 0; c < 2; ++c) {
            auto* combo = new QComboBox(scheduleTable);
            combo->addItems(r >= 5 ? kWeekendActions : kActions);
            QString value = "不上课";
            if (characterIndex >= 0 && characterIndex < schedules.size()) value = schedules[characterIndex].value(r * 2 + c, value);
            int found = combo->findText(value);
            combo->setCurrentIndex(found >= 0 ? found : 0);
            connect(combo, &QComboBox::currentTextChanged, this, [this]() {
                saveScheduleFromTable();
                refreshScheduleSummary();
            });
            scheduleTable->setCellWidget(r, c, combo);
        }
    }
    scheduleTable->blockSignals(false);
}

void MainWindow::refreshSaveList() {
    if (!saveList) return;
    saveList->clear();
    for (int slot = 1; slot <= 4; ++slot) {
        QString text = saveSummary(slot);
        if (slot == activeSaveSlot) text += "  ← 当前";
        auto* item = new QListWidgetItem(text, saveList);
        item->setData(Qt::UserRole, slot);
    }
}

void MainWindow::refreshRoleTable() {
    if (!roleTable || !skillText) return;
    roleTable->setRowCount(party.size());
    QString details;
    for (int r = 0; r < party.size(); ++r) {
        const auto& c = party[r];
        QStringList values = QStringList({
            c.name, c.role, QString::number(c.level), QString::number(c.exp),
            QString("%1/%2").arg(c.hp).arg(c.maxHp), QString("%1/%2").arg(c.mp).arg(c.maxMp),
            QString("%1/%2").arg(c.stamina).arg(c.maxStamina), QString::number(c.physAtk),
            QString::number(c.magicAtk), QString::number(c.physDef), QString::number(c.magicRes)
        });
        for (int col = 0; col < values.size(); ++col) roleTable->setItem(r, col, new QTableWidgetItem(values[col]));
        details += QString("【%1 - %2】%3\n技能：%4\n\n").arg(c.name, c.role, c.desc, c.skills.join("；"));
    }
    if (party.isEmpty()) details = "请先在“存档”页新建或读取一个存档。";
    skillText->setPlainText(details);
    if (scheduleCharacterBox) {
        int previous = scheduleCharacterBox->currentIndex();
        scheduleCharacterBox->blockSignals(true);
        scheduleCharacterBox->clear();
        for (const auto& c : party) scheduleCharacterBox->addItem(c.name + " / " + c.role);
        if (!party.isEmpty()) scheduleCharacterBox->setCurrentIndex(clampInt(previous, 0, party.size() - 1));
        scheduleCharacterBox->blockSignals(false);
        loadScheduleToTable(scheduleCharacterBox->currentIndex());
    }
}

void MainWindow::refreshScheduleSummary() {
    if (!scheduleSummaryLabel) return;
    int weeklyGold = 0;
    int weeklyStamina = 0;
    int tutors = 0;
    for (int r = 0; r < 7; ++r) {
        for (int c = 0; c < 2; ++c) {
            auto* combo = scheduleTable ? qobject_cast<QComboBox*>(scheduleTable->cellWidget(r, c)) : nullptr;
            QString action = combo ? combo->currentText() : "不上课";
            if (action == "打工") { weeklyGold += 35; weeklyStamina -= 14; }
            else if (action == "不上课") weeklyStamina += 10;
            else if (action.startsWith("补习班")) { weeklyGold -= 18; weeklyStamina -= 5; ++tutors; }
            else weeklyStamina -= (action == "体育课" ? 12 : 8);
        }
    }
    scheduleSummaryLabel->setText(QString("周收益金币：%1；周活力变化：%2；补习半效课程：%3。每名角色独立排课，执行时重复 20 周。")
        .arg(weeklyGold)
        .arg(weeklyStamina >= 0 ? "+" + QString::number(weeklyStamina) : QString::number(weeklyStamina))
        .arg(tutors));
}

void MainWindow::refreshAngelShop() {
    if (!angelShopList || !angelGoldLabel) return;
    angelGoldLabel->setText(QString("金币：%1；背包：%2/40；离开天使商店会清空未花完金币。").arg(gold).arg(inventory.size()));
    angelShopList->clear();
    for (const auto& item : angelCatalog) angelShopList->addItem(itemLine(item));
}

void MainWindow::refreshDungeonView() {
    if (!dungeonStateLabel || !demonShopList) return;
    dungeonStateLabel->setText(QString("轮回：%1；学期：%2；地窟：%3；房间：%4；恶魔币：%5；战斗力：%6")
        .arg(loopCount).arg(termCount)
        .arg(inDungeon ? QString("第%1层 %2").arg(currentFloor).arg(kFloorNames.value(currentFloor)) : "未进入")
        .arg(currentRoomType()).arg(demonCoins).arg(partyPower()));
    demonShopList->clear();
    for (const auto& item : demonCatalog) demonShopList->addItem(itemLine(item));
    for (int f = 1; f <= 7; ++f) demonShopList->addItem(itemLine(equipmentForFloor(f)));
}

void MainWindow::refreshFormationView() {
    if (!formationList || !formationBox) return;
    formationBox->setCurrentIndex(formationMode);
    formationList->clear();
    for (int i = 0; i < party.size(); ++i) {
        const auto& c = party[i];
        formationList->addItem(QString("%1. %2 / %3 / %4 / %5").arg(i + 1).arg(c.name, c.role, c.active ? "上阵" : "替补", c.position));
    }
}

void MainWindow::refreshInventoryList() {
    if (!inventoryList || !inventorySummaryLabel) return;
    inventorySummaryLabel->setText(QString("背包容量：%1/40；药品可战斗使用，食品只能在非战斗状态使用。").arg(inventory.size()));
    inventoryList->clear();
    for (int i = 0; i < inventory.size(); ++i) {
        const auto& item = inventory[i];
        inventoryList->addItem(QString("%1. %2 [%3] 战斗可用：%4 - %5").arg(i + 1).arg(item.name, item.type, yesNo(item.battleUsable), item.desc));
    }
}

void MainWindow::refreshJournal() {
    if (!journalText) return;
    QString text = "轮回游记 / 怪物百科\n\n";
    for (int f = 1; f <= 7; ++f) {
        QString normal = enemyNameForFloor(f, false);
        QString elite = enemyNameForFloor(f, true);
        text += QString("第%1层 %2\n普通敌人：%3\n精英/首领：%4\n\n")
            .arg(f).arg(kFloorNames.value(f))
            .arg(knownEnemies.contains(normal) ? normal + " - 已记录攻击方式和掉落" : "???")
            .arg(knownEnemies.contains(elite) ? elite + " - 已记录特殊机制" : "???");
    }
    journalText->setPlainText(text);
}

void MainWindow::refreshQuestList() {
    if (!questList) return;
    questList->clear();
    QVector<QPair<QString, QString>> quests = {
        {"name", "完成六名角色命名：奖励初始金币"},
        {"angel", "天使商店教学：金币清空、食品与药品差异"},
        {"demon", "恶魔商店教学：购买补给并获得第1层装备"},
        {"equip", "装备教学：获得任意装备"},
        {"formation", "编队教学：调整前后排"},
        {"first", "探索教学：完成第一场战斗或宝箱"}
    };
    for (const auto& q : quests) questList->addItem(QString("%1 %2").arg(completedQuests.contains(q.first) ? "[完成]" : "[进行中]", q.second));
}

void MainWindow::appendLog(const QString& message) {
    if (dungeonLog) dungeonLog->append(message);
}

QString MainWindow::saveDirectory() const {
    QDir dir(QCoreApplication::applicationDirPath());
    if (!dir.exists("saves")) dir.mkdir("saves");
    return dir.filePath("saves");
}

QString MainWindow::savePath(int slot) const {
    return QDir(saveDirectory()).filePath(QString("slot%1.ini").arg(slot));
}

bool MainWindow::saveExists(int slot) const {
    return QFile::exists(savePath(slot));
}

QString MainWindow::saveSummary(int slot) const {
    if (!saveExists(slot)) return QString("存档 %1：空白").arg(slot);
    QSettings s(savePath(slot), QSettings::IniFormat);
    s.setIniCodec("UTF-8");
    return QString("存档 %1：轮回%2 / 学期%3 / 金币%4 / 队长：%5")
        .arg(slot).arg(s.value("state/loop", 1).toInt()).arg(s.value("state/term", 1).toInt())
        .arg(s.value("state/gold", 0).toInt()).arg(s.value("party/0/name", "未命名").toString());
}

bool MainWindow::saveGame(int slot) {
    if (!hasGame || slot < 1 || slot > 4) return false;
    saveScheduleFromTable();
    QSettings s(savePath(slot), QSettings::IniFormat);
    s.setIniCodec("UTF-8");
    s.clear();
    s.setValue("state/gold", gold);
    s.setValue("state/demonCoins", demonCoins);
    s.setValue("state/loop", loopCount);
    s.setValue("state/term", termCount);
    s.setValue("state/floor", currentFloor);
    s.setValue("state/room", currentRoom);
    s.setValue("state/inDungeon", inDungeon);
    s.setValue("state/formationMode", formationMode);
    s.setValue("state/knownEnemies", QStringList(knownEnemies.values()));
    s.setValue("state/completedQuests", QStringList(completedQuests.values()));
    s.setValue("inventory/count", inventory.size());
    for (int i = 0; i < inventory.size(); ++i) {
        const QString p = QString("inventory/%1/").arg(i);
        s.setValue(p + "name", inventory[i].name);
        s.setValue(p + "type", inventory[i].type);
        s.setValue(p + "desc", inventory[i].desc);
        s.setValue(p + "price", inventory[i].price);
        s.setValue(p + "battle", inventory[i].battleUsable);
    }
    s.setValue("party/count", party.size());
    for (int i = 0; i < party.size(); ++i) {
        const auto& c = party[i];
        const QString p = QString("party/%1/").arg(i);
        s.setValue(p + "name", c.name); s.setValue(p + "role", c.role); s.setValue(p + "desc", c.desc);
        s.setValue(p + "level", c.level); s.setValue(p + "exp", c.exp); s.setValue(p + "hp", c.hp); s.setValue(p + "maxHp", c.maxHp);
        s.setValue(p + "mp", c.mp); s.setValue(p + "maxMp", c.maxMp); s.setValue(p + "stamina", c.stamina); s.setValue(p + "maxStamina", c.maxStamina);
        s.setValue(p + "physAtk", c.physAtk); s.setValue(p + "magicAtk", c.magicAtk); s.setValue(p + "physDef", c.physDef); s.setValue(p + "magicRes", c.magicRes);
        s.setValue(p + "active", c.active); s.setValue(p + "position", c.position); s.setValue(p + "skills", c.skills);
        s.setValue(QString("schedule/%1").arg(i), schedules.value(i));
    }
    s.sync();
    activeSaveSlot = slot;
    return true;
}

bool MainWindow::loadGame(int slot) {
    if (!saveExists(slot)) return false;
    QSettings s(savePath(slot), QSettings::IniFormat);
    s.setIniCodec("UTF-8");
    clearGameState();
    gold = s.value("state/gold", 0).toInt();
    demonCoins = s.value("state/demonCoins", 0).toInt();
    loopCount = s.value("state/loop", 1).toInt();
    termCount = s.value("state/term", 1).toInt();
    currentFloor = s.value("state/floor", 0).toInt();
    currentRoom = s.value("state/room", 0).toInt();
    inDungeon = s.value("state/inDungeon", false).toBool();
    formationMode = s.value("state/formationMode", 0).toInt();
    for (const QString& e : s.value("state/knownEnemies").toStringList()) knownEnemies.insert(e);
    for (const QString& q : s.value("state/completedQuests").toStringList()) completedQuests.insert(q);
    int invCount = s.value("inventory/count", 0).toInt();
    for (int i = 0; i < invCount; ++i) {
        const QString p = QString("inventory/%1/").arg(i);
        inventory << ItemData{s.value(p + "name").toString(), s.value(p + "type").toString(), s.value(p + "desc").toString(), s.value(p + "price").toInt(), s.value(p + "battle").toBool()};
    }
    schedules.clear();
    int count = s.value("party/count", 0).toInt();
    for (int i = 0; i < count; ++i) {
        const QString p = QString("party/%1/").arg(i);
        CharacterData c;
        c.name = s.value(p + "name").toString(); c.role = s.value(p + "role").toString(); c.desc = s.value(p + "desc").toString();
        c.level = s.value(p + "level").toInt(); c.exp = s.value(p + "exp").toInt(); c.hp = s.value(p + "hp").toInt(); c.maxHp = s.value(p + "maxHp").toInt();
        c.mp = s.value(p + "mp").toInt(); c.maxMp = s.value(p + "maxMp").toInt(); c.stamina = s.value(p + "stamina").toInt(); c.maxStamina = s.value(p + "maxStamina").toInt();
        c.physAtk = s.value(p + "physAtk").toInt(); c.magicAtk = s.value(p + "magicAtk").toInt(); c.physDef = s.value(p + "physDef").toInt(); c.magicRes = s.value(p + "magicRes").toInt();
        c.active = s.value(p + "active").toBool(); c.position = s.value(p + "position").toString(); c.skills = s.value(p + "skills").toStringList();
        party << c;
        schedules << s.value(QString("schedule/%1").arg(i)).toStringList();
    }
    while (schedules.size() < party.size()) schedules << QStringList({"专业课", "专业课", "专业课", "专业课", "专业课", "专业课", "专业课", "专业课", "专业课", "专业课", "不上课", "不上课", "不上课", "不上课"});
    activeSaveSlot = slot;
    hasGame = !party.isEmpty();
    refreshAll();
    appendLog(QString("已读取存档 %1。").arg(slot));
    return hasGame;
}

void MainWindow::deleteSaveFile(int slot) {
    QFile::remove(savePath(slot));
    if (activeSaveSlot == slot) clearGameState();
}

void MainWindow::addItem(const ItemData& item, int count) {
    for (int i = 0; i < count; ++i) {
        if (inventory.size() >= 40) {
            QMessageBox::warning(this, "背包已满", "背包容量已满，后续掉落无法拾取。");
            return;
        }
        inventory << item;
        if (item.type.contains("装备")) completeQuest("equip");
    }
}

void MainWindow::grantExp(int amount) {
    for (auto& c : party) {
        c.exp += amount;
        applyLevelUps(c);
    }
}

void MainWindow::applyLevelUps(CharacterData& c) {
    while (c.exp >= c.level * 100 && c.level < 30) {
        c.exp -= c.level * 100;
        ++c.level;
        c.maxHp += 8; c.maxMp += 5; c.physAtk += 2; c.magicAtk += 2; c.physDef += 1; c.magicRes += 1;
        c.hp = c.maxHp; c.mp = c.maxMp;
    }
}

int MainWindow::partyPower() const {
    int power = 0;
    for (const auto& c : party) {
        if (!c.active) continue;
        power += c.hp / 4 + c.mp / 8 + c.physAtk * 3 + c.magicAtk * 3 + c.physDef * 2 + c.magicRes * 2 + c.level * 10;
    }
    return power;
}

int MainWindow::activeCount() const {
    int count = 0;
    for (const auto& c : party) if (c.active) ++count;
    return count;
}

QVector<int> MainWindow::activeIndexes() const {
    QVector<int> result;
    for (int i = 0; i < party.size(); ++i) if (party[i].active) result << i;
    return result;
}

MainWindow::ItemData MainWindow::equipmentForFloor(int floorValue) const {
    QStringList names = QStringList({"", "磨损球鞋", "禁书护符", "镜面礼帽", "防酸实验服", "命运塔罗", "校长室钥匙", "真理裂片"});
    return {names.value(floorValue, "地窟装备"), QString("第%1层装备").arg(floorValue), "提升全员属性，可在恶魔商店半价出售。", 80 + floorValue * 45, false};
}

MainWindow::ItemData MainWindow::findCatalogItem(const QString& name) const {
    for (const auto& item : angelCatalog) if (item.name == name) return item;
    for (const auto& item : demonCatalog) if (item.name == name) return item;
    for (int f = 1; f <= 7; ++f) { auto item = equipmentForFloor(f); if (item.name == name) return item; }
    return {name, "未知", "未登记物品", 0, false};
}

QString MainWindow::currentRoomType() const {
    if (!inDungeon) return "未进入";
    if (currentFloor == 7) return currentRoom <= 1 ? "恶魔商店" : "最终Boss";
    if (currentRoom <= 1) return "恶魔商店";
    if (currentRoom == chestRoom) return "宝箱房";
    if (currentRoom == eliteRoom) return "精英战斗 + 下一层通道";
    return "普通战斗";
}

QString MainWindow::enemyNameForFloor(int floorValue, bool elite) const {
    QStringList normal = QStringList({"", "懒惰的短跑者", "涂鸦书灵", "镜面侍从", "酸液史莱姆", "塔罗残影", "纪律傀儡", "真理残响"});
    QStringList boss = QStringList({"", "铅球手", "禁书管理员", "悲情女主角", "剧毒试管精", "命运观测者", "门前审判官", "真理篡改者·完全体"});
    return elite ? boss.value(floorValue) : normal.value(floorValue);
}

void MainWindow::enterNewFloor(int nextFloor) {
    currentFloor = clampInt(nextFloor, 1, 7);
    currentRoom = 1;
    if (currentFloor < 7) {
        chestRoom = 2 + int(QRandomGenerator::global()->bounded(5));
        do { eliteRoom = 2 + int(QRandomGenerator::global()->bounded(5)); } while (eliteRoom == chestRoom);
    } else {
        chestRoom = 0;
        eliteRoom = 2;
    }
    appendLog(QString("进入第%1层：%2。第一间房是恶魔商店。").arg(currentFloor).arg(kFloorNames.value(currentFloor)));
}

void MainWindow::resolveBattle(bool elite, bool boss) {
    QVector<int> active = activeIndexes();
    if (active.isEmpty()) {
        QMessageBox::warning(this, "无法战斗", "至少需要 1 名角色上阵。");
        return;
    }
    for (int idx : active) party[idx].stamina = std::max(0, party[idx].stamina - 8);
    QString enemy = enemyNameForFloor(currentFloor, elite || boss);
    knownEnemies.insert(enemy);
    int enemyPower = currentFloor * 155 + (elite ? 120 : 0) + (boss ? 460 : 0);
    int playerPower = partyPower() + int(QRandomGenerator::global()->bounded(120));
    appendLog(QString("遭遇【%1】，我方战力 %2，对方威胁 %3。").arg(enemy).arg(playerPower).arg(enemyPower));
    if (playerPower >= enemyPower) {
        int exp = 45 * currentFloor + (elite ? 80 : 0) + (boss ? 260 : 0);
        int coins = 35 * currentFloor + (elite ? 55 : 0);
        grantExp(exp);
        demonCoins += coins;
        addItem(equipmentForFloor(elite ? std::min(7, currentFloor + 1) : currentFloor), 3);
        completeQuest("first");
        appendLog(QString("战斗胜利：获得 %1 经验、%2 恶魔币和 3 件装备。").arg(exp).arg(coins));
        if (boss) {
            QMessageBox::information(this, "结局", "真理篡改者被击败，暑假的轮回终于被打破。");
            inDungeon = false; currentFloor = 0; currentRoom = 0;
        }
    } else {
        appendLog("战斗失败，全队被迫退出地窟。");
        handlePartyDefeat();
    }
    refreshAll();
}

void MainWindow::handlePartyDefeat() {
    restoreBeforeDungeon();
    ++loopCount;
    inDungeon = false;
    currentFloor = 0;
    currentRoom = 0;
    demonCoins = 0;
    appendLog(QString("轮回重启：恢复进入地窟前状态，进入第 %1 次轮回，恶魔币清空。").arg(loopCount));
}

void MainWindow::snapshotBeforeDungeon() {
    preDungeonParty = party;
    preDungeonInventory = inventory;
    preDungeonSnapshotValid = true;
}

void MainWindow::restoreBeforeDungeon() {
    if (!preDungeonSnapshotValid) return;
    party = preDungeonParty;
    inventory = preDungeonInventory;
}

void MainWindow::completeQuest(const QString& key) {
    completedQuests.insert(key);
}

void MainWindow::onNewSave() {
    int slot = saveList && saveList->currentRow() >= 0 ? saveList->currentRow() + 1 : 1;
    if (saveExists(slot)) {
        QMessageBox::information(this, "存档已存在", "请选择空白槽，或先删除已有存档。");
        return;
    }
    QStringList names;
    for (const QString& role : kRoles) {
        bool ok = false;
        QString name = QInputDialog::getText(this, "角色命名", QString("请输入【%1】的姓名：").arg(role), QLineEdit::Normal, role, &ok);
        if (!ok) return;
        names << (name.trimmed().isEmpty() ? role : name.trimmed());
    }
    clearGameState();
    createInitialParty(names);
    hasGame = true;
    gold = 260;
    for (auto& c : party) { c.maxHp += 20; c.hp = c.maxHp; }
    activeSaveSlot = slot;
    completeQuest("name");
    saveGame(slot);
    appendLog(QString("新建存档 %1，第一学期开始，全员获得开局生命提升。").arg(slot));
    tabs->setCurrentIndex(1);
    refreshAll();
}

void MainWindow::onLoadSave() {
    int row = saveList ? saveList->currentRow() : -1;
    if (row < 0) return;
    if (!loadGame(row + 1)) QMessageBox::information(this, "空白存档", "这个存档槽还是空的，可以点击“新建存档”。");
}

void MainWindow::onSaveCurrent() {
    int slot = activeSaveSlot > 0 ? activeSaveSlot : (saveList && saveList->currentRow() >= 0 ? saveList->currentRow() + 1 : 1);
    if (saveGame(slot)) QMessageBox::information(this, "保存成功", QString("已保存到存档 %1。").arg(slot));
    else QMessageBox::warning(this, "无法保存", "请先新建或读取一个存档。");
    refreshAll();
}

void MainWindow::onDeleteSave() {
    int row = saveList ? saveList->currentRow() : -1;
    if (row < 0) return;
    int slot = row + 1;
    if (QMessageBox::question(this, "删除存档", QString("确定删除存档 %1？").arg(slot)) == QMessageBox::Yes) {
        deleteSaveFile(slot);
        refreshAll();
    }
}

void MainWindow::onApplySchedule() {
    if (!hasGame) return;
    saveScheduleFromTable();
    int totalGold = 0;
    for (int i = 0; i < party.size(); ++i) {
        auto& c = party[i];
        for (const QString& action : schedules.value(i)) {
            bool half = action.startsWith("补习班");
            QString base = half ? action.mid(QString("补习班+").size()) : action;
            int gain = half ? 1 : 2;
            if (base == "打工") { int earn = completedQuests.contains("angel_work") ? 45 : 35; gold += earn * 20; totalGold += earn * 20; c.stamina = std::max(0, c.stamina - 14); }
            else if (base == "不上课") c.stamina = std::min(c.maxStamina, c.stamina + 10);
            else {
                c.stamina = std::max(0, c.stamina - (base == "体育课" ? 12 : 8));
                if (base == "大物课") { c.physAtk += gain; c.physDef += gain; }
                else if (base == "外语课") demonCoins += gain;
                else if (base == "化学课") c.magicRes += gain;
                else if (base == "高数课") c.physDef += gain;
                else if (base == "专业课") { c.physAtk += gain; c.magicAtk += gain; }
                else if (base == "体育课") { c.maxHp += 2 * gain; c.maxStamina += gain; }
            }
        }
        c.hp = std::min(c.hp + 20, c.maxHp);
        c.mp = std::min(c.mp + 15, c.maxMp);
    }
    ++termCount;
    appendLog(QString("第 %1 学期完成：课程成长已结算，打工收益 %2 金币。").arg(termCount - 1).arg(totalGold));
    refreshAll();
}

void MainWindow::onWinterRest() {
    if (!hasGame) return;
    for (auto& c : party) { c.maxHp += 15; c.hp = c.maxHp; c.stamina = c.maxStamina; }
    appendLog("寒假选择休息：全员生命上限提升并恢复活力。");
    refreshAll();
}

void MainWindow::onWinterWork() {
    if (!hasGame) return;
    int earn = completedQuests.contains("angel_work") ? 420 : 320;
    gold += earn;
    for (auto& c : party) c.stamina = std::max(0, c.stamina - 20);
    appendLog(QString("寒假选择打工：获得 %1 金币，活力下降。").arg(earn));
    refreshAll();
}

void MainWindow::onBuyAngelItem() {
    if (!hasGame || !angelShopList || angelShopList->currentRow() < 0) return;
    ItemData item = angelCatalog.value(angelShopList->currentRow());
    if (gold < item.price) { QMessageBox::warning(this, "金币不足", "金币不足，无法购买。"); return; }
    gold -= item.price;
    if (item.name == "活力少年") for (auto& c : party) { c.maxStamina += 15; c.stamina = c.maxStamina; }
    else if (item.name == "家财万贯") gold += 120;
    else if (item.name == "高效打工人") completedQuests.insert("angel_work");
    else addItem(item);
    completeQuest("angel");
    appendLog(QString("天使商店购买：%1。").arg(item.name));
    refreshAll();
}

void MainWindow::onEnterDungeon() {
    if (!hasGame) return;
    if (activeCount() == 0) { QMessageBox::warning(this, "无法进入", "请至少安排 1 名角色上阵。"); return; }
    snapshotBeforeDungeon();
    inDungeon = true;
    demonCoins += completedQuests.contains("demon_friend") ? 90 : 35;
    enterNewFloor(1);
    refreshAll();
}

void MainWindow::onNextRoom() {
    if (!inDungeon) { onEnterDungeon(); return; }
    if (currentFloor == 7 && currentRoom >= 2) { resolveBattle(true, true); return; }
    if (currentRoom == 1) currentRoom = 2;
    else if (currentFloor < 7 && currentRoom == eliteRoom) { enterNewFloor(currentFloor + 1); refreshAll(); return; }
    else ++currentRoom;
    QString type = currentRoomType();
    if (type == "普通战斗") resolveBattle(false, false);
    else if (type.startsWith("精英")) resolveBattle(true, false);
    else if (type == "最终Boss") resolveBattle(true, true);
    else appendLog(QString("进入房间：%1。").arg(type));
    refreshAll();
}

void MainWindow::onOpenTreasure() {
    if (!inDungeon || currentRoomType() != "宝箱房") {
        QMessageBox::information(this, "没有宝箱", "只有宝箱房可以开启宝箱。");
        return;
    }
    grantExp(currentFloor * 35);
    demonCoins += 25 * currentFloor;
    addItem(equipmentForFloor(currentFloor));
    addItem(angelCatalog[int(QRandomGenerator::global()->bounded(angelCatalog.size() - 3))]);
    completeQuest("first");
    appendLog("宝箱开启：获得经验、恶魔币、装备和补给。");
    refreshAll();
}

void MainWindow::onDemonShop() {
    if (!inDungeon || currentRoomType() != "恶魔商店") {
        QMessageBox::information(this, "恶魔商店", "只有每层第一间房可以和恶魔商人交易。");
        return;
    }
    appendLog("恶魔商人展开货架：消耗品、各层装备和养成项均可购买。");
}

void MainWindow::onBuyDemonItem() {
    if (!hasGame || !demonShopList || demonShopList->currentRow() < 0) return;
    if (!inDungeon || currentRoomType() != "恶魔商店") { QMessageBox::information(this, "无法交易", "恶魔交易只能在地窟的恶魔商店房间进行。"); return; }
    int row = demonShopList->currentRow();
    ItemData item = row < demonCatalog.size() ? demonCatalog[row] : equipmentForFloor(row - demonCatalog.size() + 1);
    if (demonCoins < item.price) { QMessageBox::warning(this, "恶魔币不足", "恶魔币不足，无法购买。"); return; }
    demonCoins -= item.price;
    if (item.name == "恶魔之友") completedQuests.insert("demon_friend");
    else addItem(item);
    bool firstTrade = !completedQuests.contains("demon");
    completeQuest("demon");
    if (firstTrade) addItem(equipmentForFloor(1));
    appendLog(QString("恶魔商店购买：%1。").arg(item.name));
    refreshAll();
}

void MainWindow::onSetFormation() {
    if (!hasGame) return;
    formationMode = formationBox ? formationBox->currentIndex() : 0;
    QVector<int> active = activeIndexes();
    int frontNeeded = formationMode == 0 ? 1 : 2;
    for (int i = 0; i < active.size(); ++i) party[active[i]].position = i < frontNeeded ? "前排" : "后排";
    completeQuest("formation");
    appendLog(QString("编队已调整为：%1。").arg(formationBox->currentText()));
    refreshAll();
}

void MainWindow::onToggleActive() {
    if (!hasGame || !formationList || formationList->currentRow() < 0) return;
    int idx = formationList->currentRow();
    if (!party[idx].active && activeCount() >= 3) { QMessageBox::warning(this, "上阵人数已满", "最多 3 名角色上阵。"); return; }
    if (party[idx].active && activeCount() <= 1) { QMessageBox::warning(this, "至少一人上阵", "至少保留 1 名角色上阵。"); return; }
    party[idx].active = !party[idx].active;
    onSetFormation();
}

void MainWindow::onUseInventoryItem() {
    if (!hasGame || !inventoryList || inventoryList->currentRow() < 0) return;
    int row = inventoryList->currentRow();
    if (row < 0 || row >= inventory.size()) return;
    ItemData item = inventory[row];
    if (inDungeon && item.type == "食品") { QMessageBox::information(this, "不能使用", "食品不能在战斗/地窟流程中使用。"); return; }
    for (auto& c : party) {
        if (item.name.contains("回蓝") || item.name.contains("咖啡") || item.name.contains("汽水") || item.name.contains("浓茶") || item.name == "阿司匹林") c.mp = std::min(c.maxMp, c.mp + c.maxMp / 2);
        else if (item.type.contains("装备")) { c.physAtk += 2; c.magicAtk += 2; c.physDef += 1; c.magicRes += 1; }
        else { c.hp = std::min(c.maxHp, c.hp + c.maxHp / 2); c.stamina = std::min(c.maxStamina, c.stamina + 25); }
    }
    inventory.removeAt(row);
    appendLog(QString("使用物品：%1。").arg(item.name));
    refreshAll();
}

void MainWindow::onDropInventoryItem() {
    if (!inventoryList || inventoryList->currentRow() < 0) return;
    int row = inventoryList->currentRow();
    if (row >= 0 && row < inventory.size()) {
        appendLog(QString("丢弃物品：%1。").arg(inventory[row].name));
        inventory.removeAt(row);
        refreshAll();
    }
}
