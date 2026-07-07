#include "MainWindow.h"
#include <QApplication>
#include <QFont>
#include <QFrame>
#include <QSplitter>
#include <QScrollArea>
#include <QHeaderView>
#include <sstream>
#include <iomanip>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), currentEnemyIndex(0), inBattle(false) {
    setWindowTitle("校园RPG冒险游戏系统 - Campus RPG Adventure");
    setMinimumSize(1024, 720);
    resize(1100, 780);

    // Ask for player name
    bool ok;
    QString name = QInputDialog::getText(this, "创建角色",
        "请输入你的角色名称:", QLineEdit::Normal, "勇者", &ok);
    if (!ok || name.isEmpty()) name = "无名勇者";
    game.initPlayer(name.toStdString());

    setupUI();
    applyStyleSheet();
    refreshAll();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    tabs = new QTabWidget(this);
    setCentralWidget(tabs);

    createCharacterTab();
    createInventoryTab();
    createShopTab();
    createTaskTab();
    createMapTab();
    createBattleTab();

    tabs->setCurrentIndex(4);  // Start on map tab
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
    charLevelLabel->setText(QString("等级 Lv.%1  |  背包物品: %2 件")
        .arg(c->getLevel()).arg(c->inventoryCount()));

    hpBar->setMaximum(c->getMaxHp());
    hpBar->setValue(c->getHp());
    hpLabel->setText(QString("%1 / %2").arg(c->getHp()).arg(c->getMaxHp()));

    expBar->setMaximum(c->getExpToNextLevel());
    expBar->setValue(c->getExp());
    expLabel->setText(QString("%1 / %2").arg(c->getExp()).arg(c->getExpToNextLevel()));

    goldLabel->setText(QString("💰 金币: %1").arg(c->getGold()));
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
    for (size_t i = 0; i < inv.size(); ++i) {
        invList->addItem(QString("[%1] %2").arg(i+1).arg(
            QString::fromStdString(inv[i]->getInfo())));
    }
    if (inv.empty()) {
        invList->addItem("背包为空 —— 去商店购买物品吧!");
    }
}

void MainWindow::onUseItem() {
    int idx = invList->currentRow();
    if (idx < 0) {
        QMessageBox::information(this, "提示", "请先选择一件物品!");
        return;
    }
    std::string result = game.player->useItem(idx);
    QMessageBox::information(this, "使用物品", QString::fromStdString(result));
    refreshInventory();
    refreshCharacterPanel();
}

void MainWindow::onDropItem() {
    int idx = invList->currentRow();
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

    shopGoldLabel->setText(QString("💰 我的金币: %1 G").arg(game.player->getGold()));
}

void MainWindow::onBuyItem() {
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
            tabs->setCurrentIndex(5);  // Battle tab
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
            tabs->setCurrentIndex(5);
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

void MainWindow::refreshMapPanel() {
    if (!game.map) return;
    auto& loc = game.map->getCurrentLocation();
    mapLocationLabel->setText(QString::fromStdString(loc.icon + " " + loc.name));
    mapDescLabel->setText(QString::fromStdString(loc.description));
    mapWidget->setCurrentLocation(game.map->currentLocation);

    // Show/hide rest button
    restBtn->setVisible(loc.hasRest);
    restBtn->setEnabled(loc.hasRest);

    // Update reachable list
    reachableList->clear();
    auto reachable = game.map->getReachableLocations();
    for (int idx : reachable) {
        auto& rloc = game.map->locations[idx];
        QString info = QString::fromStdString(rloc.icon + " " + rloc.name);
        if (rloc.hasShop) info += " [商店]";
        if (rloc.hasRest) info += " [可休息]";
        reachableList->addItem(info);
    }
}

void MainWindow::refreshAll() {
    refreshCharacterPanel();
    refreshInventory();
    refreshShop();
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

    // Timer for battle ticks
    battleTimer = new QTimer(this);
    connect(battleTimer, &QTimer::timeout, this, &MainWindow::onBattleTick);

    tabs->addTab(w, "⚔️ 战斗");
}

void MainWindow::refreshBattle() {
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
}

void MainWindow::onStartBattle() {
    if (inBattle) {
        QMessageBox::information(this, "提示", "正在战斗中!");
        return;
    }

    int idx = enemyCombo->currentIndex();
    if (idx < 0) return;

    // Create a fresh copy of the enemy for battle
    auto& templateEnemy = game.enemies[idx];
    currentEnemyIndex = idx;

    inBattle = true;
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

        // Reset enemy HP
        enemy->takeDamage(-9999); // Force reset next time
        refreshCharacterPanel();
        refreshTasks();
        return;
    }

    // Enemy attacks
    int edmg = enemy->attackPlayer();
    if (edmg < 0) {
        battleTimer->stop();
        inBattle = false;
        battleBtn->setEnabled(true);
        enemyCombo->setEnabled(true);
        battleLog->append(QString("💨 %1 逃跑了!").arg(
            QString::fromStdString(enemy->getName())));
        return;
    }

    // Check for special effects
    if (auto* boss = dynamic_cast<Boss*>(enemy.get())) {
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
        battleBtn->setEnabled(true);
        enemyCombo->setEnabled(true);
        battleStatus->setText("💀");
        battleLog->append("══════════════════");
        battleLog->append("💀 你被击败了...");
        battleLog->append(QString("失去了 %1 G").arg(game.player->getGold() / 5));
        game.player->addGold(-game.player->getGold() / 5);
        // Heal player a bit after defeat
        game.player->heal(game.player->getMaxHp() / 2);
        refreshCharacterPanel();
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
