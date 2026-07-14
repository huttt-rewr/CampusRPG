import os
from docx import Document
from docx.shared import Pt, Cm, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml.ns import qn

FONT = "Microsoft YaHei"
BASE = "h:/CampusRPG_作业版/"

def F(run, size=12, bold=False, color=None):
    run.font.name = FONT
    run._element.rPr.rFonts.set(qn('w:eastAsia'), FONT)
    run.font.size = Pt(size)
    run.bold = bold
    if color:
        run.font.color.rgb = RGBColor(*color)

def H(doc, text, level=1):
    h = doc.add_heading(text, level=level)
    for r in h.runs:
        s = {0:26,1:18,2:15,3:13}.get(level,13)
        F(r, size=s, bold=True)

def P(doc, text, size=12, bold=False, indent=0):
    p = doc.add_paragraph()
    if indent: p.paragraph_format.first_line_indent = Cm(indent)
    if text:
        run = p.add_run(text)
        F(run, size=size, bold=bold)

def add_table(doc, headers, rows):
    tbl = doc.add_table(rows=1+len(rows), cols=len(headers))
    tbl.style = 'Light Grid Accent 1'
    tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
    for i, h in enumerate(headers):
        cell = tbl.rows[0].cells[i]
        cell.text = h
        for p in cell.paragraphs:
            for r in p.runs: F(r, size=11, bold=True)
    for ri, row in enumerate(rows):
        for ci, val in enumerate(row):
            cell = tbl.rows[ri+1].cells[ci]
            cell.text = str(val)
            for p in cell.paragraphs:
                for r in p.runs: F(r, size=10)
    return tbl

def title_page(doc, name):
    for _ in range(6): doc.add_paragraph()
    t = doc.add_paragraph()
    t.alignment = WD_ALIGN_PARAGRAPH.CENTER
    r = t.add_run("校园RPG冒险游戏")
    F(r, size=36, bold=True)
    s = doc.add_paragraph()
    s.alignment = WD_ALIGN_PARAGRAPH.CENTER
    r = s.add_run(f"答辩陈述 - {name}")
    F(r, size=20, bold=True, color=(80,80,80))
    for _ in range(3): doc.add_paragraph()
    b = doc.add_paragraph()
    b.alignment = WD_ALIGN_PARAGRAPH.CENTER
    r = b.add_run("计科2503班  -  2026年7月")
    F(r, size=14, color=(100,100,100))
    doc.add_page_break()

members = [
    {
        "name": "谢文浩",
        "role": "组长",
        "pct": "30%",
        "modules": "系统架构 + 战斗系统 + 地窟探索 + 等级成长 + Git管理",
        "table": [
            ["系统架构设计", "定义 MainWindow.h 全部数据结构（284行）；模块间接口规范；Profession抽象基类到6个派生职业的继承体系设计"],
            ["回合制战斗引擎", "startBattle()/fightOneRound()/enemyTurn()/endBattleIfNeeded()完整战斗流程；先手判定算法；伤害公式max(1,攻-防)；前后排目标选择；6种异常状态（冻结/灼烧/减速/眩晕/沉默/嘲讽）；120种技能效果实现"],
            ["地窟生成系统", "buildDungeonLayer() 7层配置；makeEnemyGroup() 生成普通/精英/BOSS共30+种敌人及AI；makeLayerEquipments() 7层装备掉落表"],
            ["等级成长系统", "addExp() 经验累计与升级；属性按职业系数增长；技能按Lv1/5/10/15/20五档解锁"],
            ["Git仓库管理", "分支策略、代码合并、冲突解决、GitHub Releases版本发布"],
        ],
        "highlights": [
            "战斗引擎如何用C++多态实现6种异常状态的统一管理",
            "地窟生成算法如何保证每层难度递增且可玩性",
            "Profession抽象基类+虚函数实现新增职业零修改",
        ],
        "tips": [
            "重点展示MainWindow.h的数据结构设计思路",
            "演示战斗全流程：选敌-先手判定-伤害计算-异常状态-胜负",
            "说明地窟7层递增难度的生成算法",
            "强调多态设计优势：新增职业只需继承，不改其他代码",
        ],
    },
    {
        "name": "黄金弘",
        "role": "组员",
        "pct": "24%",
        "modules": "商店系统 + 任务系统 + 数据持久化 + 存档序列化",
        "table": [
            ["天使商店", "refreshAngelShop()/buyAngelItem()实现药品/食品/养成类共14种商品；折扣累计系统（最高50%）"],
            ["恶魔商店", "refreshDemonShop()/buyDemonItem()/sellEquipmentToDemon()消耗品/装备/恶魔之友；恶魔币独立货币；装备半价回收"],
            ["任务系统", "refreshTasks()/acceptTask()/claimTask()完整生命周期；3种条件类型；checkTaskCompletion()自动检测；金币+恶魔币+物品奖励"],
            ["数据持久化", "writeGame()/loadGame() INI文本存档；4个独立槽位；8个序列化函数实现角色/物品/任务/图鉴完整读写"],
        ],
        "highlights": [
            "INI文本存档格式：可读性强、可手动调试、不易出错",
            "8个序列化函数的分组设计理念",
            "任务系统自动检测：玩家行为如何触发进度更新",
        ],
        "tips": [
            "打开slot_1.ini展示可读的存档格式",
            "说明8个序列化函数为何分组而非合并",
            "演示任务从接受到自动检测到领取奖励的完整数据流",
            "讲解双商店双货币的经济平衡设计理念",
        ],
    },
    {
        "name": "张轩",
        "role": "组员",
        "pct": "23%",
        "modules": "角色管理 + 背包系统 + 装备穿戴 + 编队系统",
        "table": [
            ["角色管理", "createCharacter() 6职业创建流程；refreshCharacters()角色详情；applyProfessionStats()职业差异化属性赋值"],
            ["背包系统", "refreshInventory() 40格背包与容量检测；useInventoryItem()战斗内外使用逻辑分离；addItem()容量溢出提示"],
            ["装备系统", "equipSelectedItem()穿戴/卸下切换；4种槽位(物攻/法攻/物防/法抗)独立管理；百分比属性加成计算"],
            ["编队系统", "setFormationOneFront()/setFormationTwoFront()双阵型；moveRoleUp()/moveRoleDown()角色排序；toggleSelectedRoleActive()启用/禁用"],
        ],
        "highlights": [
            "编队系统的两种阵型如何影响战斗策略",
            "4槽位装备穿戴逻辑和属性加成计算公式",
            "背包战斗限制（食品不可用）的设计意图",
        ],
        "tips": [
            "演示编队两种阵型切换和角色排序的交互效果",
            "展示装备4槽位的检查/替换/卸下完整流程",
            "说明背包战斗限制增加策略性的设计理念",
            "展示角色创建中不同职业的属性差异化赋值",
        ],
    },
    {
        "name": "张申桥",
        "role": "组员",
        "pct": "23%",
        "modules": "Qt图形界面 + 排课系统 + 集成测试 + 文档与资源管理",
        "table": [
            ["Qt界面开发", "setupSavePage()/setupGamePage()双页面架构；7个Tab页；暗色主题CSS；updateUiScale()自适应缩放；精灵图图标显示"],
            ["排课系统", "rebuildScheduleTable() 7天x2半天=14格课表；refreshSchedulePreview()收益实时预览；工作日9科+周末3种下拉选择"],
            ["集成测试", "端到端流程验证；13项测试用例覆盖全部模块"],
            ["文档与资源", "README.md项目文档；检查点报告；72张AI素材整理；免安装启动包制作"],
        ],
        "highlights": [
            "Qt GUI的7个Tab架构与后台逻辑模块的对应关系",
            "排课系统14格课表实时计算的设计思路",
            "72张AI素材的生产管线和规范化命名",
        ],
        "tips": [
            "截取7个Tab页面截图展示Qt界面效果",
            "演示排课系统14格课表和实时预览的计算逻辑",
            "展示免安装启动包的完整运行流程",
            "说明AI素材生成流程和命名规范",
        ],
    },
]

for m in members:
    doc = Document()
    title_page(doc, m["name"])

    H(doc, f"{m['name']}（{m['role']}）- 贡献度 {m['pct']}", level=1)
    P(doc, f"负责模块：{m['modules']}", bold=True)
    P(doc, "")

    add_table(doc, ["工作内容", "具体产出与关键技术"], m["table"])

    P(doc, "")
    H(doc, "技术亮点", level=2)
    for h in m["highlights"]:
        P(doc, f"  - {h}", indent=0.7)

    P(doc, "")
    H(doc, "答辩准备建议", level=2)
    for t in m["tips"]:
        P(doc, f"  - {t}", indent=0.7)

    filename = f"{m['name']}_答辩.docx"
    path = BASE + filename
    doc.save(path)
    print(f"Saved: {filename}")

print("All 4 documents generated!")
