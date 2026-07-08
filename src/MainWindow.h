#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QListWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <QSpinBox>
#include "GameManager.h"
#include "MapWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // Character
    void refreshSaveSlots();
    void onNewSave();
    void onLoadSave();
    void onSaveCurrent();
    void onDeleteSave();
    void refreshCharacterPanel();

    // Inventory
    void refreshInventory();
    void onUseItem();
    void onDropItem();

    // Shop
    void refreshShop();
    void onBuyItem();
    void onSellItem();

    // Town services
    void refreshTownServices();
    void onDepositItem();
    void onWithdrawItem();
    void onDepositGold();
    void onWithdrawGold();
    void onReinforceItem();

    // Task
    void refreshTasks();
    void onAcceptTask();
    void onClaimReward();

    // Map
    void onMapLocationClicked(int idx);
    void onRest();
    void onAdventure();
    void onReturnToTown();
    void refreshMapPanel();

    // Battle
    void refreshBattle();
    void onStartBattle();
    void onBattleTick();
    void onUseBattleItem();

private:
    void setupUI();
    void createCharacterTab();
    void createSaveTab();
    void createInventoryTab();
    void createShopTab();
    void createTownTab();
    void createTaskTab();
    void createMapTab();
    void createBattleTab();
    void applyStyleSheet();
    void refreshAll();

    GameManager game;

    // Tab widget
    QTabWidget* tabs;

    // ===== Save tab =====
    QListWidget* saveList;
    QPushButton* newSaveBtn;
    QPushButton* loadSaveBtn;
    QPushButton* saveCurrentBtn;
    QPushButton* deleteSaveBtn;

    // ===== Character tab =====
    QLabel* charNameLabel;
    QLabel* charLevelLabel;
    QProgressBar* hpBar;
    QLabel* hpLabel;
    QProgressBar* expBar;
    QLabel* expLabel;
    QLabel* goldLabel;
    QLabel* atkLabel;
    QLabel* defLabel;

    // ===== Inventory tab =====
    QListWidget* invList;
    QPushButton* useItemBtn;
    QPushButton* dropItemBtn;

    // ===== Shop tab =====
    QListWidget* shopList;
    QPushButton* buyBtn;
    QListWidget* playerSellList;
    QPushButton* sellBtn;
    QLabel* shopGoldLabel;

    // ===== Town services tab =====
    QLabel* townStatusLabel;
    QListWidget* townBackpackList;
    QListWidget* warehouseList;
    QPushButton* depositItemBtn;
    QPushButton* withdrawItemBtn;
    QLabel* bankGoldLabel;
    QPushButton* depositGoldBtn;
    QPushButton* withdrawGoldBtn;
    QListWidget* forgeList;
    QPushButton* reinforceBtn;

    // ===== Task tab =====
    QListWidget* taskList;
    QPushButton* acceptBtn;
    QPushButton* claimBtn;
    QTextEdit* taskDetail;

    // ===== Map tab =====
    MapWidget* mapWidget;
    QLabel* mapLocationLabel;
    QLabel* mapDescLabel;
    QTextEdit* mapLog;
    QPushButton* restBtn;
    QPushButton* adventureBtn;
    QPushButton* returnTownBtn;
    QListWidget* reachableList;

    // ===== Battle tab =====
    QComboBox* enemyCombo;
    QPushButton* battleBtn;
    QProgressBar* battleHpBar;
    QLabel* battleHpLabel;
    QProgressBar* enemyHpBar;
    QLabel* enemyHpLabel;
    QTextEdit* battleLog;
    QLabel* battleStatus;
    QTimer* battleTimer;
    QListWidget* battleItemList;
    QPushButton* useBattleItemBtn;

    int currentEnemyIndex;
    bool inBattle;
    int rageTurnsLeft;
    int rageAttackBonus;
};

#endif
