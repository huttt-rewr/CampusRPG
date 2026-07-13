// MainWindow.cpp
// Qt 主窗口实现：把角色、背包、商店、任务、战斗和存档做成窗口界面。
#include "MainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSettings>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("校园RPG冒险游戏系统");
    resize(1050, 720);
    setupData();
    setupUi();
    loadGame();
    refreshAll();
}

void MainWindow::setupUi() {
    tabs = new QTabWidget(this);
    setCentralWidget(tabs);

    auto* rolePage = new QWidget(this);
    auto* roleLayout = new QVBoxLayout(rolePage);
    roleLabel = new QLabel(rolePage);
    roleLabel->setWordWrap(true);
    auto* newBtn = new QPushButton("新建角色", rolePage);
    auto* saveBtn = new QPushButton("保存存档", rolePage);
    auto* loadBtn = new QPushButton("读取存档", rolePage);
    roleLayout->addWidget(roleLabel);
    roleLayout->addWidget(newBtn);
    roleLayout->addWidget(saveBtn);
    roleLayout->addWidget(loadBtn);
    roleLayout->addStretch();
    tabs->addTab(rolePage, "角色");

    auto* bagPage = new QWidget(this);
    auto* bagLayout = new QVBoxLayout(bagPage);
    inventoryList = new QListWidget(bagPage);
    auto* useBtn = new QPushButton("使用选中物品", bagPage);
    auto* delBtn = new QPushButton("删除选中物品", bagPage);
    bagLayout->addWidget(inventoryList);
    bagLayout->addWidget(useBtn);
    bagLayout->addWidget(delBtn);
    tabs->addTab(bagPage, "背包");

    auto* shopPage = new QWidget(this);
    auto* shopLayout = new QVBoxLayout(shopPage);
    shopList = new QListWidget(shopPage);
    auto* buyBtn = new QPushButton("购买选中商品", shopPage);
    auto* sellBtn = new QPushButton("出售背包选中物品", shopPage);
    shopLayout->addWidget(shopList);
    shopLayout->addWidget(buyBtn);
    shopLayout->addWidget(sellBtn);
    tabs->addTab(shopPage, "商店");

    auto* taskPage = new QWidget(this);
    auto* taskLayout = new QVBoxLayout(taskPage);
    taskList = new QListWidget(taskPage);
    auto* acceptBtn = new QPushButton("接受选中任务", taskPage);
    auto* claimBtn = new QPushButton("领取选中奖励", taskPage);
    taskLayout->addWidget(taskList);
    taskLayout->addWidget(acceptBtn);
    taskLayout->addWidget(claimBtn);
    tabs->addTab(taskPage, "任务");

    auto* battlePage = new QWidget(this);
    auto* battleLayout = new QVBoxLayout(battlePage);
    auto* normalBtn = new QPushButton("挑战普通敌人：校园占座狗", battlePage);
    auto* eliteBtn = new QPushButton("挑战精英敌人：教务处查课老师", battlePage);
    auto* bossBtn = new QPushButton("挑战BOSS：期末挂科神龙", battlePage);
    logText = new QTextEdit(battlePage);
    logText->setReadOnly(true);
    battleLayout->addWidget(normalBtn);
    battleLayout->addWidget(eliteBtn);
    battleLayout->addWidget(bossBtn);
    battleLayout->addWidget(logText, 1);
    tabs->addTab(battlePage, "战斗");

    setStyleSheet(
        "QMainWindow{background:#f5f7fb;color:#1f2933;}"
        "QTabWidget::pane{border:1px solid #cfd7e6;background:white;}"
        "QTabBar::tab{padding:9px 16px;background:#e7ecf5;border:1px solid #cfd7e6;}"
        "QTabBar::tab:selected{background:white;font-weight:600;}"
        "QLabel{font-size:15px;line-height:1.5;}"
        "QPushButton{padding:8px 12px;border:1px solid #9fb0c5;border-radius:6px;background:white;}"
        "QPushButton:hover{background:#edf4ff;}"
        "QListWidget,QTextEdit{background:white;border:1px solid #cfd7e6;border-radius:6px;padding:6px;}"
    );

    connect(newBtn, &QPushButton::clicked, this, &MainWindow::newRole);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::saveGame);
    connect(loadBtn, &QPushButton::clicked, this, &MainWindow::loadGame);
    connect(useBtn, &QPushButton::clicked, this, &MainWindow::useSelectedItem);
    connect(delBtn, &QPushButton::clicked, this, &MainWindow::deleteSelectedItem);
    connect(buyBtn, &QPushButton::clicked, this, &MainWindow::buySelectedItem);
    connect(sellBtn, &QPushButton::clicked, this, &MainWindow::sellSelectedItem);
    connect(acceptBtn, &QPushButton::clicked, this, &MainWindow::acceptSelectedTask);
    connect(claimBtn, &QPushButton::clicked, this, &MainWindow::claimSelectedTask);
    connect(normalBtn, &QPushButton::clicked, this, &MainWindow::fightNormalEnemy);
    connect(eliteBtn, &QPushButton::clicked, this, &MainWindow::fightEliteEnemy);
    connect(bossBtn, &QPushButton::clicked, this, &MainWindow::fightBossEnemy);
}

void MainWindow::setupData() {
    goods = {
        {"校园食堂盒饭", "食物", 30, "非战斗使用，回复30生命。", 30, 0, 0, false, false},
        {"校医院退烧药", "药品", 60, "战斗/非战斗都可使用，回复70生命。", 70, 0, 0, false, true},
        {"图书馆占座笔", "装备", 120, "切换穿戴，攻击+10，防御+4。", 0, 10, 4, false, false}
    };
    tasks = {
        {1, "赶走占座狗", "帮同学夺回图书馆座位。", "击败", "校园占座狗", 2, 0, 0, 80, 50, "校园食堂盒饭"},
        {2, "收集应急药", "为班级药箱准备校医院退烧药。", "收集", "校医院退烧药", 1, 0, 0, 60, 40, ""},
        {3, "直面挂科传说", "击败期末挂科神龙。", "击败", "期末挂科神龙", 1, 0, 0, 250, 200, "图书馆占座笔"}
    };
}

void MainWindow::refreshAll() {
    refreshRole();
    refreshInventory();
    refreshShop();
    refreshTasks();
}

void MainWindow::refreshRole() {
    roleLabel->setText(QString(
        "姓名：%1\n等级：Lv.%2\n生命：%3/%4\n经验：%5/%6\n金币：%7\n攻击力：%8\n防御力：%9\n背包容量：%10/20")
        .arg(role.name).arg(role.level).arg(role.hp).arg(role.maxHp)
        .arg(role.exp).arg(role.expNeed).arg(role.gold).arg(role.attack)
        .arg(role.defense).arg(inventory.size()));
}

void MainWindow::refreshInventory() {
    inventoryList->clear();
    for (int i = 0; i < inventory.size(); ++i) {
        inventoryList->addItem(QString("%1. %2").arg(i + 1).arg(itemText(inventory[i])));
    }
}

void MainWindow::refreshShop() {
    shopList->clear();
    for (int i = 0; i < goods.size(); ++i) {
        shopList->addItem(QString("%1. %2").arg(i + 1).arg(itemText(goods[i])));
    }
}

void MainWindow::refreshTasks() {
    taskList->clear();
    for (const auto& task : tasks) {
        taskList->addItem(QString("[%1] 任务%2：%3 | %4%5 %6/%7 | 奖励：%8经验 %9金币 %10")
            .arg(taskStatusText(task.status)).arg(task.id).arg(task.name)
            .arg(task.conditionType).arg(task.target).arg(task.progress).arg(task.need)
            .arg(task.rewardExp).arg(task.rewardGold).arg(task.rewardItem));
    }
}

void MainWindow::appendLog(const QString& text) {
    logText->append(text);
}

void MainWindow::addExp(int amount) {
    role.exp += amount;
    while (role.exp >= role.expNeed) {
        role.exp -= role.expNeed;
        role.level++;
        role.expNeed = int(role.expNeed * 1.35) + 30;
        role.maxHp += 20;
        role.attack += 5;
        role.defense += 3;
        role.hp = role.maxHp;
        QMessageBox::information(this, "升级", QString("等级提升到 Lv.%1\n生命上限：%2\n攻击：%3\n防御：%4")
            .arg(role.level).arg(role.maxHp).arg(role.attack).arg(role.defense));
    }
}

void MainWindow::addItem(const ItemData& item) {
    if (inventory.size() >= 20) {
        QMessageBox::warning(this, "背包已满", "背包容量已满，无法获得物品。");
        return;
    }
    inventory.push_back(item);
    updateCollectTasks();
}

MainWindow::ItemData MainWindow::makeItemByName(const QString& name) const {
    for (const auto& item : goods) {
        if (item.name == name) return item;
    }
    return {};
}

QString MainWindow::itemText(const ItemData& item) const {
    QString equip = item.type == "装备" ? QString(" | %1").arg(item.equipped ? "已穿戴" : "未穿戴") : "";
    return QString("%1 | 类型：%2 | 价格：%3 | %4%5").arg(item.name).arg(item.type).arg(item.price).arg(item.desc).arg(equip);
}

QString MainWindow::taskStatusText(int status) const {
    if (status == 0) return "未接";
    if (status == 1) return "已接未完成";
    if (status == 2) return "已完成未领奖";
    return "已领奖";
}

void MainWindow::newRole() {
    bool ok = false;
    QString name = QInputDialog::getText(this, "新建角色", "请输入角色名称：", QLineEdit::Normal, role.name, &ok);
    if (!ok || name.trimmed().isEmpty()) return;
    role = RoleData{};
    role.name = name.trimmed();
    inventory.clear();
    setupData();
    appendLog("新建角色成功。");
    refreshAll();
    saveGame();
}

QString MainWindow::savePath() const {
    return QDir(QCoreApplication::applicationDirPath()).filePath("campus_rpg_qt_save.ini");
}

void MainWindow::saveGame() {
    QSettings s(savePath(), QSettings::IniFormat);
    s.setIniCodec("UTF-8");
    s.clear();
    s.setValue("role/name", role.name);
    s.setValue("role/level", role.level);
    s.setValue("role/hp", role.hp);
    s.setValue("role/maxHp", role.maxHp);
    s.setValue("role/exp", role.exp);
    s.setValue("role/expNeed", role.expNeed);
    s.setValue("role/gold", role.gold);
    s.setValue("role/attack", role.attack);
    s.setValue("role/defense", role.defense);
    s.setValue("inventory/count", inventory.size());
    for (int i = 0; i < inventory.size(); ++i) {
        QString p = QString("inventory/%1/").arg(i);
        const auto& item = inventory[i];
        s.setValue(p + "name", item.name);
        s.setValue(p + "type", item.type);
        s.setValue(p + "price", item.price);
        s.setValue(p + "desc", item.desc);
        s.setValue(p + "heal", item.heal);
        s.setValue(p + "attackBonus", item.attackBonus);
        s.setValue(p + "defenseBonus", item.defenseBonus);
        s.setValue(p + "equipped", item.equipped);
        s.setValue(p + "battleUsable", item.battleUsable);
    }
    for (int i = 0; i < tasks.size(); ++i) {
        QString p = QString("tasks/%1/").arg(i);
        s.setValue(p + "progress", tasks[i].progress);
        s.setValue(p + "status", tasks[i].status);
    }
    s.sync();
    QMessageBox::information(this, "保存成功", "当前进度已保存。");
}

void MainWindow::loadGame() {
    QFile f(savePath());
    if (!f.exists()) {
        refreshAll();
        return;
    }
    QSettings s(savePath(), QSettings::IniFormat);
    s.setIniCodec("UTF-8");
    role.name = s.value("role/name", role.name).toString();
    role.level = s.value("role/level", role.level).toInt();
    role.hp = s.value("role/hp", role.hp).toInt();
    role.maxHp = s.value("role/maxHp", role.maxHp).toInt();
    role.exp = s.value("role/exp", role.exp).toInt();
    role.expNeed = s.value("role/expNeed", role.expNeed).toInt();
    role.gold = s.value("role/gold", role.gold).toInt();
    role.attack = s.value("role/attack", role.attack).toInt();
    role.defense = s.value("role/defense", role.defense).toInt();
    inventory.clear();
    int count = s.value("inventory/count", 0).toInt();
    for (int i = 0; i < count; ++i) {
        QString p = QString("inventory/%1/").arg(i);
        inventory.push_back({
            s.value(p + "name").toString(), s.value(p + "type").toString(), s.value(p + "price").toInt(),
            s.value(p + "desc").toString(), s.value(p + "heal").toInt(), s.value(p + "attackBonus").toInt(),
            s.value(p + "defenseBonus").toInt(), s.value(p + "equipped").toBool(), s.value(p + "battleUsable").toBool()
        });
    }
    for (int i = 0; i < tasks.size(); ++i) {
        QString p = QString("tasks/%1/").arg(i);
        tasks[i].progress = s.value(p + "progress", tasks[i].progress).toInt();
        tasks[i].status = s.value(p + "status", tasks[i].status).toInt();
    }
    appendLog("读取存档成功。");
    refreshAll();
}

void MainWindow::buySelectedItem() {
    int row = shopList->currentRow();
    if (row < 0 || row >= goods.size()) return;
    const auto item = goods[row];
    if (role.gold < item.price) {
        QMessageBox::warning(this, "金币不足", "金币不足，购买失败。");
        return;
    }
    if (inventory.size() >= 20) {
        QMessageBox::warning(this, "背包已满", "背包容量已满，购买失败。");
        return;
    }
    role.gold -= item.price;
    addItem(item);
    appendLog(QString("购买成功：%1，剩余金币 %2。").arg(item.name).arg(role.gold));
    refreshAll();
}

void MainWindow::sellSelectedItem() {
    int row = inventoryList->currentRow();
    if (row < 0 || row >= inventory.size()) return;
    int price = inventory[row].price / 2;
    appendLog(QString("出售成功：%1，获得金币 %2。").arg(inventory[row].name).arg(price));
    role.gold += price;
    inventory.remove(row);
    updateCollectTasks();
    refreshAll();
}

void MainWindow::useSelectedItem() {
    int row = inventoryList->currentRow();
    if (row < 0 || row >= inventory.size()) return;
    auto& item = inventory[row];
    if (item.type == "食物" || item.type == "药品") {
        if (role.hp >= role.maxHp) {
            QMessageBox::information(this, "使用失败", "当前已满血，不能使用回复物品。");
            return;
        }
        int before = role.hp;
        role.hp = qMin(role.maxHp, role.hp + item.heal);
        appendLog(QString("使用%1，生命 %2 -> %3。").arg(item.name).arg(before).arg(role.hp));
        inventory.remove(row);
    } else if (item.type == "装备") {
        if (item.equipped) {
            role.attack -= item.attackBonus;
            role.defense -= item.defenseBonus;
            item.equipped = false;
            appendLog(QString("卸下装备：%1。").arg(item.name));
        } else {
            role.attack += item.attackBonus;
            role.defense += item.defenseBonus;
            item.equipped = true;
            appendLog(QString("穿戴装备：%1。").arg(item.name));
        }
    }
    updateCollectTasks();
    refreshAll();
}

void MainWindow::deleteSelectedItem() {
    int row = inventoryList->currentRow();
    if (row < 0 || row >= inventory.size()) return;
    appendLog(QString("删除物品：%1。").arg(inventory[row].name));
    inventory.remove(row);
    updateCollectTasks();
    refreshAll();
}

void MainWindow::acceptSelectedTask() {
    int row = taskList->currentRow();
    if (row < 0 || row >= tasks.size()) return;
    if (tasks[row].status != 0) {
        QMessageBox::information(this, "无法接受", "该任务不能重复接受。");
        return;
    }
    tasks[row].status = 1;
    updateCollectTasks();
    refreshAll();
}

void MainWindow::claimSelectedTask() {
    int row = taskList->currentRow();
    if (row < 0 || row >= tasks.size()) return;
    auto& task = tasks[row];
    if (task.status != 2) {
        QMessageBox::information(this, "无法领取", "任务尚未完成或奖励已领取。");
        return;
    }
    addExp(task.rewardExp);
    role.gold += task.rewardGold;
    if (!task.rewardItem.isEmpty()) addItem(makeItemByName(task.rewardItem));
    task.status = 3;
    appendLog(QString("领取任务奖励：%1。").arg(task.name));
    refreshAll();
}

void MainWindow::updateCollectTasks() {
    for (auto& task : tasks) {
        if (task.status != 1 || task.conditionType != "收集") continue;
        int count = 0;
        for (const auto& item : inventory) {
            if (item.name == task.target) count++;
        }
        task.progress = qMin(task.need, count);
        if (task.progress >= task.need) task.status = 2;
    }
}

void MainWindow::updateDefeatTasks(const QString& enemyName) {
    for (auto& task : tasks) {
        if (task.status != 1 || task.conditionType != "击败" || task.target != enemyName) continue;
        task.progress = qMin(task.need, task.progress + 1);
        if (task.progress >= task.need) task.status = 2;
    }
}

void MainWindow::fightNormalEnemy() {
    fightEnemy({"校园占座狗", "普通敌人", 80, 16, 4, 40, 30, "校园食堂盒饭", 45});
}

void MainWindow::fightEliteEnemy() {
    fightEnemy({"教务处查课老师", "精英敌人", 130, 24, 8, 90, 70, "校医院退烧药", 55});
}

void MainWindow::fightBossEnemy() {
    fightEnemy({"期末挂科神龙", "BOSS敌人", 220, 34, 12, 180, 160, "图书馆占座笔", 80});
}

void MainWindow::fightEnemy(const EnemyData& enemyInput) {
    EnemyData enemy = enemyInput;
    appendLog(QString("战斗开始：%1。").arg(enemy.name));
    int round = 1;
    while (role.hp > 0 && enemy.hp > 0) {
        int damage = qMax(1, role.attack - enemy.defense);
        enemy.hp = qMax(0, enemy.hp - damage);
        appendLog(QString("第%1回合：玩家攻击造成%2伤害，敌人HP=%3。").arg(round).arg(damage).arg(enemy.hp));
        if (enemy.hp <= 0) break;
        int enemyDamage = qMax(1, enemy.attack - role.defense);
        role.hp = qMax(0, role.hp - enemyDamage);
        appendLog(QString("%1反击造成%2伤害，玩家HP=%3/%4。").arg(enemy.name).arg(enemyDamage).arg(role.hp).arg(role.maxHp));
        round++;
    }
    if (role.hp > 0) {
        appendLog(QString("战斗胜利：获得%1经验、%2金币。").arg(enemy.exp).arg(enemy.gold));
        addExp(enemy.exp);
        role.gold += enemy.gold;
        int roll = int(QRandomGenerator::global()->bounded(100)) + 1;
        if (roll <= enemy.dropRate) {
            auto item = makeItemByName(enemy.drop);
            addItem(item);
            appendLog(QString("获得掉落：%1。").arg(item.name));
        }
        updateDefeatTasks(enemy.name);
    } else {
        QMessageBox::warning(this, "战斗失败", "玩家战败，已恢复一半生命并返回。");
        role.hp = qMax(1, role.maxHp / 2);
    }
    refreshAll();
    saveGame();
}
