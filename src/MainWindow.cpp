#include "MainWindow.h"
#include <QApplication>
#include <QFont>
#include <QFrame>
#include <QSplitter>
#include <QScrollArea>
#include <QHeaderView>
#include <QDateTime>
#include <QDialog>
#include <QFormLayout>
#include <algorithm>
#include <sstream>
#include <iomanip>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), currentEnemyIndex(0), inBattle(false),
      rageTurnsLeft(0), rageAttackBonus(0) {
    setWindowTitle("校园RPG冒险游戏系统 - Campus RPG Adventure");
    setMinimumSize(1024, 720);
    resize(1100, 780);

    game.initPlayer("勇者");

    setupUI();
    applyStyleSheet();
    refreshAll();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    tabs = new QTabWidget(this);
    setCentralWidget(tabs);

    createCharacterTab();
    createSaveTab();
    createInventoryTab();
    createShopTab();
    createTownTab();
    createTaskTab();
    createMapTab();
    createBattleTab();

    tabs->setCurrentIndex(1);  // Start on save/menu tab
}

// ================================================================
// SAVE TAB
// ================================================================
void MainWindow::createSaveTab() {
    auto* w = new QWidget();
    auto* layout = new QVBoxLayout(w);
    layout->setSpacing(12);
    layout->setContentsMargins(30, 20, 30, 20);

    auto* title = new QLabel("💾 存档菜单");
    title->setAlignment(Qt::AlignCenter);
    title->setFont(QFont("Microsoft YaHei", 20, QFont::Bold));
    layout->addWidget(title);

    auto* note = new QLabel("最多4个存档。每个存档拥有独立角色、背包、仓库、银行金币和当前位置。");
    note->setAlignment(Qt::AlignCenter);
    note->setWordWrap(true);
    layout->addWidget(note);

    saveList = new QListWidget();
    saveList->setFont(QFont("Microsoft YaHei", 12));
    saveList->setAlternatingRowColors(true);
    layout->addWidget(saveList);

    auto* row = new QHBoxLayout();
    newSaveBtn = new QPushButton("新建存档");
    loadSaveBtn = new QPushButton("读取存档");
    saveCurrentBtn = new QPushButton("保存当前");
    deleteSaveBtn = new QPushButton("删除存档");
    connect(newSaveBtn, &QPushButton::clicked, this, &MainWindow::onNewSave);
    connect(loadSaveBtn, &QPushButton::clicked, this, &MainWindow::onLoadSave);
    connect(saveCurrentBtn, &QPushButton::clicked, this, &MainWindow::onSaveCurrent);
    connect(deleteSaveBtn, &QPushButton::clicked, this, &MainWindow::onDeleteSave);
    row->addWidget(newSaveBtn);
    row->addWidget(loadSaveBtn);
    row->addWidget(saveCurrentBtn);
    row->addWidget(deleteSaveBtn);
    layout->addLayout(row);

    tabs->addTab(w, "💾 存档");
}

void MainWindow::refreshSaveSlots() {
    saveList->clear();
    for (int slot = 1; slot <= GameManager::maxSaveSlots(); ++slot) {
        QString marker = (game.currentSaveSlot == slot) ? "  ← 当前" : "";
        if (game.saveExists(slot)) {
            saveList->addItem(QString("槽位%1：%2%3")
                .arg(slot)
                .arg(QString::fromStdString(game.saveSlotName(slot)))
                .arg(marker));
        } else {
            saveList->addItem(QString("槽位%1：空存档%2").arg(slot).arg(marker));
        }
    }
}

void MainWindow::onNewSave() {
    int row = saveList->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "提示", "请先选择一个存档槽。");
        return;
    }
    int slot = row + 1;
    if (game.saveExists(slot)) {
        auto reply = QMessageBox::question(this, "覆盖确认",
            "该槽位已有存档，是否覆盖并新建？",
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) return;
    }

    bool ok = false;
    QString name = QInputDialog::getText(this, "新建存档",
        "请输入角色名称:", QLineEdit::Normal, "勇者", &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    game.resetWorldForNewPlayer(name.toStdString());
    QMessageBox::information(this, "新建存档", QString::fromStdString(game.saveGame(slot)));
    rageTurnsLeft = 0;
    rageAttackBonus = 0;
    refreshAll();
}

void MainWindow::onLoadSave() {
    int row = saveList->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "提示", "请先选择要读取的存档。");
        return;
    }
    QMessageBox::information(this, "读取存档", QString::fromStdString(game.loadGame(row + 1)));
    rageTurnsLeft = 0;
    rageAttackBonus = 0;
    refreshAll();
}

void MainWindow::onSaveCurrent() {
    int row = saveList->currentRow();
    int slot = row >= 0 ? row + 1 : game.currentSaveSlot;
    if (slot < 1) {
        QMessageBox::information(this, "提示", "请选择一个存档槽。");
        return;
    }
    QMessageBox::information(this, "保存存档", QString::fromStdString(game.saveGame(slot)));
    refreshSaveSlots();
}

void MainWindow::onDeleteSave() {
    int row = saveList->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "提示", "请先选择要删除的存档。");
        return;
    }
    auto reply = QMessageBox::question(this, "删除确认",
        "确定删除这个存档吗？此操作不可恢复。",
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;
    QMessageBox::information(this, "删除存档", QString::fromStdString(game.deleteSave(row + 1)));
    refreshSaveSlots();
}

// ================================================================
// CHARACTER TAB
// ================================================================
void MainWindow::createCharacterTab() {
    auto* w = new QWidget();
    auto* layout = new QVBoxLayout(w);
    layout->setSpacing(12);
    layout->setContentsMargins(30, 20, 30, 20);

    // Title
    auto* title = new QLabel("👤 角色信息");
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont("Microsoft YaHei", 20, QFont::Bold);
    title->setFont(titleFont);
    layout->addWidget(title);

    // Character card
    auto* card = new QGroupBox();
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(10);
    cardLayout->setContentsMargins(25, 20, 25, 20);

    charNameLabel = new QLabel();
    charNameLabel->setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    cardLayout->addWidget(charNameLabel);

    charLevelLabel = new QLabel();
    charLevelLabel->setFont(QFont("Microsoft YaHei", 13));
    cardLayout->addWidget(charLevelLabel);

    // HP bar
    auto* hpRow = new QHBoxLayout();
    hpRow->addWidget(new QLabel("生命值:"));
    hpBar = new QProgressBar();
    hpBar->setMinimum(0);
    hpBar->setMaximum(100);
    hpBar->setTextVisible(false);
    hpBar->setFixedHeight(24);
    hpBar->setStyleSheet("QProgressBar { border: 1px solid #ccc; border-radius: 10px; background: #eee; }"
                         "QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
                         "stop:0 #f44336, stop:0.5 #e91e63, stop:1 #4caf50); border-radius: 10px; }");
    hpRow->addWidget(hpBar, 1);
    hpLabel = new QLabel();
    hpLabel->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    hpRow->addWidget(hpLabel);
    cardLayout->addLayout(hpRow);

    // EXP bar
    auto* expRow = new QHBoxLayout();
    expRow->addWidget(new QLabel("经验值:"));
    expBar = new QProgressBar();
    expBar->setMinimum(0);
    expBar->setMaximum(100);
    expBar->setTextVisible(false);
    expBar->setFixedHeight(20);
    expBar->setStyleSheet("QProgressBar { border: 1px solid #ccc; border-radius: 10px; background: #eee; }"
                          "QProgressBar::chunk { background: #2196F3; border-radius: 10px; }");
    expRow->addWidget(expBar, 1);
    expLabel = new QLabel();
    expLabel->setFont(QFont("Microsoft YaHei", 10));
    expRow->addWidget(expLabel);
    cardLayout->addLayout(expRow);

    // Stats grid
    auto* statsGrid = new QHBoxLayout();
    goldLabel = new QLabel();
    goldLabel->setFont(QFont("Microsoft YaHei", 12));
    atkLabel = new QLabel();
    atkLabel->setFont(QFont("Microsoft YaHei", 12));
    defLabel = new QLabel();
    defLabel->setFont(QFont("Microsoft YaHei", 12));
    statsGrid->addWidget(goldLabel);
    statsGrid->addWidget(atkLabel);
    statsGrid->addWidget(defLabel);
    cardLayout->addLayout(statsGrid);

    layout->addWidget(card);

    // Refresh button
    auto* refreshBtn = new QPushButton("🔄 刷新信息");
    refreshBtn->setFixedWidth(160);
    refreshBtn->setFixedHeight(38);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshCharacterPanel);
    layout->addWidget(refreshBtn, 0, Qt::AlignCenter);

    layout->addStretch();
    tabs->addTab(w, "🏠 角色");
}

void MainWindow::refreshCharacterPanel() {
    auto& c = game.player;
    if (!c) return;

    charNameLabel->setText(QString::fromStdString(c->getName()));
    charLevelLabel->setText(QString("等级 Lv.%1/%2  |  背包: %3/%4  |  MP: %5/%6  |  %7\n%8\n%9")
        .arg(c->getLevel()).arg(Character::MaxLevel)
        .arg(c->inventoryCount()).arg(Character::MaxInventorySlots)
        .arg(c->getMp()).arg(c->getMaxMp())
        .arg(QString::fromStdString(game.currentLocationType()))
        .arg(QString::fromStdString(game.mainlineInfo()))
        .arg(QString::fromStdString(game.formationInfo())));

    hpBar->setMaximum(c->getMaxHp());
    hpBar->setValue(c->getHp());
    hpLabel->setText(QString("%1 / %2").arg(c->getHp()).arg(c->getMaxHp()));

    expBar->setMaximum(c->getExpToNextLevel());
    expBar->setValue(c->getExp());
    expLabel->setText(QString("%1 / %2").arg(c->getExp()).arg(c->getExpToNextLevel()));

    goldLabel->setText(QString("💰 身上: %1 G | 银行: %2 G").arg(c->getGold()).arg(game.bankGold));
    atkLabel->setText(QString("⚔️ 攻击: %1").arg(c->getAttack()));
    defLabel->setText(QString("🛡️ 防御: %1").arg(c->getDefense()));
}

// ================================================================
// INVENTORY TAB
// ================================================================
void MainWindow::createInventoryTab() {
    auto* w = new QWidget();
    auto* layout = new QVBoxLayout(w);
    layout->setSpacing(10);
    layout->setContentsMargins(30, 20, 30, 20);

    auto* title = new QLabel("🎒 背包管理");
    QFont titleFont("Microsoft YaHei", 20, QFont::Bold);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    invList = new QListWidget();
    invList->setFont(QFont("Microsoft YaHei", 11));
    invList->setAlternatingRowColors(true);
    invList->setMinimumHeight(300);
    layout->addWidget(invList);

    auto* btnRow = new QHBoxLayout();
    useItemBtn = new QPushButton("✅ 使用物品");
    useItemBtn->setFixedHeight(36);
    useItemBtn->setStyleSheet("QPushButton { background: #4CAF50; color: white; border-radius: 6px; }"
                              "QPushButton:hover { background: #388E3C; }");
    connect(useItemBtn, &QPushButton::clicked, this, &MainWindow::onUseItem);

    dropItemBtn = new QPushButton("🗑️ 丢弃物品");
    dropItemBtn->setFixedHeight(36);
    dropItemBtn->setStyleSheet("QPushButton { background: #f44336; color: white; border-radius: 6px; }"
                               "QPushButton:hover { background: #D32F2F; }");
    connect(dropItemBtn, &QPushButton::clicked, this, &MainWindow::onDropItem);

    btnRow->addWidget(useItemBtn);
    btnRow->addWidget(dropItemBtn);
    layout->addLayout(btnRow);

    tabs->addTab(w, "🎒 背包");
}

void MainWindow::refreshInventory() {
    invList->clear();
    if (!game.player) return;
    auto& inv = game.player->getInventory();
    invList->addItem(QString("背包容量：%1 / %2")
        .arg(game.player->inventoryCount()).arg(Character::MaxInventorySlots));
    for (size_t i = 0; i < inv.size(); ++i) {
        invList->addItem(QString("[%1] %2").arg(i+1).arg(
            QString::fromStdString(inv[i]->getInfo())));
    }
    if (inv.empty()) {
        invList->addItem("背包为空 —— 去商店购买物品吧!");
    } else if (game.player->isInventoryFull()) {
        invList->addItem("背包已满，战斗掉落和商店购买将无法继续放入。");
    }
}

void MainWindow::onUseItem() {
    int idx = invList->currentRow() - 1;
    if (idx < 0) {
        QMessageBox::information(this, "提示", "请先选择一件物品!");
        return;
    }
    auto& inv = game.player->getInventory();
    if (idx >= 0 && idx < (int)inv.size()) {
        std::string name = inv[idx]->getName();
        if (name == "武器强化卷" || name == "铠甲强化卷") {
            QMessageBox::information(this, "提示", "强化卷需要在城镇节点的铁匠铺使用。");
            return;
        }
        if (name == "狂暴药水" || name == "虚弱药水" || name == "疲惫药水") {
            QMessageBox::information(this, "提示", "该药水需要在战斗中通过战斗背包使用。");
            return;
        }
    }
    std::string result = game.player->useItem(idx);
    QMessageBox::information(this, "使用物品", QString::fromStdString(result));
    refreshInventory();
    refreshCharacterPanel();
}

void MainWindow::onDropItem() {
    int idx = invList->currentRow() - 1;
    if (idx < 0) {
        QMessageBox::information(this, "提示", "请先选择一件物品!");
        return;
    }
    std::string result = game.player->removeItem(idx);
    QMessageBox::information(this, "丢弃物品", QString::fromStdString(result));
    refreshInventory();
}

// ================================================================
// SHOP TAB
// ================================================================
void MainWindow::createShopTab() {
    auto* w = new QWidget();
    auto* layout = new QHBoxLayout(w);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 15, 20, 15);

    // Left: Shop items
    auto* leftPanel = new QVBoxLayout();
    auto* shopTitle = new QLabel("🏪 校园商店");
    shopTitle->setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    shopTitle->setAlignment(Qt::AlignCenter);
    leftPanel->addWidget(shopTitle);

    shopGoldLabel = new QLabel();
    shopGoldLabel->setFont(QFont("Microsoft YaHei", 11));
    leftPanel->addWidget(shopGoldLabel);

    shopList = new QListWidget();
    shopList->setFont(QFont("Microsoft YaHei", 11));
    shopList->setAlternatingRowColors(true);
    leftPanel->addWidget(shopList);

    buyBtn = new QPushButton("🛒 购买选中物品");
    buyBtn->setFixedHeight(38);
    buyBtn->setStyleSheet("QPushButton { background: #FF9800; color: white; border-radius: 6px; }"
                          "QPushButton:hover { background: #F57C00; }");
    connect(buyBtn, &QPushButton::clicked, this, &MainWindow::onBuyItem);
    leftPanel->addWidget(buyBtn);

    layout->addLayout(leftPanel);

    // Right: Player items to sell
    auto* rightPanel = new QVBoxLayout();
    auto* sellTitle = new QLabel("💼 我的物品(出售)");
    sellTitle->setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    sellTitle->setAlignment(Qt::AlignCenter);
    rightPanel->addWidget(sellTitle);

    auto* sellNote = new QLabel("(售价为原价的一半)");
    sellNote->setAlignment(Qt::AlignCenter);
    rightPanel->addWidget(sellNote);

    playerSellList = new QListWidget();
    playerSellList->setFont(QFont("Microsoft YaHei", 11));
    playerSellList->setAlternatingRowColors(true);
    rightPanel->addWidget(playerSellList);

    sellBtn = new QPushButton("💰 出售选中物品");
    sellBtn->setFixedHeight(38);
    sellBtn->setStyleSheet("QPushButton { background: #9C27B0; color: white; border-radius: 6px; }"
                           "QPushButton:hover { background: #7B1FA2; }");
    connect(sellBtn, &QPushButton::clicked, this, &MainWindow::onSellItem);
    rightPanel->addWidget(sellBtn);

    layout->addLayout(rightPanel);
    tabs->addTab(w, "🏪 商店");
}

void MainWindow::refreshShop() {
    shopList->clear();
    bool town = game.isTown();
    for (size_t i = 0; i < game.shop->getItems().size(); ++i) {
        shopList->addItem(QString("[%1] %2").arg(i+1).arg(
            QString::fromStdString(game.shop->getItems()[i]->getInfo())));
    }
    if (game.shop->getItems().empty()) {
        shopList->addItem("商品已售罄!");
    }

    playerSellList->clear();
    auto& inv = game.player->getInventory();
    for (size_t i = 0; i < inv.size(); ++i) {
        playerSellList->addItem(QString("[%1] %2 (售价:%3G)").arg(i+1).arg(
            QString::fromStdString(inv[i]->getInfo())).arg(inv[i]->getPrice()/2));
    }

    shopGoldLabel->setText(QString("💰 我的金币: %1 G  |  %2")
        .arg(game.player->getGold())
        .arg(town ? "当前可交易" : "仅城镇节点可交易"));
    buyBtn->setEnabled(town);
    sellBtn->setEnabled(town);
}

void MainWindow::onBuyItem() {
    if (!game.isTown()) {
        QMessageBox::information(this, "提示", "商店仅城镇节点可开启。请前往食堂或宿舍区。");
        return;
    }
    int idx = shopList->currentRow();
    if (idx < 0) {
        QMessageBox::information(this, "提示", "请先选择商品!");
        return;
    }
    std::string result = game.shop->buyItem(idx, *game.player);
    QMessageBox::information(this, "购买结果", QString::fromStdString(result));
    refreshShop();
    refreshCharacterPanel();
}

void MainWindow::onSellItem() {
    if (!game.isTown()) {
        QMessageBox::information(this, "提示", "商店仅城镇节点可开启。请前往食堂或宿舍区。");
        return;
    }
    int idx = playerSellList->currentRow();
    if (idx < 0) {
        QMessageBox::information(this, "提示", "请先选择要出售的物品!");
        return;
    }
    std::string result = game.shop->sellItem(idx, *game.player);
    QMessageBox::information(this, "出售结果", QString::fromStdString(result));
    refreshShop();
    refreshCharacterPanel();
}

// ================================================================
// TOWN SERVICES TAB
// ================================================================
void MainWindow::createTownTab() {
    auto* w = new QWidget();
    auto* layout = new QVBoxLayout(w);
    layout->setSpacing(12);
    layout->setContentsMargins(22, 16, 22, 16);

    townStatusLabel = new QLabel();
    townStatusLabel->setAlignment(Qt::AlignCenter);
    townStatusLabel->setFont(QFont("Microsoft YaHei", 13, QFont::Bold));
    layout->addWidget(townStatusLabel);

    auto* top = new QHBoxLayout();

    auto* warehouseGroup = new QGroupBox("仓库系统");
    auto* warehouseLayout = new QHBoxLayout(warehouseGroup);
    auto* packBox = new QVBoxLayout();
    packBox->addWidget(new QLabel("背包"));
    townBackpackList = new QListWidget();
    townBackpackList->setAlternatingRowColors(true);
    packBox->addWidget(townBackpackList);
    depositItemBtn = new QPushButton("存入仓库");
    connect(depositItemBtn, &QPushButton::clicked, this, &MainWindow::onDepositItem);
    packBox->addWidget(depositItemBtn);

    auto* storeBox = new QVBoxLayout();
    storeBox->addWidget(new QLabel("仓库"));
    warehouseList = new QListWidget();
    warehouseList->setAlternatingRowColors(true);
    storeBox->addWidget(warehouseList);
    withdrawItemBtn = new QPushButton("取回背包");
    connect(withdrawItemBtn, &QPushButton::clicked, this, &MainWindow::onWithdrawItem);
    storeBox->addWidget(withdrawItemBtn);
    warehouseLayout->addLayout(packBox);
    warehouseLayout->addLayout(storeBox);
    top->addWidget(warehouseGroup, 2);

    auto* bankGroup = new QGroupBox("银行系统");
    auto* bankLayout = new QVBoxLayout(bankGroup);
    bankGoldLabel = new QLabel();
    bankGoldLabel->setAlignment(Qt::AlignCenter);
    bankGoldLabel->setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    bankLayout->addWidget(bankGoldLabel);
    depositGoldBtn = new QPushButton("存入金币");
    withdrawGoldBtn = new QPushButton("取出金币");
    connect(depositGoldBtn, &QPushButton::clicked, this, &MainWindow::onDepositGold);
    connect(withdrawGoldBtn, &QPushButton::clicked, this, &MainWindow::onWithdrawGold);
    bankLayout->addWidget(depositGoldBtn);
    bankLayout->addWidget(withdrawGoldBtn);
    bankLayout->addStretch();
    top->addWidget(bankGroup, 1);

    layout->addLayout(top, 2);

    auto* forgeGroup = new QGroupBox("铁匠铺");
    auto* forgeLayout = new QVBoxLayout(forgeGroup);
    forgeList = new QListWidget();
    forgeList->setAlternatingRowColors(true);
    forgeLayout->addWidget(forgeList);
    reinforceBtn = new QPushButton("使用强化卷强化自身");
    connect(reinforceBtn, &QPushButton::clicked, this, &MainWindow::onReinforceItem);
    forgeLayout->addWidget(reinforceBtn);
    layout->addWidget(forgeGroup, 1);

    tabs->addTab(w, "🏦 城镇服务");
}

void MainWindow::refreshTownServices() {
    bool town = game.isTown();
    townStatusLabel->setText(town
        ? "当前位于城镇节点，可以使用仓库、银行和铁匠铺。"
        : "当前不在城镇节点。请前往食堂或宿舍区使用城镇服务。");

    townBackpackList->clear();
    warehouseList->clear();
    forgeList->clear();
    if (!game.player) return;

    auto& inv = game.player->getInventory();
    townBackpackList->addItem(QString("背包容量：%1 / %2")
        .arg(game.player->inventoryCount()).arg(Character::MaxInventorySlots));
    for (size_t i = 0; i < inv.size(); ++i) {
        QString info = QString("[%1] %2").arg(i + 1).arg(QString::fromStdString(inv[i]->getInfo()));
        townBackpackList->addItem(info);
        if (inv[i]->getName() == "武器强化卷" || inv[i]->getName() == "铠甲强化卷") {
            forgeList->addItem(info);
        }
    }
    for (size_t i = 0; i < game.warehouse.size(); ++i) {
        warehouseList->addItem(QString("[%1] %2").arg(i + 1).arg(
            QString::fromStdString(game.warehouse[i]->getInfo())));
    }
    if (inv.empty()) townBackpackList->addItem("背包为空。");
    if (game.warehouse.empty()) warehouseList->addItem("仓库为空。");
    if (forgeList->count() == 0) forgeList->addItem("没有可用强化卷。");

    bankGoldLabel->setText(QString("身上金币：%1 G\n银行金币：%2 G")
        .arg(game.player->getGold()).arg(game.bankGold));
    depositItemBtn->setEnabled(town);
    withdrawItemBtn->setEnabled(town);
    depositGoldBtn->setEnabled(town);
    withdrawGoldBtn->setEnabled(town);
    reinforceBtn->setEnabled(town);
}

void MainWindow::onDepositItem() {
    QMessageBox::information(this, "仓库", QString::fromStdString(
        game.depositItem(townBackpackList->currentRow() - 1)));
    refreshAll();
}

void MainWindow::onWithdrawItem() {
    QMessageBox::information(this, "仓库", QString::fromStdString(
        game.withdrawItem(warehouseList->currentRow())));
    refreshAll();
}

void MainWindow::onDepositGold() {
    bool ok = false;
    int amount = QInputDialog::getInt(this, "存入金币", "数量:", 10, 1, 999999, 1, &ok);
    if (!ok) return;
    QMessageBox::information(this, "银行", QString::fromStdString(game.depositGold(amount)));
    refreshAll();
}

void MainWindow::onWithdrawGold() {
    bool ok = false;
    int amount = QInputDialog::getInt(this, "取出金币", "数量:", 10, 1, 999999, 1, &ok);
    if (!ok) return;
    QMessageBox::information(this, "银行", QString::fromStdString(game.withdrawGold(amount)));
    refreshAll();
}

void MainWindow::onReinforceItem() {
    int forgeRow = forgeList->currentRow();
    if (forgeRow < 0) {
        QMessageBox::information(this, "铁匠铺", "请选择一张强化卷。");
        return;
    }
    int invIndex = -1;
    int seen = -1;
    auto& inv = game.player->getInventory();
    for (size_t i = 0; i < inv.size(); ++i) {
        if (inv[i]->getName() == "武器强化卷" || inv[i]->getName() == "铠甲强化卷") {
            ++seen;
            if (seen == forgeRow) {
                invIndex = (int)i;
                break;
            }
        }
    }
    QMessageBox::information(this, "铁匠铺", QString::fromStdString(game.reinforceWithScroll(invIndex)));
    refreshAll();
}

// ================================================================
// TASK TAB
// ================================================================
void MainWindow::createTaskTab() {
    auto* w = new QWidget();
    auto* layout = new QHBoxLayout(w);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 15, 20, 15);

    // Left: task list
    auto* leftPanel = new QVBoxLayout();
    auto* title = new QLabel("📋 任务列表");
    title->setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    title->setAlignment(Qt::AlignCenter);
    leftPanel->addWidget(title);

    taskList = new QListWidget();
    taskList->setFont(QFont("Microsoft YaHei", 11));
    taskList->setAlternatingRowColors(true);
    taskList->setMinimumWidth(350);
    connect(taskList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && row < (int)game.tasks.size()) {
            auto& t = game.tasks[row];
            taskDetail->setText(QString(
                "任务: %1\n\n描述: %2\n\n条件: %3\n\n进度: %4/%5\n\n奖励: %6 EXP + %7 G\n\n状态: %8")
                .arg(QString::fromStdString(t->getName()))
                .arg(QString::fromStdString(t->getDesc()))
                .arg(QString::fromStdString(t->getCondition()))
                .arg(t->getProgress()).arg(t->getTarget())
                .arg(t->getExpReward()).arg(t->getGoldReward())
                .arg(t->isCompleted() ? "✅ 已完成" : (t->isAccepted() ? "● 进行中" : "○ 未接受")));
        }
    });
    leftPanel->addWidget(taskList);

    auto* btnRow = new QHBoxLayout();
    acceptBtn = new QPushButton("📝 接受任务");
    acceptBtn->setFixedHeight(36);
    acceptBtn->setStyleSheet("QPushButton { background: #2196F3; color: white; border-radius: 6px; }"
                             "QPushButton:hover { background: #1976D2; }");
    connect(acceptBtn, &QPushButton::clicked, this, &MainWindow::onAcceptTask);

    claimBtn = new QPushButton("🎁 领取奖励");
    claimBtn->setFixedHeight(36);
    claimBtn->setStyleSheet("QPushButton { background: #FFD700; color: #333; border-radius: 6px; }"
                            "QPushButton:hover { background: #FFC107; }");
    connect(claimBtn, &QPushButton::clicked, this, &MainWindow::onClaimReward);

    btnRow->addWidget(acceptBtn);
    btnRow->addWidget(claimBtn);
    leftPanel->addLayout(btnRow);

    layout->addLayout(leftPanel);

    // Right: task detail
    auto* rightPanel = new QVBoxLayout();
    auto* detailTitle = new QLabel("📄 任务详情");
    detailTitle->setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    detailTitle->setAlignment(Qt::AlignCenter);
    rightPanel->addWidget(detailTitle);

    taskDetail = new QTextEdit();
    taskDetail->setReadOnly(true);
    taskDetail->setFont(QFont("Microsoft YaHei", 11));
    taskDetail->setMinimumWidth(300);
    rightPanel->addWidget(taskDetail);

    layout->addLayout(rightPanel);
    tabs->addTab(w, "📋 任务");
}

void MainWindow::refreshTasks() {
    taskList->clear();
    for (size_t i = 0; i < game.tasks.size(); ++i) {
        auto& t = game.tasks[i];
        QString status = t->isCompleted() ? "✅" : (t->isAccepted() ? "●" : "○");
        taskList->addItem(QString("%1 [%2] %3").arg(status).arg(i+1).arg(
            QString::fromStdString(t->getName())));
    }
}

void MainWindow::onAcceptTask() {
    int idx = taskList->currentRow();
    if (idx < 0) {
        QMessageBox::information(this, "提示", "请先选择任务!");
        return;
    }
    if (game.tasks[idx]->isAccepted()) {
        QMessageBox::information(this, "提示", "该任务已接受!");
        return;
    }
    game.tasks[idx]->accept();
    QMessageBox::information(this, "接受任务",
        QString("接受了任务: %1").arg(QString::fromStdString(game.tasks[idx]->getName())));
    refreshTasks();
    taskList->setCurrentRow(idx);  // Refresh detail
}

void MainWindow::onClaimReward() {
    int idx = taskList->currentRow();
    if (idx < 0) {
        QMessageBox::information(this, "提示", "请先选择任务!");
        return;
    }
    auto& t = game.tasks[idx];
    if (!t->isCompleted()) {
        QMessageBox::information(this, "提示", "任务尚未完成!");
        return;
    }
    std::string result = t->claimReward();
    game.player->gainExp(t->getExpReward());
    game.player->addGold(t->getGoldReward());
    QMessageBox::information(this, "领取奖励", QString::fromStdString(result));
    refreshTasks();
    refreshCharacterPanel();
}

// ================================================================
// MAP TAB
// ================================================================
void MainWindow::createMapTab() {
    auto* w = new QWidget();
    auto* layout = new QHBoxLayout(w);
    layout->setSpacing(12);
    layout->setContentsMargins(15, 10, 15, 10);

    // LEFT: Visual map
    auto* leftPanel = new QVBoxLayout();
    mapWidget = new MapWidget();
    mapWidget->setMap(game.map.get());
    mapWidget->setCurrentLocation(0);
    connect(mapWidget, &MapWidget::locationClicked, this, &MainWindow::onMapLocationClicked);
    leftPanel->addWidget(mapWidget);
    layout->addLayout(leftPanel, 2);

    // RIGHT: Info panel
    auto* rightPanel = new QVBoxLayout();
    rightPanel->setSpacing(8);

    auto* infoGroup = new QGroupBox("📍 当前位置");
    auto* infoLayout = new QVBoxLayout(infoGroup);
    infoLayout->setSpacing(5);
    mapLocationLabel = new QLabel();
    mapLocationLabel->setFont(QFont("Microsoft YaHei", 15, QFont::Bold));
    mapLocationLabel->setStyleSheet("color: #E65100;");
    infoLayout->addWidget(mapLocationLabel);

    mapDescLabel = new QLabel();
    mapDescLabel->setFont(QFont("Microsoft YaHei", 11));
    mapDescLabel->setWordWrap(true);
    infoLayout->addWidget(mapDescLabel);

    // Special features
    restBtn = new QPushButton("😴 休息恢复");
    restBtn->setFixedHeight(36);
    restBtn->setStyleSheet("QPushButton { background: #66BB6A; color: white; border-radius: 6px; }"
                           "QPushButton:hover { background: #43A047; }");
    connect(restBtn, &QPushButton::clicked, this, &MainWindow::onRest);
    infoLayout->addWidget(restBtn);

    adventureBtn = new QPushButton("出城探险");
    adventureBtn->setFixedHeight(36);
    adventureBtn->setStyleSheet("QPushButton { background: #EF6C00; color: white; border-radius: 6px; }"
                                "QPushButton:hover { background: #E65100; }");
    connect(adventureBtn, &QPushButton::clicked, this, &MainWindow::onAdventure);
    infoLayout->addWidget(adventureBtn);

    returnTownBtn = new QPushButton("休息点回城");
    returnTownBtn->setFixedHeight(36);
    returnTownBtn->setStyleSheet("QPushButton { background: #455A64; color: white; border-radius: 6px; }"
                                 "QPushButton:hover { background: #263238; }");
    connect(returnTownBtn, &QPushButton::clicked, this, &MainWindow::onReturnToTown);
    infoLayout->addWidget(returnTownBtn);

    rightPanel->addWidget(infoGroup);

    // Reachable locations
    auto* reachGroup = new QGroupBox("🚶 可前往的位置");
    auto* reachLayout = new QVBoxLayout(reachGroup);
    reachableList = new QListWidget();
    reachableList->setFont(QFont("Microsoft YaHei", 11));
    reachableList->setMaximumHeight(150);
    connect(reachableList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) {
        int idx = reachableList->currentRow();
        if (idx >= 0) {
            auto reachable = mapWidget->getReachable();
            if (idx < (int)reachable.size()) {
                onMapLocationClicked(reachable[idx]);
            }
        }
    });
    reachLayout->addWidget(reachableList);
    rightPanel->addWidget(reachGroup);

    // Message log
    auto* logGroup = new QGroupBox("📜 探索日志");
    auto* logLayout = new QVBoxLayout(logGroup);
    mapLog = new QTextEdit();
    mapLog->setReadOnly(true);
    mapLog->setFont(QFont("Microsoft YaHei", 10));
    mapLog->setMaximumHeight(150);
    logLayout->addWidget(mapLog);
    rightPanel->addWidget(logGroup);

    rightPanel->addStretch();
    layout->addLayout(rightPanel, 1);

    tabs->addTab(w, "🗺️ 地图");
}

void MainWindow::onMapLocationClicked(int idx) {
    if (!game.map || !game.player) return;

    // Check for random encounter first
    int encounter = game.checkEncounter();
    if (encounter >= 0) {
        auto& en = game.enemies[encounter];
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "遭遇敌人!",
            QString("在探索途中遇到了 %1!\n\n是否战斗?").arg(
                QString::fromStdString(en->getName())),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            // Switch to battle tab and fight this enemy
            currentEnemyIndex = encounter;
            enemyCombo->setCurrentIndex(encounter);
            tabs->setCurrentIndex(7);  // Battle tab
            onStartBattle();
        }
    }

    // Move
    std::string result = game.moveTo(idx);
    mapLog->append(QString::fromStdString(result));
    refreshMapPanel();
    refreshCharacterPanel();

    // Check encounter after move too
    int encounter2 = game.checkEncounter();
    if (encounter2 >= 0 && encounter < 0) {  // Only if didn't just fight
        auto& en = game.enemies[encounter2];
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "遭遇敌人!",
            QString("到达后遇到了 %1!\n\n是否战斗?").arg(
                QString::fromStdString(en->getName())),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            currentEnemyIndex = encounter2;
            enemyCombo->setCurrentIndex(encounter2);
            tabs->setCurrentIndex(7);
            onStartBattle();
        }
    }
}

void MainWindow::onRest() {
    std::string result = game.rest();
    mapLog->append(QString::fromStdString(result));
    refreshCharacterPanel();
    QMessageBox::information(this, "休息", QString::fromStdString(result));
}

void MainWindow::onAdventure() {
    if (!game.isTown()) {
        QMessageBox::information(this, "冒险", "只有在城镇节点可以整备后出城探险。");
        return;
    }
    auto reply = QMessageBox::question(this, "出城探险",
        "出城前请确认背包里已经携带药水。死亡会掉落身上半数金币，背包每件物品都有50%概率掉落。\n每完成3次战斗会发现一个休息点，可通过休息点回城。\n\n是否出城？",
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    std::vector<int> wild;
    for (size_t i = 0; i < game.map->locations.size(); ++i) {
        const auto& loc = game.map->locations[i];
        if (!loc.hasRest && !loc.hasShop) wild.push_back((int)i);
    }
    if (wild.empty()) return;
    int target = wild[rand() % wild.size()];
    game.map->currentLocation = target;
    mapLog->append(QString("你从城镇出发，抵达了 %1。")
        .arg(QString::fromStdString(game.map->locations[target].name)));
    refreshAll();
}

void MainWindow::onReturnToTown() {
    std::string result = game.returnToTown();
    mapLog->append(QString::fromStdString(result));
    QMessageBox::information(this, "休息点", QString::fromStdString(result));
    refreshAll();
}

void MainWindow::refreshMapPanel() {
    if (!game.map) return;
    auto& loc = game.map->getCurrentLocation();
    mapLocationLabel->setText(QString::fromStdString(loc.icon + " " + loc.name));
    mapDescLabel->setText(QString("%1\n城镇/区域：%2（推荐等级 %3-%4）\n休息点：%5")
        .arg(QString::fromStdString(loc.description))
        .arg(QString::fromStdString(game.currentTownName()))
        .arg(game.currentTownMinLevel())
        .arg(game.currentTownMaxLevel())
        .arg(game.restPointAvailable ? "已发现，可回城" : "未发现"));
    mapWidget->setCurrentLocation(game.map->currentLocation);

    // Show/hide rest button
    restBtn->setVisible(loc.hasRest);
    restBtn->setEnabled(loc.hasRest);
    adventureBtn->setEnabled(game.isTown());
    returnTownBtn->setEnabled(game.restPointAvailable || game.isTown());

    // Update reachable list
    reachableList->clear();
    auto reachable = game.map->getReachableLocations();
    for (int idx : reachable) {
        auto& rloc = game.map->locations[idx];
        QString info = QString::fromStdString(rloc.icon + " " + rloc.name);
        if (rloc.hasShop) info += " [商店]";
        if (rloc.hasRest) info += " [可休息]";
        if (rloc.hasShop || rloc.hasRest) info += " [城镇服务]";
        reachableList->addItem(info);
    }
}

void MainWindow::refreshAll() {
    refreshSaveSlots();
    refreshCharacterPanel();
    refreshInventory();
    refreshShop();
    refreshTownServices();
    refreshTasks();
    refreshMapPanel();
    refreshBattle();
}

// ================================================================
// BATTLE TAB
// ================================================================
void MainWindow::createBattleTab() {
    auto* w = new QWidget();
    auto* layout = new QVBoxLayout(w);
    layout->setSpacing(10);
    layout->setContentsMargins(30, 15, 30, 15);

    auto* title = new QLabel("⚔️ 战斗系统");
    QFont titleFont("Microsoft YaHei", 20, QFont::Bold);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // Enemy selection
    auto* selRow = new QHBoxLayout();
    selRow->addWidget(new QLabel("选择敌人:"));
    enemyCombo = new QComboBox();
    enemyCombo->setFont(QFont("Microsoft YaHei", 11));
    enemyCombo->setMinimumWidth(250);
    selRow->addWidget(enemyCombo);

    battleBtn = new QPushButton("⚔️ 开始战斗!");
    battleBtn->setFixedHeight(40);
    battleBtn->setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    battleBtn->setStyleSheet("QPushButton { background: #f44336; color: white; border-radius: 8px; }"
                             "QPushButton:hover { background: #D32F2F; }"
                             "QPushButton:disabled { background: #ccc; }");
    connect(battleBtn, &QPushButton::clicked, this, &MainWindow::onStartBattle);
    selRow->addWidget(battleBtn);
    layout->addLayout(selRow);

    // HP bars
    auto* hpRow = new QHBoxLayout();

    // Player HP
    auto* pHpBox = new QVBoxLayout();
    auto* pHpTitle = new QLabel("我的生命值");
    pHpTitle->setAlignment(Qt::AlignCenter);
    pHpTitle->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    pHpBox->addWidget(pHpTitle);
    battleHpBar = new QProgressBar();
    battleHpBar->setFixedHeight(28);
    battleHpBar->setStyleSheet("QProgressBar { border: 1px solid #ccc; border-radius: 10px; background: #eee; }"
                               "QProgressBar::chunk { background: #4CAF50; border-radius: 10px; }");
    pHpBox->addWidget(battleHpBar);
    battleHpLabel = new QLabel();
    battleHpLabel->setAlignment(Qt::AlignCenter);
    pHpBox->addWidget(battleHpLabel);
    hpRow->addLayout(pHpBox);

    battleStatus = new QLabel("⚡");
    battleStatus->setFont(QFont("Microsoft YaHei", 18));
    battleStatus->setAlignment(Qt::AlignCenter);
    battleStatus->setFixedWidth(60);
    hpRow->addWidget(battleStatus);

    // Enemy HP
    auto* eHpBox = new QVBoxLayout();
    auto* eHpTitle = new QLabel("敌人生命值");
    eHpTitle->setAlignment(Qt::AlignCenter);
    eHpTitle->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    eHpBox->addWidget(eHpTitle);
    enemyHpBar = new QProgressBar();
    enemyHpBar->setFixedHeight(28);
    enemyHpBar->setStyleSheet("QProgressBar { border: 1px solid #ccc; border-radius: 10px; background: #eee; }"
                              "QProgressBar::chunk { background: #f44336; border-radius: 10px; }");
    eHpBox->addWidget(enemyHpBar);
    enemyHpLabel = new QLabel();
    enemyHpLabel->setAlignment(Qt::AlignCenter);
    eHpBox->addWidget(enemyHpLabel);
    hpRow->addLayout(eHpBox);

    layout->addLayout(hpRow);

    // Battle log
    auto* logTitle = new QLabel("📜 战斗日志");
    logTitle->setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    layout->addWidget(logTitle);

    battleLog = new QTextEdit();
    battleLog->setReadOnly(true);
    battleLog->setFont(QFont("Consolas", 10));
    battleLog->setMinimumHeight(250);
    layout->addWidget(battleLog);

    auto* battleItemGroup = new QGroupBox("战斗背包");
    auto* itemLayout = new QHBoxLayout(battleItemGroup);
    battleItemList = new QListWidget();
    battleItemList->setAlternatingRowColors(true);
    itemLayout->addWidget(battleItemList, 1);
    useBattleItemBtn = new QPushButton("使用选中物品");
    useBattleItemBtn->setFixedHeight(40);
    connect(useBattleItemBtn, &QPushButton::clicked, this, &MainWindow::onUseBattleItem);
    itemLayout->addWidget(useBattleItemBtn);
    layout->addWidget(battleItemGroup);

    // Timer for battle ticks
    battleTimer = new QTimer(this);
    connect(battleTimer, &QTimer::timeout, this, &MainWindow::onBattleTick);

    tabs->addTab(w, "⚔️ 战斗");
}

void MainWindow::refreshBattle() {
    if (!inBattle) {
        enemyCombo->clear();
        for (size_t i = 0; i < game.enemies.size(); ++i) {
            auto& e = game.enemies[i];
            enemyCombo->addItem(QString("%1 (HP:%2 ATK:%3 DEF:%4 奖励:%5EXP/%6G)")
                .arg(QString::fromStdString(e->getName()))
                .arg(e->getMaxHp()).arg(e->getAttack()).arg(e->getDefense())
                .arg(e->getExpReward()).arg(e->getGoldReward()));
        }

        if (game.player) {
            battleHpBar->setMaximum(game.player->getMaxHp());
            battleHpBar->setValue(game.player->getHp());
            battleHpLabel->setText(QString("%1 / %2").arg(game.player->getHp()).arg(game.player->getMaxHp()));
        }
        enemyHpBar->setValue(0);
        enemyHpLabel->setText("--");
        battleStatus->setText("⚡");
    } else if (game.player && currentEnemyIndex >= 0 && currentEnemyIndex < (int)game.enemies.size()) {
        auto& enemy = game.enemies[currentEnemyIndex];
        battleHpBar->setMaximum(game.player->getMaxHp());
        battleHpBar->setValue(game.player->getHp());
        battleHpLabel->setText(QString("%1 / %2").arg(game.player->getHp()).arg(game.player->getMaxHp()));
        enemyHpBar->setMaximum(enemy->getMaxHp());
        enemyHpBar->setValue(enemy->getHp());
        enemyHpLabel->setText(QString("%1 / %2").arg(enemy->getHp()).arg(enemy->getMaxHp()));
    }

    battleItemList->clear();
    if (game.player) {
        auto& inv = game.player->getInventory();
        for (size_t i = 0; i < inv.size(); ++i) {
            battleItemList->addItem(QString("[%1] %2").arg(i + 1).arg(
                QString::fromStdString(inv[i]->getInfo())));
        }
        if (inv.empty()) battleItemList->addItem("背包为空。");
    }
    useBattleItemBtn->setEnabled(inBattle);
}

void MainWindow::onStartBattle() {
    if (inBattle) {
        QMessageBox::information(this, "提示", "正在战斗中!");
        return;
    }

    int idx = enemyCombo->currentIndex();
    if (idx < 0) return;

    currentEnemyIndex = idx;

    inBattle = true;
    rageTurnsLeft = 0;
    rageAttackBonus = 0;
    battleBtn->setEnabled(false);
    enemyCombo->setEnabled(false);
    battleLog->clear();
    battleLog->append(QString("⚔️ 开始战斗! 挑战 %1!").arg(
        QString::fromStdString(game.enemies[idx]->getName())));

    // Reset player HP for new battle (restore from game state)
    // We'll use the current player HP directly

    enemyHpBar->setMaximum(game.enemies[idx]->getMaxHp());
    enemyHpBar->setValue(game.enemies[idx]->getMaxHp());
    enemyHpLabel->setText(QString("%1 / %2").arg(game.enemies[idx]->getMaxHp()).arg(game.enemies[idx]->getMaxHp()));

    // Start battle animation
    battleTimer->start(800);
    refreshBattle();
}

void MainWindow::onUseBattleItem() {
    if (!inBattle || !game.player) {
        QMessageBox::information(this, "战斗背包", "只有战斗中可以使用战斗背包。");
        return;
    }
    int idx = battleItemList->currentRow();
    auto& inv = game.player->getInventory();
    if (idx < 0 || idx >= (int)inv.size()) {
        QMessageBox::information(this, "战斗背包", "请选择一件物品。");
        return;
    }
    std::string itemName = inv[idx]->getName();
    std::string result = inv[idx]->useInBattle(*game.player, game.enemies[currentEnemyIndex].get());
    if (itemName == "狂暴药水") {
        rageAttackBonus += 8;
        rageTurnsLeft = std::max(rageTurnsLeft, 3);
    }
    inv.erase(inv.begin() + idx);
    battleLog->append(QString("使用 %1：%2")
        .arg(QString::fromStdString(itemName))
        .arg(QString::fromStdString(result)));
    refreshAll();
}

void MainWindow::onBattleTick() {
    if (!inBattle || !game.player) return;

    auto& enemy = game.enemies[currentEnemyIndex];

    // Player attacks
    int pdmg = game.player->dealDamage();
    enemy->takeDamage(pdmg);
    battleLog->append(QString("⚔️ 你造成 %1 点伤害!").arg(pdmg));
    enemyHpBar->setValue(enemy->getHp());
    enemyHpLabel->setText(QString("%1 / %2").arg(enemy->getHp()).arg(enemy->getMaxHp()));
    battleHpBar->setValue(game.player->getHp());
    battleHpLabel->setText(QString("%1 / %2").arg(game.player->getHp()).arg(game.player->getMaxHp()));

    // Check enemy defeat
    if (!enemy->isAlive()) {
        battleTimer->stop();
        inBattle = false;
        if (rageAttackBonus > 0) {
            game.player->buffAttack(-rageAttackBonus);
            rageAttackBonus = 0;
            rageTurnsLeft = 0;
        }
        battleBtn->setEnabled(true);
        enemyCombo->setEnabled(true);
        battleStatus->setText("🎉");
        battleLog->append("══════════════════");
        battleLog->append(QString("🎉 胜利! 击败了 %1!").arg(
            QString::fromStdString(enemy->getName())));

        int er = enemy->getExpReward();
        int gr = enemy->getGoldReward();
        game.player->gainExp(er);
        game.player->addGold(gr);
        battleLog->append(QString("获得 %1 EXP + %2 G!").arg(er).arg(gr));
        battleLog->append(QString::fromStdString(game.addBattleRewardItem()));
        game.recordBattleFinished();
        if (game.restPointAvailable) {
            battleLog->append("连续战斗后发现了休息点，可以回城整备。");
        }

        // Reset enemy HP
        enemy->resetBattleState();
        refreshAll();
        return;
    }

    // Enemy attacks
    if (enemy->consumeFatigueSkip()) {
        battleLog->append(QString("%1 受到疲惫影响，跳过了本回合行动。")
            .arg(QString::fromStdString(enemy->getName())));
        refreshBattle();
        if (rageTurnsLeft > 0) {
            --rageTurnsLeft;
            if (rageTurnsLeft == 0 && rageAttackBonus > 0) {
                game.player->buffAttack(-rageAttackBonus);
                battleLog->append("狂暴药水效果结束。");
                rageAttackBonus = 0;
                refreshCharacterPanel();
            }
        }
        return;
    }

    int edmg = enemy->attackPlayer();
    if (edmg < 0) {
        battleTimer->stop();
        inBattle = false;
        if (rageAttackBonus > 0) {
            game.player->buffAttack(-rageAttackBonus);
            rageAttackBonus = 0;
            rageTurnsLeft = 0;
        }
        battleBtn->setEnabled(true);
        enemyCombo->setEnabled(true);
        battleLog->append(QString("💨 %1 逃跑了!").arg(
            QString::fromStdString(enemy->getName())));
        return;
    }

    // Check for special effects
    if (dynamic_cast<Boss*>(enemy.get())) {
        if (edmg > enemy->getAttack() + 10) {
            battleLog->append(QString("🔥 %1 使用技能!").arg(
                QString::fromStdString(enemy->getName())));
        }
    }
    if (auto* elite = dynamic_cast<EliteMonster*>(enemy.get())) {
        if (elite->isCrit()) {
            battleLog->append("💥 暴击!");
        }
    }

    game.player->takeDamage(edmg);
    battleLog->append(QString("💢 %1 造成 %2 点伤害!").arg(
        QString::fromStdString(enemy->getName())).arg(edmg));
    battleHpBar->setValue(game.player->getHp());
    battleHpLabel->setText(QString("%1 / %2").arg(game.player->getHp()).arg(game.player->getMaxHp()));

    // Check player defeat
    if (!game.player->isAlive()) {
        battleTimer->stop();
        inBattle = false;
        if (rageAttackBonus > 0) {
            game.player->buffAttack(-rageAttackBonus);
            rageAttackBonus = 0;
            rageTurnsLeft = 0;
        }
        battleBtn->setEnabled(true);
        enemyCombo->setEnabled(true);
        battleStatus->setText("💀");
        battleLog->append("══════════════════");
        battleLog->append("💀 你被击败了...");
        int lostGold = game.player->getGold() / 2;
        battleLog->append(QString("失去了身上金币的一半：%1 G").arg(lostGold));
        game.player->addGold(-lostGold);
        auto& inv = game.player->getInventory();
        int lostItems = 0;
        for (int i = (int)inv.size() - 1; i >= 0; --i) {
            if (rand() % 2 == 0) {
                battleLog->append(QString("掉落了 %1").arg(QString::fromStdString(inv[i]->getName())));
                inv.erase(inv.begin() + i);
                ++lostItems;
            }
        }
        if (lostItems == 0) battleLog->append("背包物品没有掉落。");
        game.recordBattleFinished();
        if (game.restPointAvailable) {
            battleLog->append("战斗后发现了休息点，可以回城整备。");
        }
        // Heal player a bit after defeat
        game.player->heal(game.player->getMaxHp() / 2);
        refreshAll();
        return;
    }

    if (rageTurnsLeft > 0) {
        --rageTurnsLeft;
        if (rageTurnsLeft == 0 && rageAttackBonus > 0) {
            game.player->buffAttack(-rageAttackBonus);
            battleLog->append("狂暴药水效果结束。");
            rageAttackBonus = 0;
            refreshCharacterPanel();
        }
    }
}

// ================================================================
// STYLE
// ================================================================
void MainWindow::applyStyleSheet() {
    setStyleSheet(
        "QMainWindow { background: #ECEFF1; }"
        "QTabWidget::pane { border: 1px solid #B0BEC5; background: #FAFAFA; border-radius: 4px; }"
        "QTabBar::tab { padding: 10px 20px; font-size: 13px; font-weight: bold;"
        "  border: 1px solid #B0BEC5; border-bottom: none; border-radius: 6px 6px 0 0;"
        "  background: #CFD8DC; margin-right: 3px; }"
        "QTabBar::tab:selected { background: #FAFAFA; border-bottom: 2px solid #1565C0; }"
        "QTabBar::tab:hover { background: #ECEFF1; }"
        "QGroupBox { background: white; border: 2px solid #90A4AE; border-radius: 10px;"
        "  padding: 15px; font-size: 13px; font-weight: bold; }"
        "QListWidget { border: 1px solid #B0BEC5; border-radius: 6px; padding: 5px;"
        "  background: white; }"
        "QListWidget::item { padding: 6px; border-bottom: 1px solid #ECEFF1; }"
        "QListWidget::item:selected { background: #BBDEFB; color: #333; }"
        "QListWidget::item:alternate { background: #F5F5F5; }"
        "QTextEdit { border: 1px solid #B0BEC5; border-radius: 6px; padding: 8px;"
        "  background: white; }"
        "QComboBox { padding: 6px 12px; border: 1px solid #B0BEC5; border-radius: 4px;"
        "  background: white; font-size: 11px; }"
        "QPushButton { padding: 8px 18px; border: none; border-radius: 6px;"
        "  font-size: 12px; font-weight: bold; }"
        "QLabel { color: #37474F; }"
        "QProgressBar { text-align: center; }"
    );
}
