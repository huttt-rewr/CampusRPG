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
#include "GameManager.h"
#include "MapWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // Character
    void refreshCharacterPanel();

    // Inventory
    void refreshInventory();
    void onUseItem();
    void onDropItem();

    // Shop
    void refreshShop();
    void onBuyItem();
    void onSellItem();

    // Task
    void refreshTasks();
    void onAcceptTask();
    void onClaimReward();

    // Map
    void onMapLocationClicked(int idx);
    void onRest();
    void refreshMapPanel();

    // Battle
    void refreshBattle();
    void onStartBattle();
    void onBattleTick();

private:
    void setupUI();
    void createCharacterTab();
    void createInventoryTab();
    void createShopTab();
    void createTaskTab();
    void createMapTab();
    void createBattleTab();
    void applyStyleSheet();
    void refreshAll();

    GameManager game;

    // Tab widget
    QTabWidget* tabs;

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

    int currentEnemyIndex;
    bool inBattle;
};

#endif
