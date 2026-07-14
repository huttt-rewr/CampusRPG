// MainWindow.h
// Qt 主窗口定义：实现 4 存档、轮回 Roguelike 流程、排课、商店、地窟、编队、图鉴等界面。
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Profession.h"

#include <QMainWindow>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>
#include <memory>
#include <vector>

class QGridLayout;
class QComboBox;
class QLabel;
class QListWidget;
class QPushButton;
class QStackedWidget;
class QTableWidget;
class QTabWidget;
class QTextEdit;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void slotClicked();
    void createInSelectedSlot();
    void deleteSelectedSlot();
    void saveGame();
    void runSemester();
    void winterRest();
    void winterWork();
    void enterAngelShop();
    void buyAngelItem();
    void buyDemonItem();
    void sellEquipmentToDemon();
    void useInventoryItem();
    void discardInventoryItem();
    void equipSelectedItem();
    void acceptTask();
    void claimTask();
    void setFormationOneFront();
    void setFormationTwoFront();
    void toggleSelectedRoleActive();
    void moveRoleUp();
    void moveRoleDown();
    void enterDungeon();
    void exploreRoom();
    void fightOneRound();
    void useBattleMedicine();
    void moveToMapRoom();
    void enterNextDungeonLayer();
    void moveTaskUp();
    void moveTaskDown();

private:
    enum class ItemType { Medicine, Food, Growth, Consumable, Equipment };
    enum class RoomType { DemonShop, Battle, EliteBattle, Chest, Boss };
    enum class GamePhase { SaveSelect, SchoolFirst, Winter, SchoolSecond, AngelShop, Dungeon, Ending };

    struct ItemData {
        QString name;
        ItemType type = ItemType::Food;
        QString category;
        int price = 0;
        int demonPrice = 0;
        QString effect;
        int hpRecover = 0;
        int mpRecover = 0;
        int staminaRecover = 0;
        int attackBonus = 0;
        int magicBonus = 0;
        int defenseBonus = 0;
        int resistBonus = 0;
        QString equipSlot;
        bool battleUsable = false;
    };

    struct CharacterData {
        QString name;
        QString profession;
        int level = 1;
        int exp = 0;
        int expNeed = 100;
        int hp = 100;
        int maxHp = 100;
        int mp = 50;
        int maxMp = 50;
        int vigor = 100;
        int physicalAttack = 10;
        int magicAttack = 10;
        int physicalDefense = 5;
        int magicResistance = 5;
        int battleStun = 0;
        int tauntRounds = 0;
        bool active = true;
        QMap<QString, QString> equipment;
    };

    struct TaskData {
        int id = 0;
        QString name;
        QString description;
        QString conditionType;
        QString target;
        int need = 1;
        int progress = 0;
        int status = 0;
        int rewardGold = 0;
        int rewardDemonCoin = 0;
        QString rewardItem;
    };

    struct EnemyData {
        QString name;
        QString kind;
        int layer = 1;
        int hp = 80;
        int maxHp = 80;
        int attack = 12;
        int magicAttack = 0;
        int defense = 4;
        int resist = 4;
        int exp = 20;
        int demonCoin = 5;
        bool elite = false;
        bool boss = false;
        bool backlineAttack = false;
        int frozen = 0;
        int slow = 0;
        int burn = 0;
        QString skills;
    };

    struct RoomData {
        RoomType type = RoomType::Battle;
        bool cleared = false;
        QVector<EnemyData> enemies;
        bool visited = false;
        QVector<int> connections;
        int gridRow = 0;
        int gridColumn = 0;
    };

    struct ScheduleHalfDay {
        QString firstAction = "不上课";
        QString extraCourse;
        bool tutoring = false;
    };

    void setupData();
    void setupSavePage();
    void setupGamePage();
    void updateUiScale();
    void rebuildScheduleTable();
    void loadScheduleForCurrentRole();
    void connectActions();
    void showSavePage();
    void showGamePage();
    void refreshAll();
    void refreshSaveSlots();
    void refreshOverview();
    void refreshSchedulePreview();
    void refreshCharacters();
    void refreshInventory();
    void refreshTasks();
    void refreshAngelShop();
    void refreshDemonShop();
    void refreshDungeon();
    void refreshCodex();
    void appendLog(const QString& text);
    void resetGameForNewRun(bool keepGrowth);
    void createCharacter(const QString& professionName, const QString& roleName);
    void applyProfessionStats(CharacterData& role, const Profession& profession);
    void addExp(CharacterData& role, int amount);
    void addItem(const ItemData& item);
    void addTaskProgress(const QString& type, const QString& target, int amount);
    void checkTaskCompletion();
    void buildDungeonLayer(int layer);
    void rebuildDungeonMap();
    void registerCodexEnemies();
    void updatePhaseUi();
    void advanceBattleActor();
    int currentBattleRole() const;
    void resolveEnemyTurnIfRoundComplete();
    QVector<EnemyData> makeEnemyGroup(int layer, bool elite, bool boss) const;
    QVector<ItemData> makeLayerEquipments(int layer) const;
    void startBattle(const QVector<EnemyData>& enemies, bool elite, bool boss);
    void enemyTurn();
    void endBattleIfNeeded();
    void nextLoopAfterDeath();
    int alivePlayerCount() const;
    int activeRoleCount() const;
    int aliveEnemyCount() const;
    int firstAliveEnemy() const;
    int firstAlivePlayer() const;
    int damage(int attack, int defense, double scale) const;
    QString itemTypeText(ItemType type) const;
    QString phaseText() const;
    QString taskStatusText(int status) const;
    QString roleText(const CharacterData& role) const;
    QString skillsText(const CharacterData& role) const;
    QString inventoryText(const ItemData& item) const;
    QString battleStatusText() const;
    int selectedRoleOrFirstAlive() const;
    void applyItemEffect(CharacterData& role, const ItemData& item);
    void applyEquipmentStats(CharacterData& role, const ItemData& item, int sign);
    void updateVisualPreviews();
    QString saveFilePath(int slot) const;
    QString currentSaveFilePath() const;
    void loadSlotMeta();
    bool loadGame(int slot);
    void writeGame(int slot);
    void deleteGameFile(int slot);
    ItemData itemByName(const QString& name) const;
    Profession* professionByName(const QString& name) const;
    QString serializeInventory() const;
    void deserializeInventory(const QString& text);
    QString serializeCharacters() const;
    void deserializeCharacters(const QString& text);
    QString serializeTasks() const;
    void deserializeTasks(const QString& text);
    QString serializeCodex() const;
    void deserializeCodex(const QString& text);
    QString activeText(const CharacterData& role, int visibleIndex) const;

    QStackedWidget* stack = nullptr;
    QWidget* savePage = nullptr;
    QWidget* gamePage = nullptr;
    QGridLayout* slotGrid = nullptr;
    QLabel* saveTitleLabel = nullptr;
    QLabel* saveHintLabel = nullptr;
    QLabel* saveSceneLabel = nullptr;
    QTabWidget* tabs = nullptr;
    QLabel* overviewLabel = nullptr;
    QLabel* overviewSceneLabel = nullptr;
    QPushButton* taskHeaderButton = nullptr;
    QTextEdit* logText = nullptr;
    QTableWidget* scheduleTable = nullptr;
    QLabel* schedulePreviewLabel = nullptr;
    QLabel* scheduleRoleLabel = nullptr;
    QListWidget* characterList = nullptr;
    QLabel* characterPreviewLabel = nullptr;
    QListWidget* inventoryList = nullptr;
    QListWidget* characterInventoryList = nullptr;
    QListWidget* taskList = nullptr;
    QListWidget* angelShopList = nullptr;
    QListWidget* demonShopList = nullptr;
    QListWidget* dungeonRoomList = nullptr;
    QGridLayout* dungeonMapGrid = nullptr;
    QListWidget* battleTargetList = nullptr;
    QComboBox* battleActionCombo = nullptr;
    QLabel* battleTurnLabel = nullptr;
    QListWidget* codexList = nullptr;
    QLabel* dungeonLabel = nullptr;
    QLabel* dungeonSceneLabel = nullptr;
    QLabel* dungeonPreviewLabel = nullptr;
    QPushButton* fightRoundBtn = nullptr;
    QPushButton* battleMedicineBtn = nullptr;

    int currentSlot = -1;
    int loopCount = 1;
    int semester = 1;
    int week = 1;
    int gold = 100;
    int demonCoin = 0;
    int initialGoldBonus = 0;
    int initialVigorBonus = 0;
    int workGoldBonus = 0;
    int demonFriendBonus = 0;
    int angelDiscount = 0;
    int demonDiscount = 0;
    int formationType = 1;
    int dungeonLayer = 0;
    int currentRoom = 0;
    int schedulingRoleIndex = 0;
    int battleRound = 1;
    int battleActorIndex = 0;
    int selectedInventoryRow = -1;
    int selectedSlot = -1;
    bool inBattle = false;
    bool firstAngelShopBought = false;
    bool firstDemonShopBought = false;
    bool equippedOnce = false;
    bool formationChanged = false;
    bool nextLayerReady = false;
    GamePhase phase = GamePhase::SaveSelect;

    QVector<CharacterData> party;
    QVector<ItemData> inventory;
    QVector<ItemData> angelGoods;
    QVector<ItemData> demonGoods;
    QVector<TaskData> tasks;
    QVector<RoomData> rooms;
    QVector<EnemyData> battleEnemies;
    QMap<QString, EnemyData> codex;
    QMap<QString, bool> encountered;
    QMap<QString, QVector<QString>> savedSchedules;
    QMap<QString, bool> keepSchedulePreset;
    QSet<int> actedPlayers;
    QVector<int> battleOrder;
    std::vector<std::unique_ptr<Profession>> professions;
    QMap<QString, QString> saveNames;
};

#endif
