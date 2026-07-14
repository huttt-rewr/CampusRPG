// MainWindow.cpp
// Qt 主窗口实现：校园题材轮回 Roguelike 回合制 RPG 的界面、存档、排课、商店、地窟与战斗逻辑。
#include "MainWindow.h"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QGraphicsOpacityEffect>
#include <QDir>
#include <QFile>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRandomGenerator>
#include <QResizeEvent>
#include <QSettings>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QStyle>
#include <QSplitter>
#include <QTableWidget>
#include <QTextEdit>
#include <QTime>
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

QString assetRoot() {
    const QString exeDir = QCoreApplication::applicationDirPath();
    // Try all possible asset locations, checking for a known file
    const QStringList roots = {
        exeDir + "/assets/",
        exeDir + "/../assets/",
        QDir::currentPath() + "/assets/",
        QDir::currentPath() + "/../assets/",
    };
    for (const QString& root : roots) {
        // Check with a file we know exists in assets/
        if (QFile::exists(root + "sprites/student.png") || QFile::exists(root + "scenes/classroom_dusk.jpg")) {
            return root;
        }
    }
    // Last resort: return the most likely path
    return exeDir + "/assets/";
}

QString existingAsset(const QStringList& candidates) {
    for (const QString& path : candidates) {
        if (QFile::exists(path)) return path;
    }
    return QString();
}

QString professionSpritePath(const QString& profession) {
    const QString base = assetRoot() + "sprites/";
    if (profession == QString::fromUtf8("学生")) return existingAsset({base + "student.png", base + QString::fromUtf8("学生.jpg")});
    if (profession == QString::fromUtf8("冰法师")) return existingAsset({base + "ice_mage.png", base + QString::fromUtf8("冰法师.jpg")});
    if (profession == QString::fromUtf8("圣骑士")) return existingAsset({base + "paladin.png", base + QString::fromUtf8("圣骑士.jpg")});
    if (profession == QString::fromUtf8("祈福者")) return existingAsset({base + "blesser.png", base + QString::fromUtf8("祈福者.jpg")});
    if (profession == QString::fromUtf8("血战士")) return existingAsset({base + "blood_warrior.png", base + QString::fromUtf8("血战士.jpg")});
    if (profession == QString::fromUtf8("魔术师")) return existingAsset({base + "magician.png", base + QString::fromUtf8("魔术师.jpg")});
    return QString();
}

QString enemySpritePath(const QString& name) {
    const QString base = assetRoot() + "sprites/";
    const auto has = [&name](const char* text) { return name.contains(QString::fromUtf8(text)); };
    if (has("体操幽灵")) return existingAsset({base + "gym_gymnast.png", base + QString::fromUtf8("体操幽灵.jpg")});
    if (has("短跑者")) return existingAsset({base + "gym_sprinter.png", base + QString::fromUtf8("懈怠的短跑者.jpg")});
    if (has("铅球手")) return existingAsset({base + "gym_shot_putter.png", base + QString::fromUtf8("驼背的铅球手.jpg")});
    if (has("涂鸦书灵")) return existingAsset({base + "library_scribble_spirit.png", base + QString::fromUtf8("涂鸦书灵.jpg")});
    if (has("目录魔像")) return existingAsset({base + "library_catalog_golem.png", base + QString::fromUtf8("目录魔像.jpg")});
    if (has("禁书管理员")) return existingAsset({base + "library_banned_librarian.png", base + QString::fromUtf8("禁书管理员.jpg")});
    if (has("小丑")) return existingAsset({base + "theater_clown.png", base + QString::fromUtf8("微笑小丑.jpg")});
    if (has("悲情女主角")) return existingAsset({base + "theater_tragedy.png", base + QString::fromUtf8("悲情女主角.jpg")});
    if (has("镜面侍从")) return existingAsset({base + "theater_mirror_attendant.png", base + QString::fromUtf8("镜面侍从.jpg")});
    if (has("酸液")) return existingAsset({base + "lab_acid_slime.png", base + QString::fromUtf8("酸液史莱姆.jpg")});
    if (has("烧杯")) return existingAsset({base + "lab_beaker.png", base + QString::fromUtf8("爆炸烧杯怪.jpg")});
    if (has("试管")) return existingAsset({base + "lab_tube.png", base + QString::fromUtf8("剧毒试管精.jpg")});
    if (has("塔罗") || has("命运轮盘")) return existingAsset({base + "divination_tarot.png", base + QString::fromUtf8("塔罗士兵.jpg")});
    if (has("时针")) return existingAsset({base + "divination_clock_ghost.png", base + QString::fromUtf8("时针幽灵.jpg")});
    if (has("命运轮盘")) return existingAsset({base + "divination_tarot.png", base + QString::fromUtf8("塔罗士兵.jpg")});
    if (has("铁腕")) return existingAsset({base + "office_iron_hand.png", base + QString::fromUtf8("教导处铁腕.jpg")});
    if (has("纪律巡查")) return existingAsset({base + "office_discipline_inspector.png", base + QString::fromUtf8("纪律巡查使.jpg")});
    if (has("扉页守护")) return existingAsset({base + "elite_book_guardian.png", base + QString::fromUtf8("扉页守护者.jpg")});
    if (has("百米王者")) return existingAsset({base + "elite_winged_runner.png", base + QString::fromUtf8("折翼的百米王者.jpg")});
    if (has("无面舞者")) return existingAsset({base + "elite_faceless_dancer.png", base + QString::fromUtf8("无面舞者.jpg")});
    if (has("时计塔")) return existingAsset({base + "elite_clock_prisoner.png", base + QString::fromUtf8("时计塔的囚徒.jpg")});
    if (has("畸变融合")) return existingAsset({base + "elite_mutant.png", base + QString::fromUtf8("畸变融合体.jpg")});
    if (has("铁锈执念") || has("陈暮")) return existingAsset({base + "elite_rust_regret.png", base + QString::fromUtf8("铁锈执念·陈暮.jpg")});
    if (has("伪典校长")) return existingAsset({base + "boss_principal.png", base + QString::fromUtf8("伪典校长·零.jpg")});
    return QString();
}

QString fallbackEnemySpritePath(int layer) {
    const QString base = assetRoot() + "sprites/";
    switch (std::max(1, layer)) {
    case 1: return existingAsset({base + "gym_sprinter.png"});
    case 2: return existingAsset({base + "library_scribble_spirit.png"});
    case 3: return existingAsset({base + "theater_clown.png"});
    case 4: return existingAsset({base + "lab_beaker.png"});
    case 5: return existingAsset({base + "divination_tarot.png"});
    case 6: return existingAsset({base + "office_iron_hand.png"});
    default: return existingAsset({base + "boss_principal.png"});
    }
}

QString scenePath(bool inDungeon, int layer, bool inDemonShop = false, bool inChestRoom = false) {
    const QString base = assetRoot() + "scenes/";
    if (inDungeon) {
        if (inDemonShop) return base + "demon_shop.jpg";
        if (inChestRoom) return base + "equipment_room.jpg";
        if (layer <= 1) return base + "dungeon_01_gym.jpg";
        if (layer == 2) return base + "dungeon_02_library.jpg";
        if (layer == 3) return base + "dungeon_03_theater.jpg";
        if (layer == 4) return base + "dungeon_04_lab.jpg";
        if (layer == 5) return base + "dungeon_05_divination.jpg";
        if (layer == 6) return base + "dungeon_06_office.jpg";
        return base + "dungeon_07_boss.png";
    }
    return base + "classroom_dusk.jpg";
}

QString phaseScenePath(int phaseValue) {
    const QString base = assetRoot() + "scenes/";
    switch (phaseValue) {
    case 1: return base + "classroom_dusk.jpg"; // First semester.
    case 2: return base + "playground.jpg";      // Winter break.
    case 3: return base + "classroom_dusk.jpg"; // Second semester.
    case 4: return base + "demon_shop.jpg";     // Angel shop fallback scene.
    default: return base + "classroom_dusk.jpg";
    }
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
    updateUiScale();
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
        {1, "角色命名", "为六名预设职业角色完成命名。", "创建角色", "任意", 1, 0, 0, 100, 0, ""},
        {2, "第一次选课", "执行第一学期课表，熟悉一周14个半天。", "选课", "任意", 1, 0, 0, 120, 0, ""},
        {3, "第一次过寒假", "完成寒假休息或寒假打工选择。", "寒假", "任意", 1, 0, 0, 150, 0, ""},
        {4, "天使商店购物", "了解金币清空规则、食品和药品区别，完成任意一次购物。", "天使购物", "任意", 1, 0, 0, 0, 20, ""},
        {5, "恶魔商店购物", "在地窟恶魔商店完成一次购物。", "恶魔购物", "任意", 1, 0, 0, 0, 0, "生锈的哑铃"},
        {6, "装备穿戴", "为任意角色穿戴一件装备。", "穿戴装备", "任意", 1, 0, 0, 0, 15, ""},
        {7, "编队", "完成上阵/下阵、阵型或站位调整。", "编队", "任意", 1, 0, 0, 0, 15, ""},
        {8, "人生第一个宝箱房", "开启地窟中的第一个宝箱房。", "宝箱", "任意", 1, 0, 0, 80, 10, ""},
        {9, "人生第一次战斗", "进入地窟中的第一次战斗。", "开战", "任意", 1, 0, 0, 80, 10, ""},
        {10, "人生第一次胜利", "赢下第一场地窟战斗，了解掉落规则。", "战斗胜利", "任意", 1, 0, 0, 100, 20, ""},
        {11, "通关第一层", "通过神秘地窟第一层。", "通层", "1", 1, 0, 0, 120, 15, ""},
        {12, "通关第二层", "通过神秘地窟第二层。", "通层", "2", 1, 0, 0, 150, 20, ""},
        {13, "通关第三层", "通过神秘地窟第三层。", "通层", "3", 1, 0, 0, 180, 25, ""},
        {14, "通关第四层", "通过神秘地窟第四层。", "通层", "4", 1, 0, 0, 210, 30, ""},
        {15, "通关第五层", "通过神秘地窟第五层。", "通层", "5", 1, 0, 0, 240, 35, ""},
        {16, "通关第六层", "通过神秘地窟第六层。", "通层", "6", 1, 0, 0, 270, 40, ""},
        {17, "逃离轮回", "击败第七层最终BOSS伪典校长·零。", "最终BOSS", "伪典校长·零", 1, 0, 0, 1000, 100, ""}
    };
    registerCodexEnemies();
}

void MainWindow::registerCodexEnemies() {
    codex.clear();
    const auto addGroup = [this](const QVector<EnemyData>& enemies) {
        for (const EnemyData& enemy : enemies) {
            codex[enemy.name] = enemy;
            if (!encountered.contains(enemy.name)) encountered[enemy.name] = false;
        }
    };
    for (int layer = 1; layer <= 6; ++layer) {
        addGroup(makeEnemyGroup(layer, false, false));
        addGroup(makeEnemyGroup(layer, true, false));
    }
    addGroup(makeEnemyGroup(7, false, true));
}

void MainWindow::setupSavePage() {
    savePage = new QWidget(this);
    savePage->setObjectName("saveRoot");
    auto* layout = new QVBoxLayout(savePage);
    saveTitleLabel = new QLabel("校园RPG冒险游戏\n选择存档开始轮回", savePage);
    saveTitleLabel->setAlignment(Qt::AlignCenter);
    saveHintLabel = new QLabel("总共 4 个存档位。空白存档可创建角色，已有存档可读取或删除。", savePage);
    saveHintLabel->setAlignment(Qt::AlignCenter);
    saveSceneLabel = new QLabel(savePage);
    saveSceneLabel->setAlignment(Qt::AlignCenter);
    saveSceneLabel->setMinimumHeight(210);
    saveSceneLabel->setStyleSheet("background:#111a2b;color:#f4ddb1;border:2px solid #b69755;border-radius:10px;");
    auto* portraits = new QHBoxLayout();
    for (const auto& prof : professions) {
        const QString name = QString::fromStdString(prof->name());
        const QPixmap portrait(professionSpritePath(name));
        auto* label = new QLabel(name, savePage);
        label->setAlignment(Qt::AlignCenter);
        label->setMinimumSize(120, 155);
        label->setStyleSheet("background:#1a2940;border:2px solid #8e774b;border-radius:10px;color:#f1dca8;font-weight:600;");
        if (!portrait.isNull()) {
            label->setPixmap(portrait.scaled(112, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            label->setToolTip(name);
        }
        portraits->addWidget(label);
    }
    slotGrid = new QGridLayout();
    layout->addWidget(saveTitleLabel);
    layout->addWidget(saveHintLabel);
    layout->addWidget(saveSceneLabel);
    layout->addLayout(portraits);
    layout->addLayout(slotGrid);
    layout->addStretch();
    stack->addWidget(savePage);
}

void MainWindow::setupGamePage() {
    gamePage = new QWidget(this);
    gamePage->setObjectName("gameRoot");
    auto* root = new QVBoxLayout(gamePage);
    root->setContentsMargins(14, 12, 14, 14);
    root->setSpacing(10);
    tabs = new QTabWidget(gamePage);
    taskHeaderButton = new QPushButton(gamePage);
    taskHeaderButton->setMinimumHeight(44);
    taskHeaderButton->setObjectName("taskHeader");
    root->addWidget(taskHeaderButton);
    root->addWidget(tabs, 1);

    auto decorateAction = [this](QPushButton* button, QStyle::StandardPixmap icon, bool accent = false) {
        button->setIcon(style()->standardIcon(icon));
        button->setIconSize(QSize(28, 28));
        button->setMinimumHeight(58);
        button->setProperty("accent", accent);
    };

    auto* overviewPage = new QWidget(gamePage);
    auto* overviewLayout = new QVBoxLayout(overviewPage);
    overviewLayout->setContentsMargins(18, 16, 18, 16);
    overviewLayout->setSpacing(12);
    overviewSceneLabel = new QLabel(overviewPage);
    overviewSceneLabel->setAlignment(Qt::AlignCenter);
    overviewSceneLabel->setMinimumHeight(230);
    overviewSceneLabel->setStyleSheet("background:#111a2b;color:#f4ddb1;border:2px solid #b69755;border-radius:10px;");
    overviewLabel = new QLabel(overviewPage);
    overviewLabel->setWordWrap(true);
    logText = new QTextEdit(overviewPage);
    logText->setReadOnly(true);
    auto* saveBtn = new QPushButton("保存当前存档", overviewPage);
    decorateAction(saveBtn, QStyle::SP_DialogSaveButton, true);
    overviewLayout->addWidget(overviewSceneLabel);
    overviewLayout->addWidget(overviewLabel);
    overviewLayout->addWidget(saveBtn);
    overviewLayout->addWidget(logText, 1);
    tabs->addTab(overviewPage, "轮回总览");
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::saveGame);
    connect(taskHeaderButton, &QPushButton::clicked, this, [this]() {
        for (int i = 0; i < tabs->count(); ++i) {
            if (tabs->tabText(i) == QString::fromUtf8("任务")) {
                tabs->setCurrentIndex(i);
                break;
            }
        }
    });

    auto* schedulePage = new QWidget(gamePage);
    auto* scheduleLayout = new QVBoxLayout(schedulePage);
    scheduleLayout->setContentsMargins(18, 16, 18, 16);
    scheduleLayout->setSpacing(10);
    schedulePreviewLabel = new QLabel(schedulePage);
    scheduleRoleLabel = new QLabel(schedulePage);
    scheduleTable = new QTableWidget(kDays, kHalfDays, schedulePage);
    scheduleTable->setHorizontalHeaderLabels({"上午", "下午"});
    scheduleTable->setVerticalHeaderLabels({"周一", "周二", "周三", "周四", "周五", "周六", "周日"});
    auto* runSemesterBtn = new QPushButton("执行本学期20周课表", schedulePage);
    auto* restBtn = new QPushButton("寒假休息（提升全体血量）", schedulePage);
    auto* workBtn = new QPushButton("寒假打工（获得额外金币）", schedulePage);
    decorateAction(runSemesterBtn, QStyle::SP_MediaPlay, true);
    decorateAction(restBtn, QStyle::SP_TitleBarShadeButton);
    decorateAction(workBtn, QStyle::SP_FileDialogDetailedView);
    scheduleLayout->addWidget(scheduleRoleLabel);
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
    auto* roleLayout = new QHBoxLayout(rolePage);
    roleLayout->setContentsMargins(18, 16, 18, 16);
    roleLayout->setSpacing(14);
    characterList = new QListWidget(rolePage);
    characterPreviewLabel = new QLabel(rolePage);
    characterPreviewLabel->setAlignment(Qt::AlignCenter);
    characterPreviewLabel->setMinimumWidth(300);
    characterPreviewLabel->setStyleSheet("background:#111a2b;color:#f4ddb1;border:2px solid #b69755;border-radius:10px;");
    auto* roleActions = new QVBoxLayout();
    auto* formation1 = new QPushButton("阵型：前排1 后排2", rolePage);
    auto* formation2 = new QPushButton("阵型：前排2 后排1", rolePage);
    auto* activeBtn = new QPushButton("选中角色 上阵/下阵", rolePage);
    auto* upBtn = new QPushButton("选中角色上移站位", rolePage);
    auto* downBtn = new QPushButton("选中角色下移站位", rolePage);
    auto* equipBtn = new QPushButton("给选中角色穿戴背包选中装备", rolePage);
    decorateAction(formation1, QStyle::SP_ArrowUp);
    decorateAction(formation2, QStyle::SP_ArrowDown);
    decorateAction(activeBtn, QStyle::SP_DialogApplyButton, true);
    decorateAction(upBtn, QStyle::SP_ArrowUp);
    decorateAction(downBtn, QStyle::SP_ArrowDown);
    decorateAction(equipBtn, QStyle::SP_DialogOpenButton);
    roleActions->addWidget(formation1);
    roleActions->addWidget(formation2);
    roleActions->addWidget(activeBtn);
    roleActions->addWidget(upBtn);
    roleActions->addWidget(downBtn);
    roleActions->addWidget(equipBtn);
    roleActions->addStretch();
    auto* characterPanel = new QVBoxLayout();
    characterPanel->addWidget(characterPreviewLabel, 1);
    auto* characterBagTitle = new QLabel("快捷背包：选中装备后可直接穿戴，食品/药品可直接使用", rolePage);
    characterInventoryList = new QListWidget(rolePage);
    auto* quickEquipBtn = new QPushButton("快速穿戴选中装备", rolePage);
    auto* quickUseBtn = new QPushButton("使用选中物品", rolePage);
    decorateAction(quickEquipBtn, QStyle::SP_DialogOpenButton, true);
    decorateAction(quickUseBtn, QStyle::SP_DialogApplyButton);
    characterPanel->addWidget(characterBagTitle);
    characterPanel->addWidget(characterInventoryList, 1);
    auto* quickBagActions = new QHBoxLayout();
    quickBagActions->addWidget(quickEquipBtn);
    quickBagActions->addWidget(quickUseBtn);
    characterPanel->addLayout(quickBagActions);
    roleLayout->addWidget(characterList, 2);
    roleLayout->addLayout(characterPanel, 3);
    roleLayout->addLayout(roleActions, 2);
    tabs->addTab(rolePage, "角色信息/战前编队");
    connect(formation1, &QPushButton::clicked, this, &MainWindow::setFormationOneFront);
    connect(formation2, &QPushButton::clicked, this, &MainWindow::setFormationTwoFront);
    connect(activeBtn, &QPushButton::clicked, this, &MainWindow::toggleSelectedRoleActive);
    connect(upBtn, &QPushButton::clicked, this, &MainWindow::moveRoleUp);
    connect(downBtn, &QPushButton::clicked, this, &MainWindow::moveRoleDown);
    connect(equipBtn, &QPushButton::clicked, this, &MainWindow::equipSelectedItem);
    connect(quickEquipBtn, &QPushButton::clicked, this, &MainWindow::equipSelectedItem);
    connect(quickUseBtn, &QPushButton::clicked, this, &MainWindow::useInventoryItem);
    connect(characterList, &QListWidget::currentRowChanged, this, [this](int) {
        updateVisualPreviews();
    });
    connect(characterInventoryList, &QListWidget::currentRowChanged, this, [this](int row) {
        selectedInventoryRow = row;
        if (inventoryList && inventoryList->currentRow() != row) inventoryList->setCurrentRow(row);
    });

    auto* bagPage = new QWidget(gamePage);
    auto* bagLayout = new QVBoxLayout(bagPage);
    bagLayout->setContentsMargins(18, 16, 18, 16);
    bagLayout->setSpacing(10);
    inventoryList = new QListWidget(bagPage);
    auto* useBtn = new QPushButton("使用选中物品", bagPage);
    auto* discardBtn = new QPushButton("丢弃选中物品", bagPage);
    decorateAction(useBtn, QStyle::SP_DialogApplyButton, true);
    decorateAction(discardBtn, QStyle::SP_TrashIcon);
    bagLayout->addWidget(inventoryList, 1);
    bagLayout->addWidget(useBtn);
    bagLayout->addWidget(discardBtn);
    tabs->addTab(bagPage, "背包");
    connect(useBtn, &QPushButton::clicked, this, &MainWindow::useInventoryItem);
    connect(discardBtn, &QPushButton::clicked, this, &MainWindow::discardInventoryItem);
    connect(inventoryList, &QListWidget::currentRowChanged, this, [this](int row) {
        selectedInventoryRow = row;
        if (characterInventoryList && characterInventoryList->currentRow() != row) characterInventoryList->setCurrentRow(row);
    });

    auto* taskPage = new QWidget(gamePage);
    auto* taskLayout = new QVBoxLayout(taskPage);
    taskLayout->setContentsMargins(18, 16, 18, 16);
    taskLayout->setSpacing(10);
    taskList = new QListWidget(taskPage);
    auto* acceptBtn = new QPushButton("接受选中任务", taskPage);
    auto* claimBtn = new QPushButton("领取选中奖励", taskPage);
    auto* taskUpBtn = new QPushButton("提高任务优先级", taskPage);
    auto* taskDownBtn = new QPushButton("降低任务优先级", taskPage);
    decorateAction(acceptBtn, QStyle::SP_DialogApplyButton, true);
    decorateAction(claimBtn, QStyle::SP_DialogSaveButton);
    taskLayout->addWidget(taskList, 1);
    taskLayout->addWidget(acceptBtn);
    taskLayout->addWidget(claimBtn);
    taskLayout->addWidget(taskUpBtn);
    taskLayout->addWidget(taskDownBtn);
    tabs->addTab(taskPage, "任务");
    connect(acceptBtn, &QPushButton::clicked, this, &MainWindow::acceptTask);
    connect(claimBtn, &QPushButton::clicked, this, &MainWindow::claimTask);
    connect(taskUpBtn, &QPushButton::clicked, this, &MainWindow::moveTaskUp);
    connect(taskDownBtn, &QPushButton::clicked, this, &MainWindow::moveTaskDown);

    auto* angelPage = new QWidget(gamePage);
    auto* angelLayout = new QVBoxLayout(angelPage);
    angelLayout->setContentsMargins(18, 16, 18, 16);
    angelLayout->setSpacing(10);
    auto* angelBalanceLabel = new QLabel(angelPage);
    angelBalanceLabel->setObjectName("angelBalance");
    angelShopList = new QListWidget(angelPage);
    auto* enterAngelBtn = new QPushButton("暑假进入天使商店", angelPage);
    auto* buyAngelBtn = new QPushButton("购买选中天使商品", angelPage);
    auto* enterDungeonBtn = new QPushButton("购买完成，进入7层神秘地窟（离开后金币清空）", angelPage);
    decorateAction(enterAngelBtn, QStyle::SP_DirHomeIcon, true);
    decorateAction(buyAngelBtn, QStyle::SP_DialogOpenButton);
    decorateAction(enterDungeonBtn, QStyle::SP_ArrowForward, true);
    angelLayout->addWidget(angelBalanceLabel);
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
    dungeonLayout->setContentsMargins(18, 16, 18, 16);
    dungeonLayout->setSpacing(10);
    dungeonLabel = new QLabel(dungeonPage);
    dungeonLabel->setWordWrap(true);
    dungeonSceneLabel = new QLabel(dungeonPage);
    dungeonSceneLabel->setAlignment(Qt::AlignCenter);
    dungeonSceneLabel->setMinimumHeight(220);
    dungeonSceneLabel->setStyleSheet("background:#111a2b;color:#f4ddb1;border:2px solid #b69755;border-radius:10px;");
    dungeonPreviewLabel = new QLabel(dungeonPage);
    dungeonPreviewLabel->setAlignment(Qt::AlignCenter);
    dungeonPreviewLabel->setMinimumHeight(220);
    dungeonPreviewLabel->setStyleSheet("background:#111a2b;color:#f4ddb1;border:2px solid #b69755;border-radius:10px;");
    dungeonRoomList = new QListWidget(dungeonPage);
    dungeonRoomList->setVisible(false);
    auto* mapHost = new QWidget(dungeonPage);
    dungeonMapGrid = new QGridLayout(mapHost);
    dungeonMapGrid->setSpacing(10);
    dungeonMapGrid->setContentsMargins(14, 14, 14, 14);
    demonShopList = new QListWidget(dungeonPage);
    auto* demonBalanceLabel = new QLabel(dungeonPage);
    demonBalanceLabel->setObjectName("demonBalance");
    auto* exploreBtn = new QPushButton("进入/探索选中房间", dungeonPage);
    auto* buyDemonBtn = new QPushButton("购买选中恶魔商品", dungeonPage);
    auto* sellDemonBtn = new QPushButton("出售背包装备给恶魔商店（半价）", dungeonPage);
    fightRoundBtn = new QPushButton("确认当前角色行动", dungeonPage);
    battleMedicineBtn = new QPushButton("战斗：使用背包选中药品", dungeonPage);
    battleTurnLabel = new QLabel(dungeonPage);
    battleActionCombo = new QComboBox(dungeonPage);
    battleTargetList = new QListWidget(dungeonPage);
    auto* nextLayerBtn = new QPushButton("通过精英房，进入下一层", dungeonPage);
    nextLayerBtn->setObjectName("nextLayerButton");
    decorateAction(exploreBtn, QStyle::SP_DirOpenIcon, true);
    decorateAction(buyDemonBtn, QStyle::SP_DialogOpenButton);
    decorateAction(sellDemonBtn, QStyle::SP_DialogSaveButton);
    decorateAction(fightRoundBtn, QStyle::SP_MediaPlay, true);
    decorateAction(battleMedicineBtn, QStyle::SP_DialogApplyButton);
    auto* dungeonVisuals = new QHBoxLayout();
    dungeonVisuals->addWidget(dungeonSceneLabel, 3);
    dungeonVisuals->addWidget(dungeonPreviewLabel, 2);

    auto* mapBox = new QGroupBox("地窟路线", dungeonPage);
    auto* mapLayout = new QVBoxLayout(mapBox);
    mapLayout->setContentsMargins(12, 16, 12, 12);
    mapLayout->addWidget(mapHost, 1);

    auto* tradeBox = new QGroupBox("恶魔商店", dungeonPage);
    tradeBox->setObjectName("demonShopBox");
    auto* tradeLayout = new QVBoxLayout(tradeBox);
    tradeLayout->setContentsMargins(12, 16, 12, 12);
    tradeLayout->setSpacing(8);
    tradeLayout->addWidget(demonBalanceLabel);
    tradeLayout->addWidget(demonShopList, 1);
    auto* tradeButtons = new QHBoxLayout();
    tradeButtons->addWidget(buyDemonBtn);
    tradeButtons->addWidget(sellDemonBtn);
    tradeLayout->addLayout(tradeButtons);

    auto* battleBox = new QGroupBox("战斗指令", dungeonPage);
    battleBox->setObjectName("battleBox");
    auto* battleLayout = new QVBoxLayout(battleBox);
    battleLayout->setContentsMargins(12, 16, 12, 12);
    battleLayout->setSpacing(8);
    battleLayout->addWidget(battleTurnLabel);
    battleLayout->addWidget(battleActionCombo);
    battleLayout->addWidget(battleTargetList, 1);
    auto* battleButtons = new QHBoxLayout();
    battleButtons->addWidget(fightRoundBtn);
    battleButtons->addWidget(battleMedicineBtn);
    battleLayout->addLayout(battleButtons);

    auto* lowerSplit = new QSplitter(Qt::Horizontal, dungeonPage);
    lowerSplit->setChildrenCollapsible(false);
    lowerSplit->addWidget(mapBox);
    lowerSplit->addWidget(tradeBox);
    lowerSplit->addWidget(battleBox);
    lowerSplit->setStretchFactor(0, 3);
    lowerSplit->setStretchFactor(1, 2);
    lowerSplit->setStretchFactor(2, 2);
    dungeonLayout->addWidget(dungeonLabel);
    dungeonLayout->addLayout(dungeonVisuals);
    dungeonLayout->addWidget(lowerSplit, 1);
    dungeonLayout->addWidget(dungeonRoomList);
    exploreBtn->setVisible(false);
    dungeonLayout->addWidget(exploreBtn);
    dungeonLayout->addWidget(nextLayerBtn);
    tabs->addTab(dungeonPage, "地窟/恶魔商店");
    connect(exploreBtn, &QPushButton::clicked, this, &MainWindow::exploreRoom);
    connect(buyDemonBtn, &QPushButton::clicked, this, &MainWindow::buyDemonItem);
    connect(sellDemonBtn, &QPushButton::clicked, this, &MainWindow::sellEquipmentToDemon);
    connect(fightRoundBtn, &QPushButton::clicked, this, &MainWindow::fightOneRound);
    connect(battleMedicineBtn, &QPushButton::clicked, this, &MainWindow::useBattleMedicine);
    connect(dungeonRoomList, &QListWidget::currentRowChanged, this, [this](int) { updateVisualPreviews(); });
    connect(nextLayerBtn, &QPushButton::clicked, this, &MainWindow::enterNextDungeonLayer);

    auto* codexPage = new QWidget(gamePage);
    auto* codexLayout = new QVBoxLayout(codexPage);
    codexList = new QListWidget(codexPage);
    codexLayout->addWidget(codexList, 1);
    tabs->addTab(codexPage, "轮回游记");

    stack->addWidget(gamePage);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    updateUiScale();
    updateVisualPreviews();
}

void MainWindow::updateUiScale() {
    int base = std::max(14, std::min(width() / 70, height() / 46));
    int buttonFont = std::max(16, base + 2);
    int titleFont = std::max(28, base * 2);
    int tabFont = std::max(15, base + 1);
    int listFont = std::max(14, base);
    int padY = std::max(10, base / 2 + 2);
    int padX = std::max(16, base + 2);
    int radius = std::max(8, base / 2);

    setStyleSheet(QString(
        "QMainWindow{background:#111a2b;color:#eadfbe;}"
        "QWidget{font-family:'Microsoft YaHei','SimHei',sans-serif;}"
        "QWidget#saveRoot,QWidget#gameRoot{border-image:url(\"%9/scenes/main_academy_background.png\") 0 0 0 0 stretch stretch;}"
        "QWidget#saveRoot QWidget,QWidget#gameRoot QWidget{background:transparent;}"
        "QWidget#gameRoot > QTabWidget{background:rgba(10,18,33,190);border-radius:%5px;border:1px solid #a98b52;}"
        "QLabel{font-size:%1px;line-height:1.35;color:#eadfbe;}"
        "QPushButton{font-size:%2px;font-weight:600;padding:%3px %4px;border:1px solid #a98b52;border-radius:%5px;background:#1d2b43;color:#f2e7c9;text-align:left;}"
        "QPushButton:hover{background:#2a3d5c;border-color:#d0b170;color:#fff5d9;}"
        "QPushButton:pressed{background:#142238;border-color:#e3c27d;}"
        "QPushButton[accent=\"true\"]{background:#6f293b;color:#fff2d2;border-color:#c29b56;}"
        "QPushButton[accent=\"true\"]:hover{background:#853247;border-color:#e0bd70;}"
        "QPushButton:disabled{color:#778093;background:#182335;border-color:#36465c;}"
        "QPushButton#taskHeader{background:#17233a;border:1px solid #a98b52;color:#f1dca8;font-weight:700;text-align:left;padding-left:%4px;}"
        "QTabWidget::pane{border:1px solid #8e774b;background:rgba(17,28,47,205);border-radius:%5px;}"
        "QTabBar::tab{font-size:%6px;font-weight:600;padding:%3px %4px;background:#202f48;border:1px solid #61708a;color:#cfc29d;border-top-left-radius:%5px;border-top-right-radius:%5px;}"
        "QTabBar::tab:hover{background:#314562;color:#fff0c7;}"
        "QTabBar::tab:selected{background:#6b293b;color:#ffe9b2;border-bottom:3px solid #d4ab5c;}"
        "QGroupBox{font-size:%7px;font-weight:700;color:#edcf91;border:1px solid #8e774b;border-radius:%5px;margin-top:10px;background:#17243a;}"
        "QGroupBox::title{subcontrol-origin:margin;left:12px;padding:0 6px;}"
        "QListWidget,QTextEdit,QTableWidget,QComboBox{font-size:%7px;background:#202a3a;border:1px solid #756a51;border-radius:%5px;padding:%8px;color:#ede4ce;selection-background-color:#653247;selection-color:#fff4d7;}"
        "QListWidget::item{padding:%8px;border-bottom:1px solid #3c4657;}"
        "QListWidget::item:hover{background:#293850;}"
        "QListWidget::item:selected{background:#653247;border-left:4px solid #d4ab5c;}"
        "QHeaderView::section{font-size:%7px;font-weight:600;padding:%8px;background:#293850;border:1px solid #756a51;color:#f2dfb3;}"
    ).arg(base).arg(buttonFont).arg(padY).arg(padX).arg(radius).arg(tabFont).arg(listFont).arg(std::max(5, base / 3)).arg(assetRoot().chopped(1)));

    if (saveTitleLabel) {
        saveTitleLabel->setStyleSheet(QString("font-size:%1px;font-weight:800;padding:%2px;color:#f4ddb1;").arg(titleFont).arg(std::max(16, base)));
    }
    if (slotGrid) {
        for (int i = 0; i < slotGrid->count(); ++i) {
            auto* btn = qobject_cast<QPushButton*>(slotGrid->itemAt(i)->widget());
            if (!btn) continue;
            btn->setMinimumHeight(std::max(110, height() / 7));
            btn->setStyleSheet(QString("font-size:%1px;text-align:center;").arg(std::max(16, base + 4)));
        }
    }
    if (scheduleTable) {
        scheduleTable->verticalHeader()->setDefaultSectionSize(std::max(42, base * 3));
        scheduleTable->horizontalHeader()->setDefaultSectionSize(std::max(120, width() / 5));
    }
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

void MainWindow::loadScheduleForCurrentRole() {
    if (!scheduleTable || schedulingRoleIndex < 0 || schedulingRoleIndex >= party.size()) return;
    const QString key = party[schedulingRoleIndex].profession;
    const QVector<QString> preset = keepSchedulePreset.value(key) ? savedSchedules.value(key) : QVector<QString>();
    for (int day = 0; day < kDays; ++day) {
        for (int half = 0; half < kHalfDays; ++half) {
            auto* combo = qobject_cast<QComboBox*>(scheduleTable->cellWidget(day, half));
            if (!combo) continue;
            const int index = day * kHalfDays + half;
            combo->setCurrentText(index < preset.size() ? preset[index] : QString::fromUtf8("不上课"));
        }
    }
    refreshSchedulePreview();
}

void MainWindow::connectActions() {
    refreshSaveSlots();
}

void MainWindow::showSavePage() {
    phase = GamePhase::SaveSelect;
    stack->setCurrentWidget(savePage);
    refreshSaveSlots();
    if (saveSceneLabel) {
        const QPixmap scene(assetRoot() + "scenes/classroom_dusk.jpg");
        if (scene.isNull()) {
            saveSceneLabel->setPixmap(QPixmap());
            saveSceneLabel->setText(QString::fromUtf8("校园场景图未找到"));
        } else {
            saveSceneLabel->setText(QString());
            saveSceneLabel->setPixmap(scene.scaled(saveSceneLabel->size(), Qt::KeepAspectRatioByExpanding,
                Qt::SmoothTransformation));
        }
    }
}

void MainWindow::showGamePage() {
    stack->setCurrentWidget(gamePage);
    refreshAll();
}

void MainWindow::updatePhaseUi() {
    if (!tabs) return;
    const bool school = phase == GamePhase::SchoolFirst || phase == GamePhase::SchoolSecond;
    const bool winter = phase == GamePhase::Winter;
    const bool angel = phase == GamePhase::AngelShop;
    const bool dungeon = phase == GamePhase::Dungeon;
    const bool ending = phase == GamePhase::Ending;

    for (int i = 0; i < tabs->count(); ++i) {
        const QString label = tabs->tabText(i);
        bool visible = label == "轮回总览" || label == "角色编队" || label == "背包" || label == "任务" || label == "轮回游记";
        if (label == "排课") visible = school || winter;
        if (label == "天使商店") visible = angel;
        if (label == "地窟/恶魔商店") visible = dungeon;
        tabs->setTabVisible(i, visible);
    }

    if (school || winter) {
        for (int i = 0; i < tabs->count(); ++i) if (tabs->tabText(i) == "排课") tabs->setCurrentIndex(i);
    }
    if (angel) {
        for (int i = 0; i < tabs->count(); ++i) if (tabs->tabText(i) == "天使商店") tabs->setCurrentIndex(i);
    }
    if (dungeon) {
        for (int i = 0; i < tabs->count(); ++i) if (tabs->tabText(i) == "地窟/恶魔商店") tabs->setCurrentIndex(i);
    }
    if (ending) {
        for (int i = 0; i < tabs->count(); ++i) if (tabs->tabText(i) == "轮回总览") tabs->setCurrentIndex(i);
    }
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
    updateVisualPreviews();
    updatePhaseUi();
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
        btn->setProperty("slot", i);
        connect(btn, &QPushButton::clicked, this, &MainWindow::slotClicked);
        slotGrid->addWidget(btn, (i - 1) / 2, (i - 1) % 2);
    }
    updateUiScale();
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
        .arg(demonDiscount).arg(activeRoleCount()).arg(formationType == 1 ? "前排1 后排2" : "前排2 后排1")
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
    if (scheduleRoleLabel) {
        QString roleName = schedulingRoleIndex >= 0 && schedulingRoleIndex < party.size() ? party[schedulingRoleIndex].name : "未选择角色";
        scheduleRoleLabel->setText(QString("当前为：%1 排课（%2/%3）").arg(roleName).arg(schedulingRoleIndex + 1).arg(party.size()));
    }
    schedulePreviewLabel->setText(QString("本周预计盈利金币：%1 | 本周剩余活力：%2 | 前五天可安排所有行动，周末只能不上课/打工/补习班。%3")
        .arg(profit).arg(vigor).arg(details.isEmpty() ? "" : "\n" + details.join("\n")));
}

void MainWindow::refreshCharacters() {
    if (!characterList) return;
    const int selectedRow = characterList->currentRow();
    characterList->clear();
    characterList->setIconSize(QSize(92, 92));
    int activeIndex = 0;
    for (int i = 0; i < party.size(); ++i) {
        QString position = activeText(party[i], activeIndex);
        if (party[i].active) activeIndex++;
        const QString battleMarker = inBattle && i == currentBattleRole() ? "  <当前行动>" : "";
        auto* item = new QListWidgetItem(QString("%1 [%2]%3\n%4\n技能：\n%5")
            .arg(roleText(party[i])).arg(position).arg(battleMarker)
            .arg(party[i].equipment.isEmpty() ? "装备：无" : "装备：" + QStringList(party[i].equipment.values()).join("、"))
            .arg(skillsText(party[i])));
        const QPixmap sprite(professionSpritePath(party[i].profession));
        if (!sprite.isNull()) item->setIcon(QIcon(sprite.scaled(92, 92, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        characterList->addItem(item);
    }
    if (selectedRow >= 0 && selectedRow < characterList->count()) characterList->setCurrentRow(selectedRow);
    else if (!party.isEmpty()) characterList->setCurrentRow(0);
}

void MainWindow::refreshInventory() {
    if (!inventoryList) return;
    const int currentRow = selectedInventoryRow;
    inventoryList->clear();
    if (characterInventoryList) characterInventoryList->clear();
    for (int i = 0; i < inventory.size(); ++i) {
        const QString text = QString("%1. %2").arg(i + 1).arg(inventoryText(inventory[i]));
        inventoryList->addItem(text);
        if (characterInventoryList) characterInventoryList->addItem(text);
    }
    const int row = currentRow >= 0 && currentRow < inventory.size() ? currentRow : (inventory.isEmpty() ? -1 : 0);
    selectedInventoryRow = row;
    QSignalBlocker inventoryBlocker(inventoryList);
    inventoryList->setCurrentRow(row);
    if (characterInventoryList) {
        QSignalBlocker characterInventoryBlocker(characterInventoryList);
        characterInventoryList->setCurrentRow(row);
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
    if (taskHeaderButton) {
        const TaskData* top = nullptr;
        for (const auto& task : tasks) {
            if (task.status == 1 || task.status == 0) { top = &task; break; }
        }
        const QString text = top
            ? QString("当前任务：%1  [%2]  %3/%4    点击展开任务列表")
                .arg(top->name).arg(taskStatusText(top->status)).arg(top->progress).arg(top->need)
            : QString("任务：所有任务已完成    点击展开任务列表");
        taskHeaderButton->setText(text);
    }
}

void MainWindow::refreshAngelShop() {
    if (!angelShopList) return;
    angelShopList->clear();
    if (auto* balance = gamePage->findChild<QLabel*>("angelBalance")) {
        balance->setText(QString("当前资产：金币 %1    背包 %2/%3").arg(gold).arg(inventory.size()).arg(kInventoryLimit));
    }
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
    if (auto* balance = gamePage->findChild<QLabel*>("demonBalance")) {
        balance->setText(QString("当前资产：恶魔币 %1    背包 %2/%3").arg(demonCoin).arg(inventory.size()).arg(kInventoryLimit));
    }
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
    dungeonRoomList->setIconSize(QSize(82, 82));
    dungeonLabel->setText(QString("地窟层数：%1/7 | 当前房间：%2 | 战斗状态：%3 | 恶魔币：%4\n%5")
        .arg(dungeonLayer).arg(currentRoom + 1).arg(inBattle ? "战斗中" : "非战斗").arg(demonCoin)
        .arg(battleStatusText()));
    for (int i = 0; i < rooms.size(); ++i) {
        const bool revealed = rooms[i].visited || rooms.value(currentRoom).connections.contains(i) || i == currentRoom;
        QString type;
        if (!revealed) type = "未知房间";
        else if (rooms[i].type == RoomType::DemonShop) type = "恶魔商店";
        else if (rooms[i].type == RoomType::Battle) type = "普通战斗";
        else if (rooms[i].type == RoomType::EliteBattle) type = "精英战斗+下一层通道";
        else if (rooms[i].type == RoomType::Chest) type = "宝箱房";
        else if (rooms[i].type == RoomType::Boss) type = "最终BOSS";
        auto* item = new QListWidgetItem(QString("房间%1：%2 | %3").arg(i + 1).arg(type).arg(rooms[i].cleared ? "已完成" : "未完成"));
        if (revealed && !rooms[i].enemies.isEmpty()) {
            const QPixmap sprite(enemySpritePath(rooms[i].enemies.first().name));
            if (!sprite.isNull()) item->setIcon(QIcon(sprite.scaled(82, 82, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        }
        dungeonRoomList->addItem(item);
    }
    rebuildDungeonMap();
    fightRoundBtn->setEnabled(inBattle);
    battleMedicineBtn->setEnabled(inBattle);
    const bool atDemonShop = !rooms.isEmpty() && currentRoom >= 0 && currentRoom < rooms.size()
        && rooms[currentRoom].type == RoomType::DemonShop && !inBattle;
    if (auto* tradeBox = gamePage->findChild<QGroupBox*>("demonShopBox")) tradeBox->setVisible(atDemonShop);
    if (auto* battleBox = gamePage->findChild<QGroupBox*>("battleBox")) battleBox->setVisible(inBattle);
    if (auto* next = gamePage->findChild<QPushButton*>("nextLayerButton")) next->setVisible(nextLayerReady);
    if (battleActionCombo && battleTargetList && battleTurnLabel) {
        battleActionCombo->setVisible(inBattle);
        battleTargetList->setVisible(inBattle);
        battleTurnLabel->setVisible(inBattle);
        battleActionCombo->clear();
        battleTargetList->clear();
        if (inBattle) {
            int roleIndex = currentBattleRole();
            if (roleIndex >= 0) {
                const CharacterData& role = party[roleIndex];
                battleTurnLabel->setText(QString("第%1回合：当前行动 %2（%3/%4）\n行动后将自动切换至下一站位角色。")
                    .arg(battleRound).arg(role.name).arg(battleActorIndex + 1).arg(battleOrder.size()));
                battleActionCombo->addItem("普通攻击");
                if (auto* prof = professionByName(role.profession)) {
                    for (const auto& skill : prof->skills()) {
                        if (role.level >= skill.unlockLevel) battleActionCombo->addItem(QString::fromStdString(skill.name));
                    }
                }
            }
            for (int i = 0; i < battleEnemies.size(); ++i) {
                if (battleEnemies[i].hp > 0) {
                    auto* target = new QListWidgetItem(QString("%1  HP %2/%3").arg(battleEnemies[i].name).arg(battleEnemies[i].hp).arg(battleEnemies[i].maxHp));
                    target->setData(Qt::UserRole, i);
                    battleTargetList->addItem(target);
                }
            }
            if (battleTargetList->count() > 0) battleTargetList->setCurrentRow(0);
        }
    }
}

void MainWindow::refreshCodex() {
    if (!codexList) return;
    codexList->clear();
    codexList->setViewMode(QListView::IconMode);
    codexList->setResizeMode(QListView::Adjust);
    codexList->setMovement(QListView::Static);
    codexList->setWrapping(true);
    codexList->setSpacing(12);
    codexList->setIconSize(QSize(136, 136));
    codexList->setGridSize(QSize(245, 242));
    QStringList names = codex.keys();
    names.sort();
    for (const QString& name : names) {
        if (!encountered.value(name, false)) {
            auto* item = new QListWidgetItem(QIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion)),
                QString("未知单位\n？？？\n击败或遭遇后解锁"));
            item->setTextAlignment(Qt::AlignCenter);
            item->setToolTip("尚未遭遇该单位");
            codexList->addItem(item);
            continue;
        }
        const auto& e = codex[name];
        QString path = enemySpritePath(e.name);
        if (path.isEmpty()) path = fallbackEnemySpritePath(e.layer);
        const QPixmap sprite(path);
        auto* item = new QListWidgetItem(sprite.isNull() ? QIcon() : QIcon(sprite.scaled(136, 136, Qt::KeepAspectRatio, Qt::SmoothTransformation)),
            QString("%1\n第%2层 %3\nHP %4  攻 %5  防 %6\n%7")
                .arg(e.name).arg(e.layer).arg(e.kind).arg(e.maxHp).arg(e.attack).arg(e.defense).arg(e.skills));
        item->setTextAlignment(Qt::AlignCenter);
        item->setToolTip(QString("法抗 %1\n技能：%2").arg(e.resist).arg(e.skills));
        codexList->addItem(item);
    }
}

void MainWindow::appendLog(const QString& text) {
    if (!logText) return;
    logText->append(QString("[%1] %2").arg(QTime::currentTime().toString("HH:mm:ss"), text));
}

void MainWindow::updateVisualPreviews() {
    auto animatePreview = [](QLabel* label) {
        auto* effect = qobject_cast<QGraphicsOpacityEffect*>(label->graphicsEffect());
        if (!effect) {
            effect = new QGraphicsOpacityEffect(label);
            label->setGraphicsEffect(effect);
        }
        effect->setOpacity(0.25);
        auto* animation = new QPropertyAnimation(effect, "opacity", label);
        animation->setDuration(220);
        animation->setStartValue(0.25);
        animation->setEndValue(1.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    };
    const bool inDemonShop = phase == GamePhase::Dungeon && currentRoom >= 0
        && currentRoom < rooms.size() && rooms[currentRoom].type == RoomType::DemonShop;
    const bool inChestRoom = phase == GamePhase::Dungeon && currentRoom >= 0
        && currentRoom < rooms.size() && rooms[currentRoom].type == RoomType::Chest;
    const QPixmap scene(phase == GamePhase::Dungeon
        ? scenePath(true, dungeonLayer, inDemonShop, inChestRoom)
        : phaseScenePath(static_cast<int>(phase)));
    if (overviewSceneLabel) {
        if (scene.isNull()) {
            overviewSceneLabel->setPixmap(QPixmap());
            overviewSceneLabel->setText(QString::fromUtf8("当前场景资源未找到"));
        } else {
            overviewSceneLabel->setText(QString());
            overviewSceneLabel->setPixmap(scene.scaled(overviewSceneLabel->size(), Qt::KeepAspectRatioByExpanding,
                Qt::SmoothTransformation));
            animatePreview(overviewSceneLabel);
        }
    }

    if (dungeonSceneLabel) {
        const QPixmap dungeonScene(scenePath(true, dungeonLayer));
        if (dungeonScene.isNull()) {
            dungeonSceneLabel->setPixmap(QPixmap());
            dungeonSceneLabel->setText(QString::fromUtf8("当前层场景图暂不可用"));
        } else {
            dungeonSceneLabel->setText(QString());
            dungeonSceneLabel->setPixmap(dungeonScene.scaled(dungeonSceneLabel->size(), Qt::KeepAspectRatioByExpanding,
                Qt::SmoothTransformation));
            animatePreview(dungeonSceneLabel);
        }
    }

    if (characterPreviewLabel) {
        const int row = characterList ? characterList->currentRow() : -1;
        const QString path = row >= 0 && row < party.size() ? professionSpritePath(party[row].profession) : QString();
        const QPixmap sprite(path);
        if (sprite.isNull()) {
            characterPreviewLabel->setPixmap(QPixmap());
            characterPreviewLabel->setText(QString::fromUtf8("选择角色查看立绘"));
        } else {
            characterPreviewLabel->setText(QString());
            characterPreviewLabel->setPixmap(sprite.scaled(characterPreviewLabel->size(), Qt::KeepAspectRatio,
                Qt::SmoothTransformation));
            animatePreview(characterPreviewLabel);
        }
    }

    if (dungeonPreviewLabel) {
        QString name;
        if (inBattle && !battleEnemies.isEmpty()) {
            name = battleEnemies.first().name;
        } else {
            const int row = dungeonRoomList ? dungeonRoomList->currentRow() : -1;
            if (row >= 0 && row < rooms.size() && !rooms[row].enemies.isEmpty()) name = rooms[row].enemies.first().name;
        }
        QString path = enemySpritePath(name);
        if (path.isEmpty()) path = fallbackEnemySpritePath(dungeonLayer);
        const QPixmap sprite(path);
        if (sprite.isNull()) {
            dungeonPreviewLabel->setPixmap(QPixmap());
            dungeonPreviewLabel->setText(QString::fromUtf8("当前层敌方模型暂不可用"));
        } else {
            dungeonPreviewLabel->setText(QString());
            dungeonPreviewLabel->setPixmap(sprite.scaled(dungeonPreviewLabel->size(), Qt::KeepAspectRatio,
                Qt::SmoothTransformation));
            animatePreview(dungeonPreviewLabel);
        }
    }
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
    currentSlot = selectedSlot;
    resetGameForNewRun(false);
    for (const auto& prof : professions) {
        bool ok = false;
        QString profName = QString::fromStdString(prof->name());
        QString defaultName = QString("%1同学").arg(profName);
        QString roleName = QInputDialog::getText(this, "角色命名",
            QString("请输入%1的自定义角色名：").arg(profName),
            QLineEdit::Normal, defaultName, &ok).trimmed();
        if (!ok || roleName.isEmpty()) return;
        createCharacter(profName, roleName);
    }
    for (int i = 0; i < party.size(); ++i) party[i].active = i < 3;
    phase = GamePhase::SchoolFirst;
    schedulingRoleIndex = 0;
    loadScheduleForCurrentRole();
    addTaskProgress("创建角色", "任意", 1);
    writeGame(currentSlot);
    showGamePage();
    appendLog("六名角色创建完成，默认前三名上阵，进入第一学期排课。");
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
    if (!keepGrowth) encountered.clear();
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
    schedulingRoleIndex = 0;
    actedPlayers.clear();
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
    encountered.clear();
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
    savedSchedules.clear();
    keepSchedulePreset.clear();
    s.beginGroup("schedules");
    for (const auto& prof : professions) {
        const QString key = QString::fromStdString(prof->name());
        keepSchedulePreset[key] = s.value(key + "/keep", false).toBool();
        savedSchedules[key] = QVector<QString>::fromList(s.value(key + "/actions").toStringList());
    }
    s.endGroup();
    schedulingRoleIndex = 0;
    loadScheduleForCurrentRole();
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
    s.remove("schedules");
    s.beginGroup("schedules");
    for (auto it = keepSchedulePreset.cbegin(); it != keepSchedulePreset.cend(); ++it) {
        s.setValue(it.key() + "/keep", it.value());
        QStringList actions;
        for (const QString& action : savedSchedules.value(it.key())) actions << action;
        s.setValue(it.key() + "/actions", actions);
    }
    s.endGroup();
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
    if (schedulingRoleIndex < party.size()) {
        QVector<QString> actions;
        for (int day = 0; day < kDays; ++day) {
            for (int half = 0; half < kHalfDays; ++half) {
                auto* combo = qobject_cast<QComboBox*>(scheduleTable->cellWidget(day, half));
                actions.push_back(combo ? combo->currentText() : QString::fromUtf8("不上课"));
            }
        }
        const QString key = party[schedulingRoleIndex].profession;
        const auto keep = QMessageBox::question(this, "保留课表", QString("是否保留 %1 的课表预设？下次将自动填充。").arg(party[schedulingRoleIndex].name),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        keepSchedulePreset[key] = keep == QMessageBox::Yes;
        if (keepSchedulePreset[key]) savedSchedules[key] = actions;
        else savedSchedules.remove(key);

        int vigor = 100 + initialVigorBonus;
        int profit = 0;
        CharacterData& role = party[schedulingRoleIndex];
        for (const QString& action : actions) {
            vigor -= actionVigorCost(action);
            profit += actionGoldChange(action, workGoldBonus);
            if (action == QString::fromUtf8("大物课")) { role.physicalAttack += 1; role.physicalDefense += 1; }
            else if (action == QString::fromUtf8("语文课")) angelDiscount = std::min(50, angelDiscount + 1);
            else if (action == QString::fromUtf8("外语课")) demonDiscount = std::min(50, demonDiscount + 1);
            else if (action == QString::fromUtf8("化学课")) role.magicResistance += 2;
            else if (action == QString::fromUtf8("高数课")) role.physicalDefense += 2;
            else if (action == QString::fromUtf8("专业课")) { role.physicalAttack += 2; role.magicAttack += 2; }
            else if (action == QString::fromUtf8("体育课")) { role.maxHp += 4; role.hp = role.maxHp; }
        }
        gold += profit * 20;
        addExp(role, 1200);
        appendLog(QString("%1 的课表已确认：学期收益 %2 金币。").arg(role.name).arg(profit * 20));
        schedulingRoleIndex++;
        if (schedulingRoleIndex < party.size()) {
            loadScheduleForCurrentRole();
            refreshAll();
            return;
        }
        schedulingRoleIndex = 0;
    }

    addTaskProgress("选课", "任意", 1);
    appendLog(QString("第%1学期：所有角色课表已确认并结算完成。").arg(semester));
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
    addTaskProgress("寒假", "任意", 1);
    semester = 2;
    phase = GamePhase::SchoolSecond;
    schedulingRoleIndex = 0;
    loadScheduleForCurrentRole();
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
    addTaskProgress("寒假", "任意", 1);
    semester = 2;
    phase = GamePhase::SchoolSecond;
    schedulingRoleIndex = 0;
    loadScheduleForCurrentRole();
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
    if (item.type != ItemType::Growth && inventory.size() >= kInventoryLimit) {
        QMessageBox::warning(this, "背包已满", "背包已满，无法购买会占用背包格的物品。");
        return;
    }
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
    if (currentRoom < 0 || currentRoom >= rooms.size() || rooms[currentRoom].type != RoomType::DemonShop) {
        QMessageBox::warning(this, "不在商店", "请先移动到当前层入口的恶魔商店房间后再交易。");
        return;
    }
    int row = demonShopList->currentRow();
    if (row < 0 || row >= demonGoods.size()) return;
    ItemData item = demonGoods[row];
    int price = std::max(item.demonPrice * (100 - std::min(50, demonDiscount)) / 100, item.demonPrice / 2);
    if (item.type != ItemType::Growth && inventory.size() >= kInventoryLimit) {
        QMessageBox::warning(this, "背包已满", "背包已满，无法购买会占用背包格的物品。");
        return;
    }
    if (demonCoin < price) {
        QMessageBox::warning(this, "恶魔币不足", "恶魔币不足，购买失败。");
        return;
    }
    demonCoin -= price;
    for (auto& role : party) role.vigor = std::max(0, role.vigor - 5);
    if (item.name == "恶魔之友") demonFriendBonus += 8;
    else addItem(item);
    firstDemonShopBought = true;
    addTaskProgress("恶魔购物", "任意", 1);
    appendLog(QString("恶魔商店购买：%1，剩余恶魔币%2。").arg(item.name).arg(demonCoin));
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::sellEquipmentToDemon() {
    if (phase != GamePhase::Dungeon || dungeonLayer <= 0) {
        QMessageBox::warning(this, "无法出售", "只能在地窟恶魔商店出售装备。");
        return;
    }
    if (currentRoom < 0 || currentRoom >= rooms.size() || rooms[currentRoom].type != RoomType::DemonShop) {
        QMessageBox::warning(this, "不在商店", "请先移动到当前层入口的恶魔商店房间后再交易。");
        return;
    }
    int row = inventoryList->currentRow();
    if (row < 0 || row >= inventory.size()) {
        QMessageBox::information(this, "请选择装备", "请先在背包列表选中要出售的装备。");
        return;
    }
    if (inventory[row].type != ItemType::Equipment) {
        QMessageBox::warning(this, "无法出售", "恶魔商店只收购装备。");
        return;
    }
    int value = std::max(1, inventory[row].demonPrice / 2);
    QString name = inventory[row].name;
    inventory.removeAt(row);
    demonCoin += value;
    for (auto& role : party) role.vigor = std::max(0, role.vigor - 5);
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
    int row = selectedInventoryRow;
    if (row < 0 || row >= inventory.size() || party.isEmpty()) {
        QMessageBox::information(this, "请选择物品", "请先在背包列表选中要使用的物品。");
        return;
    }
    ItemData item = inventory[row];
    if (item.type == ItemType::Equipment || item.type == ItemType::Growth) {
        QMessageBox::information(this, "提示", "装备请在角色编队页面穿戴，养成物品购买时立即生效。");
        return;
    }
    int roleIndex = selectedRoleOrFirstAlive();
    if (roleIndex < 0) {
        QMessageBox::warning(this, "无人可用", "没有可使用物品的角色。");
        return;
    }
    CharacterData& role = party[roleIndex];
    if (item.hpRecover > 0 && role.hp >= role.maxHp && item.mpRecover == 0 && item.staminaRecover == 0) {
        QMessageBox::information(this, "无需使用", QString("%1 当前生命值已满。").arg(role.name));
        return;
    }
    applyItemEffect(role, item);
    inventory.removeAt(row);
    appendLog(QString("%1 非战斗使用 %2。").arg(role.name).arg(item.name));
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::discardInventoryItem() {
    int row = selectedInventoryRow;
    if (row < 0 || row >= inventory.size()) {
        QMessageBox::information(this, "请选择物品", "请先在背包列表选中要丢弃的物品。");
        return;
    }
    QString name = inventory[row].name;
    inventory.removeAt(row);
    appendLog(QString("丢弃物品：%1。").arg(name));
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::equipSelectedItem() {
    if (inBattle) {
        QMessageBox::information(this, "战斗中不可换装", "请在进入战斗房前于角色信息页面完成装备调整。");
        return;
    }
    int roleRow = characterList->currentRow();
    int itemRow = selectedInventoryRow;
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
    inventory.removeAt(itemRow);
    if (!old.isEmpty()) {
        ItemData oldItem = itemByName(old);
        applyEquipmentStats(role, oldItem, -1);
        inventory.push_back(oldItem);
    }
    role.equipment[item.equipSlot] = item.name;
    applyEquipmentStats(role, item, 1);
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

void MainWindow::moveTaskUp() {
    const int row = taskList ? taskList->currentRow() : -1;
    if (row <= 0 || row >= tasks.size()) return;
    std::swap(tasks[row], tasks[row - 1]);
    refreshTasks();
    taskList->setCurrentRow(row - 1);
}

void MainWindow::moveTaskDown() {
    const int row = taskList ? taskList->currentRow() : -1;
    if (row < 0 || row + 1 >= tasks.size()) return;
    std::swap(tasks[row], tasks[row + 1]);
    refreshTasks();
    taskList->setCurrentRow(row + 1);
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
        rooms[0].connections = {1}; rooms[0].gridRow = 1; rooms[0].gridColumn = 0;
        rooms[1].connections = {0}; rooms[1].gridRow = 1; rooms[1].gridColumn = 2;
    } else {
        rooms.push_back({RoomType::DemonShop, false, {}});
        QVector<RoomType> rest = {RoomType::Chest, RoomType::Battle, RoomType::Battle, RoomType::Battle, RoomType::Battle, RoomType::EliteBattle};
        std::shuffle(rest.begin(), rest.end(), *QRandomGenerator::global());
        for (RoomType type : rest) {
            bool elite = type == RoomType::EliteBattle;
            QVector<EnemyData> enemies = (type == RoomType::Chest) ? QVector<EnemyData>{} : makeEnemyGroup(layer, elite, false);
            rooms.push_back({type, false, enemies});
        }
        // 入口商店与精英出口之间的网状分支，所有房间最多四向相连。
        const QVector<QPair<int, int>> positions = {{1, 0}, {0, 1}, {1, 1}, {2, 1}, {0, 2}, {1, 2}, {1, 3}};
        const QVector<QVector<int>> links = {{1, 2}, {0, 2, 4}, {0, 1, 3, 5}, {2, 5}, {1, 5}, {2, 3, 4, 6}, {5}};
        int eliteIndex = 1;
        for (int i = 1; i < rooms.size(); ++i) if (rooms[i].type == RoomType::EliteBattle) eliteIndex = i;
        if (eliteIndex != 6) std::swap(rooms[eliteIndex], rooms[6]);
        for (int i = 0; i < rooms.size(); ++i) {
            rooms[i].gridRow = positions[i].first;
            rooms[i].gridColumn = positions[i].second;
            rooms[i].connections = links[i];
        }
    }
    rooms[0].visited = true;
    for (const auto& room : rooms) {
        for (const auto& enemy : room.enemies) {
            codex[enemy.name] = enemy;
            if (!encountered.contains(enemy.name)) encountered[enemy.name] = false;
        }
    }
    rebuildDungeonMap();
}

void MainWindow::rebuildDungeonMap() {
    if (!dungeonMapGrid) return;
    while (QLayoutItem* item = dungeonMapGrid->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    for (int i = 0; i < rooms.size(); ++i) {
        const RoomData& room = rooms[i];
        QString title;
        const bool adjacent = rooms.value(currentRoom).connections.contains(i);
        const bool revealed = room.visited || adjacent || i == currentRoom;
        if (!revealed) title = "？\n未知房间";
        else if (room.type == RoomType::DemonShop) title = "入口\n恶魔商店";
        else if (room.type == RoomType::Chest) title = room.cleared ? "空白房" : "宝箱房";
        else if (room.type == RoomType::EliteBattle) title = room.cleared ? "出口已开启" : "精英出口";
        else if (room.type == RoomType::Boss) title = room.cleared ? "终局已完成" : "最终BOSS";
        else title = room.cleared ? "空白房" : "战斗房";
        auto* button = new QPushButton(title, gamePage);
        button->setProperty("room", i);
        button->setMinimumSize(116, 82);
        const bool current = i == currentRoom;
        button->setEnabled((current || adjacent) && !inBattle);
        button->setStyleSheet(current
            ? "background:#f39a73;color:white;border:3px solid #ffffff;border-radius:10px;font-weight:800;"
            : (!revealed ? "background:#d6e0e5;color:#617784;border:2px dashed #aebfc9;border-radius:10px;font-weight:700;"
                : (room.cleared ? "background:#dfeaf2;color:#587187;border:2px solid #b9d1e6;border-radius:10px;" : "background:#ffffff;color:#29425c;border:2px solid #78a9cb;border-radius:10px;")));
        connect(button, &QPushButton::clicked, this, &MainWindow::moveToMapRoom);
        dungeonMapGrid->addWidget(button, room.gridRow, room.gridColumn);
    }
}

void MainWindow::moveToMapRoom() {
    if (inBattle) {
        QMessageBox::information(this, "房门封锁", "战斗尚未结束，必须击败敌人后才能离开房间。");
        return;
    }
    auto* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    const int destination = button->property("room").toInt();
    if (destination != currentRoom && !rooms[currentRoom].connections.contains(destination)) return;
    currentRoom = destination;
    RoomData& room = rooms[currentRoom];
    room.visited = true;
    if (!room.cleared && room.type == RoomType::DemonShop) {
        for (auto& role : party) role.vigor = std::max(0, role.vigor - 5);
        room.cleared = true;
        appendLog(QString("进入第%1层恶魔商店：可消耗恶魔币交易并继续探索相邻房间。").arg(dungeonLayer));
    } else if (!room.cleared && room.type != RoomType::Chest) {
        for (auto& role : party) role.vigor = std::max(0, role.vigor - 5);
        startBattle(room.enemies, room.type == RoomType::EliteBattle, room.type == RoomType::Boss);
    }
    if (!room.cleared && room.type == RoomType::Chest) exploreRoom();
    refreshAll();
}

void MainWindow::enterNextDungeonLayer() {
    if (!nextLayerReady) {
        QMessageBox::information(this, "出口未开启", "需要先完成本层精英房，才能进入下一层。");
        return;
    }
    if (dungeonLayer >= 7) return;
    for (auto& role : party) role.vigor = std::max(0, role.vigor - 5);
    dungeonLayer++;
    currentRoom = 0;
    nextLayerReady = false;
    buildDungeonLayer(dungeonLayer);
    appendLog(QString("进入第%1层地窟，无法返回上一层。").arg(dungeonLayer));
    refreshAll();
    writeGame(currentSlot);
}

QVector<MainWindow::EnemyData> MainWindow::makeEnemyGroup(int layer, bool elite, bool boss) const {
    if (boss) {
        return {
            {"教导处铁腕·左", "护卫", 7, 360, 360, 52, 0, 34, 28, 160, 35, false, false, false, 0, 0, 0, "铁壁护盾；防御号令：全体防+20%"},
            {"教导处铁腕·右", "护卫", 7, 360, 360, 50, 0, 32, 30, 160, 35, false, false, false, 0, 0, 0, "净化之光；生命链接：分担左卫30%伤害"},
            {"伪典校长·零", "最终BOSS", 7, 1200, 1200, 72, 64, 46, 44, 900, 160, false, true, true, 0, 0, 0, "校规禁制、成绩单改写、真理崩坏、终末的校钟、不灭的执念"}
        };
    }
    if (elite) {
        switch (layer) {
        case 1: return {{"折翼的百米王者", "精英·体育祭的怨念", 1, 260, 260, 36, 0, 16, 10, 160, 38, true, false, false, 0, 0, 0, "起跑冲刺、弯道超越、终点的执念"}};
        case 2: return {{"扉页守护者", "精英·禁书的执念", 2, 300, 300, 18, 42, 18, 28, 210, 48, true, false, true, 0, 0, 0, "知识封印、真理扭曲、书页风暴"}};
        case 3: return {{"无面舞者", "精英·镜中的真我", 3, 360, 360, 48, 35, 24, 24, 260, 60, true, false, false, 0, 0, 0, "模仿、无面无忆、破碎舞台"}};
        case 4: return {{"畸变融合体", "精英·禁忌的合成物", 4, 470, 470, 55, 38, 30, 32, 330, 74, true, false, false, 0, 0, 0, "酸血喷溅、腐蚀光环、不稳定爆炸"}};
        case 5: return {{"时计塔的囚徒", "精英·命运的反叛者", 5, 520, 520, 40, 58, 32, 36, 410, 90, true, false, true, 0, 0, 0, "命运抽牌、时间回溯、死神牌"}};
        default: return {{"铁锈执念·陈暮", "精英·永恒的值日生", 6, 650, 650, 68, 0, 42, 34, 520, 110, true, false, false, 0, 0, 0, "积灰的扫帚、值日生点名、十年值夜的怨念、最后的坚守"}};
        }
    }
    switch (layer) {
    case 1:
        return {
            {"懈怠的短跑者A", "小怪·前排", 1, 120, 120, 24, 0, 10, 8, 45, 8, false, false, false, 0, 0, 0, "冲刺突袭：单体1.5倍伤害，降攻20%"},
            {"懈怠的短跑者B", "小怪·后排", 1, 95, 95, 22, 0, 8, 8, 40, 8, false, false, false, 0, 0, 0, "冲刺突袭：单体1.5倍伤害，降攻20%"},
            {"驼背的铅球手", "小怪·后排", 1, 145, 145, 28, 0, 14, 8, 55, 10, false, false, false, 0, 0, 0, "蓄力投掷：防御后全体大量伤害"},
            {"体操幽灵", "辅助·后排", 1, 80, 80, 0, 0, 6, 12, 35, 8, false, false, false, 0, 0, 0, "鼓励：给攻击最高队友攻击+20%"}
        };
    case 2:
        return {
            {"目录魔像", "小怪·前排", 2, 190, 190, 30, 0, 28, 12, 75, 14, false, false, false, 0, 0, 0, "检索锁定：降低目标防御30%"},
            {"涂鸦书灵A", "小怪·后排", 2, 120, 120, 0, 34, 12, 20, 65, 14, false, false, false, 0, 0, 0, "逗号飞弹：远程单体，降攻20%"},
            {"涂鸦书灵B", "小怪·后排", 2, 120, 120, 0, 34, 12, 20, 65, 14, false, false, false, 0, 0, 0, "句号禁锢：禁止目标行动1回合"},
            {"禁书管理员", "辅助·后排", 2, 140, 140, 0, 20, 10, 26, 70, 16, false, false, false, 0, 0, 0, "禁书光环：随机沉默；强制归位：清负面"}
        };
    case 3:
        return {
            {"微笑小丑", "小怪·前排", 3, 220, 220, 38, 0, 24, 16, 90, 18, false, false, false, 0, 0, 0, "来笑一个：嘲讽并提高防御"},
            {"镜面侍从A", "小怪·前排", 3, 180, 180, 42, 20, 18, 18, 88, 18, false, false, false, 0, 0, 0, "模仿：复制我方攻击最高者攻击力"},
            {"镜面侍从B", "小怪·后排", 3, 160, 160, 40, 20, 16, 18, 88, 18, false, false, false, 0, 0, 0, "远程模仿攻击"},
            {"悲情女主角", "辅助·后排", 3, 170, 170, 0, 40, 14, 24, 95, 20, false, false, false, 0, 0, 0, "治愈泪水、脆弱呢喃"}
        };
    case 4:
        return {
            {"酸液史莱姆A", "小怪·前排", 4, 250, 250, 0, 48, 22, 28, 115, 24, false, false, false, 0, 0, 0, "酸液喷射：降低防御25%，可叠加"},
            {"酸液史莱姆B", "小怪·前排", 4, 240, 240, 0, 48, 22, 28, 115, 24, false, false, false, 0, 0, 0, "酸液喷射：优先血量最低前排"},
            {"爆炸烧杯怪", "小怪·后排", 4, 170, 170, 0, 55, 12, 22, 125, 26, false, false, true, 0, 0, 0, "不稳定化合物：死亡自爆并灼烧"},
            {"剧毒试管精", "辅助·后排", 4, 180, 180, 0, 45, 14, 25, 125, 26, false, false, true, 0, 0, 0, "毒性扩散、腐蚀强化"}
        };
    case 5:
        return {
            {"塔罗士兵A", "小怪·前排", 5, 300, 300, 56, 0, 30, 28, 150, 32, false, false, false, 0, 0, 0, "每回合抽牌：力量/倒吊人/高塔"},
            {"塔罗士兵B", "小怪·后排", 5, 260, 260, 58, 0, 26, 28, 150, 32, false, false, true, 0, 0, 0, "抽牌并优先攻击血量最低者"},
            {"时针幽灵", "小怪·后排", 5, 240, 240, 0, 62, 20, 34, 165, 36, false, false, true, 0, 0, 0, "时间停滞、时之枷锁"},
            {"命运轮盘", "环境陷阱", 5, 180, 180, 0, 45, 20, 30, 120, 28, false, false, false, 0, 0, 0, "每4回合：全体攻-20%/防-20%/清除状态"}
        };
    default:
        return {
            {"教导处铁腕·左", "小怪·前排", 6, 390, 390, 64, 0, 42, 34, 210, 45, false, false, false, 0, 0, 0, "铁壁护盾；防御号令"},
            {"教导处铁腕·右", "小怪·前排", 6, 380, 380, 62, 0, 40, 36, 210, 45, false, false, false, 0, 0, 0, "净化之光；生命链接"},
            {"纪律巡查使A", "小怪·后排", 6, 280, 280, 68, 0, 30, 32, 220, 48, false, false, true, 0, 0, 0, "记过处分：降低攻击15%，可叠加"},
            {"纪律巡查使B", "小怪·后排", 6, 280, 280, 68, 0, 30, 32, 220, 48, false, false, true, 0, 0, 0, "记过处分：优先血量最低者"}
        };
    }
}

QVector<MainWindow::ItemData> MainWindow::makeLayerEquipments(int layer) const {
    struct EquipRow { const char* a; const char* b; const char* c; const char* d; int bonus; const char* scene; };
    static const EquipRow rows[] = {
        {"生锈的哑铃", "涂鸦笔记本", "护膝护具", "运动头带", 5, "废弃体育馆"},
        {"书脊铁尺", "灵摆书签", "硬壳封面盾", "羊皮纸护符", 10, "倒悬图书馆"},
        {"幕布匕首", "提词员魔杖", "小丑面具", "丝绒披肩", 15, "镜面戏剧社"},
        {"腐蚀钢爪", "试剂管法杖", "防化服内衬", "防毒面罩", 20, "失序化学实验室"},
        {"命运之刃", "占星术士杖", "星轨披风", "命运护符", 28, "占卜社旧址"},
        {"教导戒尺", "校规法典", "铁腕护臂", "净化领结", 35, "校长室前厅"}
    };
    int idx = std::max(1, std::min(6, layer)) - 1;
    int b = rows[idx].bonus;
    int price = 18 + layer * 8;
    return {
        {rows[idx].a, ItemType::Equipment, "装备", 0, price, QString("%1物攻装备，属性+%2%。").arg(rows[idx].scene).arg(b), 0, 0, 0, b, 0, 0, 0, "物攻", false},
        {rows[idx].b, ItemType::Equipment, "装备", 0, price, QString("%1法攻装备，属性+%2%。").arg(rows[idx].scene).arg(b), 0, 0, 0, 0, b, 0, 0, "法攻", false},
        {rows[idx].c, ItemType::Equipment, "装备", 0, price, QString("%1物防装备，属性+%2%。").arg(rows[idx].scene).arg(b), 0, 0, 0, 0, 0, b, 0, "物防", false},
        {rows[idx].d, ItemType::Equipment, "装备", 0, price, QString("%1法抗装备，属性+%2%。").arg(rows[idx].scene).arg(b), 0, 0, 0, 0, 0, 0, b, "法抗", false}
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
    actedPlayers.clear();
    battleRound = 1;
    battleOrder.clear();
    for (int i = 0; i < party.size(); ++i) {
        if (party[i].active && party[i].hp > 0) battleOrder.push_back(i);
    }
    battleActorIndex = 0;
    QString kind = boss ? "最终BOSS" : (elite ? "精英战斗" : "普通战斗");
    addTaskProgress("开战", "任意", 1);
    appendLog(QString("进入%1：%2。按编队站位顺序依次行动，三名角色行动后敌方全体行动。").arg(kind).arg(battleEnemies.first().name));
}

void MainWindow::fightOneRound() {
    if (!inBattle) return;
    const int roleIndex = currentBattleRole();
    if (roleIndex < 0) return;
    CharacterData& role = party[roleIndex];
    int targetIndex = battleTargetList && battleTargetList->currentItem()
        ? battleTargetList->currentItem()->data(Qt::UserRole).toInt() : firstAliveEnemy();
    if (targetIndex < 0 || targetIndex >= battleEnemies.size() || battleEnemies[targetIndex].hp <= 0) targetIndex = firstAliveEnemy();
    if (targetIndex < 0) return;

    QString action = battleActionCombo ? battleActionCombo->currentText() : "普通攻击";
    double scale = 1.0;
    int mpCost = 0;
    if (action != "普通攻击") {
        if (auto* prof = professionByName(role.profession)) {
            for (const auto& skill : prof->skills()) {
                if (QString::fromStdString(skill.name) == action) { scale = 1.0 + std::min(1.5, skill.hpCostPercent / 100.0); mpCost = skill.mpCost; break; }
            }
        }
    }
    if (role.mp < mpCost) {
        QMessageBox::information(this, "蓝量不足", "当前蓝量不足以使用这个技能。");
        return;
    }
    role.mp -= mpCost;
    const int power = std::max(role.physicalAttack, role.magicAttack);
    const int dmg = damage(power, battleEnemies[targetIndex].defense, scale);
    battleEnemies[targetIndex].hp = std::max(0, battleEnemies[targetIndex].hp - dmg);
    actedPlayers.insert(roleIndex);
    appendLog(QString("%1 使用 %2 攻击 %3，造成%4伤害。").arg(role.name).arg(action).arg(battleEnemies[targetIndex].name).arg(dmg));
    endBattleIfNeeded();
    resolveEnemyTurnIfRoundComplete();
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::resolveEnemyTurnIfRoundComplete() {
    advanceBattleActor();
}

int MainWindow::currentBattleRole() const {
    if (!inBattle || battleActorIndex < 0 || battleActorIndex >= battleOrder.size()) return -1;
    const int roleIndex = battleOrder[battleActorIndex];
    return roleIndex >= 0 && roleIndex < party.size() && party[roleIndex].active && party[roleIndex].hp > 0 ? roleIndex : -1;
}

void MainWindow::advanceBattleActor() {
    if (!inBattle) return;
    ++battleActorIndex;
    while (battleActorIndex < battleOrder.size()) {
        const int roleIndex = battleOrder[battleActorIndex];
        if (roleIndex >= 0 && roleIndex < party.size() && party[roleIndex].active && party[roleIndex].hp > 0) break;
        ++battleActorIndex;
    }
    if (battleActorIndex < battleOrder.size()) return;
    enemyTurn();
    endBattleIfNeeded();
    actedPlayers.clear();
    battleRound++;
    battleOrder.clear();
    for (int i = 0; i < party.size(); ++i) {
        if (party[i].active && party[i].hp > 0) battleOrder.push_back(i);
    }
    battleActorIndex = 0;
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
                if (party[i].active && party[i].hp > 0) {
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
        addTaskProgress("战斗胜利", "任意", 1);
        if (boss) {
            addTaskProgress("最终BOSS", "伪典校长·零", 1);
            phase = GamePhase::Ending;
            QMessageBox::information(this, "通关结局", "打破诅咒，所有角色逃离轮回。");
        } else if (elite) {
            addTaskProgress("通层", QString::number(dungeonLayer), 1);
            nextLayerReady = dungeonLayer < 7;
            appendLog("精英房已完成：下一层出口已开启，确认后将不可返回本层。");
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
    const QMap<QString, bool> keepEncountered = encountered;
    resetGameForNewRun(true);
    encountered = keepEncountered;
    registerCodexEnemies();
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
    int row = selectedInventoryRow;
    if (row < 0 || row >= inventory.size()) {
        QMessageBox::information(this, "请选择药品", "请先在背包列表选中战斗中要使用的药品或消耗品。");
        return;
    }
    ItemData item = inventory[row];
    if (!item.battleUsable || item.type == ItemType::Food) {
        QMessageBox::warning(this, "无法使用", "食品无法在战斗中使用，只有药品/战斗消耗品可用。");
        return;
    }
    const int roleIndex = currentBattleRole();
    if (roleIndex < 0) return;
    CharacterData& role = party[roleIndex];
    if (item.name == "阿司匹林" || item.name == "神圣药水") {
        role.battleStun = 0;
    }
    applyItemEffect(role, item);
    if (item.name.contains("手雷")) {
        for (auto& enemy : battleEnemies) {
            if (enemy.hp > 0) enemy.hp = std::max(0, enemy.hp - std::max(item.attackBonus, item.magicBonus));
            if (item.name == "冰冻手雷") enemy.frozen = 1;
        }
    }
    inventory.removeAt(row);
    actedPlayers.insert(roleIndex);
    appendLog(QString("%1 战斗中使用 %2，行动结束。").arg(role.name).arg(item.name));
    endBattleIfNeeded();
    resolveEnemyTurnIfRoundComplete();
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::setFormationOneFront() {
    if (inBattle) {
        QMessageBox::information(this, "战斗中不可编队", "请在进入战斗房前完成编队和站位调整。");
        return;
    }
    formationType = 1;
    formationChanged = true;
    addTaskProgress("编队", "任意", 1);
    refreshAll();
}

void MainWindow::setFormationTwoFront() {
    if (inBattle) {
        QMessageBox::information(this, "战斗中不可编队", "请在进入战斗房前完成编队和站位调整。");
        return;
    }
    formationType = 2;
    formationChanged = true;
    addTaskProgress("编队", "任意", 1);
    refreshAll();
}

void MainWindow::toggleSelectedRoleActive() {
    if (inBattle) {
        QMessageBox::information(this, "战斗中不可编队", "请在进入战斗房前完成编队和站位调整。");
        return;
    }
    int row = characterList->currentRow();
    if (row < 0 || row >= party.size()) return;
    if (party[row].active) {
        if (activeRoleCount() <= 1) {
            QMessageBox::warning(this, "至少一人上阵", "队伍至少需要保留1名上阵角色。");
            return;
        }
        party[row].active = false;
    } else {
        if (activeRoleCount() >= 3) {
            QMessageBox::warning(this, "上阵人数已满", "最多只能上阵3名角色。");
            return;
        }
        party[row].active = true;
    }
    formationChanged = true;
    addTaskProgress("编队", "任意", 1);
    refreshAll();
    writeGame(currentSlot);
}

void MainWindow::moveRoleUp() {
    if (inBattle) {
        QMessageBox::information(this, "战斗中不可编队", "请在进入战斗房前完成编队和站位调整。");
        return;
    }
    int row = characterList->currentRow();
    if (row > 0 && row < party.size()) {
        std::swap(party[row], party[row - 1]);
        formationChanged = true;
        addTaskProgress("编队", "任意", 1);
        refreshAll();
    }
}

void MainWindow::moveRoleDown() {
    if (inBattle) {
        QMessageBox::information(this, "战斗中不可编队", "请在进入战斗房前完成编队和站位调整。");
        return;
    }
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
    for (const auto& role : party) if (role.active && role.hp > 0) count++;
    return count;
}

int MainWindow::activeRoleCount() const {
    int count = 0;
    for (const auto& role : party) if (role.active) count++;
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
    for (int i = 0; i < party.size(); ++i) if (party[i].active && party[i].hp > 0) return i;
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

QString MainWindow::activeText(const CharacterData& role, int visibleIndex) const {
    if (!role.active) return "未上阵";
    if (formationType == 1) return visibleIndex == 0 ? "上阵·前排" : "上阵·后排";
    return visibleIndex < 2 ? "上阵·前排" : "上阵·后排";
}

QString MainWindow::battleStatusText() const {
    QStringList lines;
    QStringList players;
    int visibleIndex = 0;
    for (const auto& role : party) {
        QString pos = activeText(role, visibleIndex);
        if (role.active) visibleIndex++;
        players << QString("%1[%2] HP %3/%4 MP %5/%6")
            .arg(role.name, pos).arg(role.hp).arg(role.maxHp).arg(role.mp).arg(role.maxMp);
    }
    lines << "我方：" + (players.isEmpty() ? "无角色" : players.join("；"));
    if (inBattle && !battleEnemies.isEmpty()) {
        QStringList enemies;
        for (const auto& enemy : battleEnemies) {
            QString state;
            if (enemy.frozen > 0) state += " 冻结";
            if (enemy.slow > 0) state += " 迟缓";
            enemies << QString("%1[%2] HP %3/%4%5")
                .arg(enemy.name, enemy.kind).arg(enemy.hp).arg(enemy.maxHp).arg(state);
        }
        lines << "敌方：" + enemies.join("；");
    } else {
        lines << "敌方：当前没有进行中的战斗。";
    }
    return lines.join("\n");
}

int MainWindow::selectedRoleOrFirstAlive() const {
    int row = characterList ? characterList->currentRow() : -1;
    if (row >= 0 && row < party.size() && party[row].hp > 0) return row;
    return firstAlivePlayer();
}

void MainWindow::applyItemEffect(CharacterData& role, const ItemData& item) {
    int hpGain = item.hpRecover;
    if (item.name == "回血药" && hpGain == 0) hpGain = role.maxHp * 30 / 100;
    role.hp = std::min(role.maxHp, role.hp + hpGain);
    role.mp = std::min(role.maxMp, role.mp + item.mpRecover);
    role.vigor = std::min(160, role.vigor + item.staminaRecover);
    role.physicalAttack += item.attackBonus;
    role.magicAttack += item.magicBonus;
    role.physicalDefense += item.defenseBonus;
    role.magicResistance += item.resistBonus;
}

void MainWindow::applyEquipmentStats(CharacterData& role, const ItemData& item, int sign) {
    role.physicalAttack = std::max(1, role.physicalAttack + item.attackBonus * sign);
    role.magicAttack = std::max(1, role.magicAttack + item.magicBonus * sign);
    role.physicalDefense = std::max(0, role.physicalDefense + item.defenseBonus * sign);
    role.magicResistance = std::max(0, role.magicResistance + item.resistBonus * sign);
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
    if (name == "第一层学生木剑" || name == "生锈的哑铃") return makeLayerEquipments(1).first();
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
            QString::number(role.physicalDefense), QString::number(role.magicResistance), QString::number(role.active ? 1 : 0), equipRows.join(",")
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
        QString equipText;
        if (p.size() >= 16) {
            role.active = p.value(14).toInt() != 0;
            equipText = p.value(15);
        } else {
            role.active = party.size() < 3;
            equipText = p.value(14);
        }
        for (const QString& equip : equipText.split(",", Qt::SkipEmptyParts)) {
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
