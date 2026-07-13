// MainWindow.h
// Qt 主窗口定义：提供校园 RPG 的图形化菜单界面。
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QVector>

class QLabel;
class QListWidget;
class QPushButton;
class QTabWidget;
class QTextEdit;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void newRole();
    void saveGame();
    void loadGame();
    void buySelectedItem();
    void sellSelectedItem();
    void useSelectedItem();
    void deleteSelectedItem();
    void acceptSelectedTask();
    void claimSelectedTask();
    void fightNormalEnemy();
    void fightEliteEnemy();
    void fightBossEnemy();

private:
    struct RoleData {
        QString name = "未命名同学";
        int level = 1;
        int hp = 100;
        int maxHp = 100;
        int exp = 0;
        int expNeed = 100;
        int gold = 100;
        int attack = 18;
        int defense = 6;
    };

    struct ItemData {
        QString name;
        QString type;
        int price = 0;
        QString desc;
        int heal = 0;
        int attackBonus = 0;
        int defenseBonus = 0;
        bool equipped = false;
        bool battleUsable = false;
    };

    struct TaskData {
        int id = 0;
        QString name;
        QString desc;
        QString conditionType;
        QString target;
        int need = 0;
        int progress = 0;
        int status = 0;
        int rewardExp = 0;
        int rewardGold = 0;
        QString rewardItem;
    };

    struct EnemyData {
        QString name;
        QString type;
        int hp = 0;
        int attack = 0;
        int defense = 0;
        int exp = 0;
        int gold = 0;
        QString drop;
        int dropRate = 0;
    };

    void setupUi();
    void setupData();
    void refreshAll();
    void refreshRole();
    void refreshInventory();
    void refreshShop();
    void refreshTasks();
    void appendLog(const QString& text);
    void addExp(int amount);
    void addItem(const ItemData& item);
    ItemData makeItemByName(const QString& name) const;
    QString itemText(const ItemData& item) const;
    QString taskStatusText(int status) const;
    void updateCollectTasks();
    void updateDefeatTasks(const QString& enemyName);
    void fightEnemy(const EnemyData& enemy);
    QString savePath() const;

    QTabWidget* tabs = nullptr;
    QLabel* roleLabel = nullptr;
    QListWidget* inventoryList = nullptr;
    QListWidget* shopList = nullptr;
    QListWidget* taskList = nullptr;
    QTextEdit* logText = nullptr;

    RoleData role;
    QVector<ItemData> inventory;
    QVector<ItemData> goods;
    QVector<TaskData> tasks;
};

#endif
