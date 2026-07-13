#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

class QComboBox;
class QGridLayout;
class QLabel;
class QListWidget;
class QPushButton;
class QTableWidget;
class QTabWidget;
class QTextEdit;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    struct CharacterData {
        QString name;
        QString role;
        QString desc;
        int level = 1;
        int exp = 0;
        int hp = 100;
        int maxHp = 100;
        int mp = 50;
        int maxMp = 50;
        int stamina = 100;
        int maxStamina = 100;
        int physAtk = 10;
        int magicAtk = 10;
        int physDef = 5;
        int magicRes = 5;
        bool active = false;
        QString position = "后排";
        QStringList skills;
    };

    struct ItemData {
        QString name;
        QString type;
        QString desc;
        int price = 0;
        bool battleUsable = false;
    };

private:
    void setupUi();
    void setupSaveTab();
    void setupRoleTab();
    void setupScheduleTab();
    void setupAngelShopTab();
    void setupDungeonTab();
    void setupFormationTab();
    void setupInventoryTab();
    void setupJournalTab();
    void setupQuestTab();
    void applyStyle();

    void initializeCatalogs();
    void createDefaultSchedule();
    void createInitialParty(const QStringList& names);
    void clearGameState();
    void refreshAll();
    void saveScheduleFromTable();
    void loadScheduleToTable(int characterIndex);
    void refreshSaveList();
    void refreshRoleTable();
    void refreshScheduleSummary();
    void refreshAngelShop();
    void refreshDungeonView();
    void refreshFormationView();
    void refreshInventoryList();
    void refreshJournal();
    void refreshQuestList();
    void appendLog(const QString& message);

    QString saveDirectory() const;
    QString savePath(int slot) const;
    bool saveExists(int slot) const;
    QString saveSummary(int slot) const;
    bool saveGame(int slot);
    bool loadGame(int slot);
    void deleteSaveFile(int slot);

    void addItem(const ItemData& item, int count = 1);
    void grantExp(int amount);
    void applyLevelUps(CharacterData& c);
    int partyPower() const;
    int activeCount() const;
    QVector<int> activeIndexes() const;
    ItemData equipmentForFloor(int floorValue) const;
    ItemData findCatalogItem(const QString& name) const;
    QString currentRoomType() const;
    QString enemyNameForFloor(int floorValue, bool elite) const;
    void enterNewFloor(int nextFloor);
    void resolveBattle(bool elite, bool boss);
    void handlePartyDefeat();
    void snapshotBeforeDungeon();
    void restoreBeforeDungeon();
    void completeQuest(const QString& key);

    void onNewSave();
    void onLoadSave();
    void onSaveCurrent();
    void onDeleteSave();
    void onApplySchedule();
    void onWinterRest();
    void onWinterWork();
    void onBuyAngelItem();
    void onEnterDungeon();
    void onNextRoom();
    void onOpenTreasure();
    void onDemonShop();
    void onBuyDemonItem();
    void onSetFormation();
    void onToggleActive();
    void onUseInventoryItem();
    void onDropInventoryItem();

    QTabWidget* tabs = nullptr;

    QListWidget* saveList = nullptr;
    QPushButton* newSaveBtn = nullptr;
    QPushButton* loadSaveBtn = nullptr;
    QPushButton* saveCurrentBtn = nullptr;
    QPushButton* deleteSaveBtn = nullptr;

    QTableWidget* roleTable = nullptr;
    QTextEdit* skillText = nullptr;

    QComboBox* scheduleCharacterBox = nullptr;
    QTableWidget* scheduleTable = nullptr;
    QLabel* scheduleSummaryLabel = nullptr;

    QListWidget* angelShopList = nullptr;
    QLabel* angelGoldLabel = nullptr;

    QLabel* dungeonStateLabel = nullptr;
    QTextEdit* dungeonLog = nullptr;
    QPushButton* enterDungeonBtn = nullptr;
    QPushButton* nextRoomBtn = nullptr;
    QPushButton* treasureBtn = nullptr;
    QPushButton* demonShopBtn = nullptr;
    QListWidget* demonShopList = nullptr;

    QListWidget* formationList = nullptr;
    QComboBox* formationBox = nullptr;

    QListWidget* inventoryList = nullptr;
    QLabel* inventorySummaryLabel = nullptr;

    QTextEdit* journalText = nullptr;
    QListWidget* questList = nullptr;

    QVector<CharacterData> party;
    QVector<CharacterData> preDungeonParty;
    QVector<ItemData> inventory;
    QVector<ItemData> preDungeonInventory;
    QVector<ItemData> angelCatalog;
    QVector<ItemData> demonCatalog;
    QSet<QString> knownEnemies;
    QSet<QString> completedQuests;
    QVector<QStringList> schedules;

    int activeSaveSlot = -1;
    int gold = 0;
    int demonCoins = 0;
    int loopCount = 1;
    int termCount = 1;
    int currentFloor = 0;
    int currentRoom = 0;
    int eliteRoom = 6;
    int chestRoom = 3;
    int formationMode = 0;
    bool hasGame = false;
    bool inDungeon = false;
    bool preDungeonSnapshotValid = false;
};

#endif
