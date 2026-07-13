"""Build packaged game art from the supplied model folders.

The models use a light, connected studio background.  This script removes only
the background region connected to the image border, so light details inside a
character silhouette are preserved.  It is intentionally deterministic: rerun
it whenever a source model changes.
"""

from __future__ import annotations

from pathlib import Path
from shutil import copy2

import cv2
import numpy as np
from PIL import Image, ImageFilter


ROOT = Path(__file__).resolve().parents[1]
SPRITES = ROOT / "assets" / "sprites"
SCENES = ROOT / "assets" / "scenes"

CHARACTERS = {
    "student": "角色模型/学生.jpg",
    "ice_mage": "角色模型/冰法师.jpg",
    "paladin": "角色模型/圣骑士.jpg",
    "blesser": "角色模型/祈福者.jpg",
    "blood_warrior": "角色模型/血战士.jpg",
    "magician": "角色模型/魔术师.jpg",
}

ENEMIES = {
    "gym_sprinter": "敌人建模/01_废弃体育馆/懈怠的短跑者.jpg",
    "gym_shot_putter": "敌人建模/01_废弃体育馆/驼背的铅球手.jpg",
    "gym_gymnast": "敌人建模/01_废弃体育馆/体操幽灵.jpg",
    "library_scribble_spirit": "敌人建模/02_倒悬图书馆/涂鸦书灵.jpg",
    "library_catalog_golem": "敌人建模/02_倒悬图书馆/目录魔像.jpg",
    "library_banned_librarian": "敌人建模/02_倒悬图书馆/禁书管理员.jpg",
    "theater_clown": "敌人建模/03_镜面戏剧社/微笑小丑.jpg",
    "theater_tragedy": "敌人建模/03_镜面戏剧社/悲情女主角.jpg",
    "theater_mirror_attendant": "敌人建模/03_镜面戏剧社/镜面侍从.jpg",
    "lab_acid_slime": "敌人建模/04_失序化学实验室/酸液史莱姆.jpg",
    "lab_beaker": "敌人建模/04_失序化学实验室/爆炸烧杯怪.jpg",
    "lab_tube": "敌人建模/04_失序化学实验室/剧毒试管精.jpg",
    "divination_tarot": "敌人建模/05_占卜社旧址/塔罗士兵.jpg",
    "divination_clock_ghost": "敌人建模/05_占卜社旧址/时针幽灵.jpg",
    "office_iron_hand": "敌人建模/06_校长室前厅/教导处铁腕.jpg",
    "office_discipline_inspector": "敌人建模/06_校长室前厅/纪律巡查使.jpg",
    "elite_book_guardian": "敌人建模/07_精英敌人/扉页守护者.jpg",
    "elite_winged_runner": "敌人建模/07_精英敌人/折翼的百米王者.jpg",
    "elite_faceless_dancer": "敌人建模/07_精英敌人/无面舞者.jpg",
    "elite_clock_prisoner": "敌人建模/07_精英敌人/时计塔的囚徒.jpg",
    "elite_mutant": "敌人建模/07_精英敌人/畸变融合体.jpg",
    "elite_rust_regret": "敌人建模/07_精英敌人/铁锈执念·陈暮.jpg",
    "boss_principal": "敌人建模/08_最终BOSS/伪典校长·零.jpg",
}

SCENE_SOURCES = {
    "gym": "场景建模/室内体育馆.jpg",
    "library": "场景建模/校园图书馆.jpg",
    "classroom_dusk": "场景建模/教学楼教室黄昏.jpg",
    "abandoned_classroom": "场景建模/废弃教室.jpg",
    "equipment_room": "场景建模/体育器材室.jpg",
    "corridor": "场景建模/校园教学楼走廊.jpg",
    "playground": "场景建模/校园操场黄昏.jpg",
}


def background_mask(image: Image.Image) -> Image.Image:
    """Return an alpha mask after removing the connected light background."""
    pixels = np.asarray(image.convert("RGB"), dtype=np.int16)
    height, width, _ = pixels.shape
    edge_samples = np.concatenate((pixels[0], pixels[-1], pixels[1:-1, 0], pixels[1:-1, -1]))
    background = np.median(edge_samples, axis=0)
    distance = np.linalg.norm(pixels - background, axis=2)
    candidates = (distance < 62).astype(np.uint8)
    _, labels = cv2.connectedComponents(candidates, connectivity=4)
    edge_labels = np.concatenate((labels[0], labels[-1], labels[1:-1, 0], labels[1:-1, -1]))
    background_labels = np.unique(edge_labels[edge_labels > 0])
    connected_background = np.isin(labels, background_labels)
    alpha_array = (~connected_background * 255).astype(np.uint8)
    # Model art is framed with safe padding. Clear the outer rim so JPEG noise
    # cannot leave a visible backdrop strip when a scene is behind the sprite.
    alpha_array[:6, :] = 0
    alpha_array[-6:, :] = 0
    alpha_array[:, :6] = 0
    alpha_array[:, -6:] = 0
    alpha = Image.fromarray(alpha_array, "L")

    # A very small feather prevents jagged JPEG contours without eroding detail.
    return alpha.filter(ImageFilter.GaussianBlur(0.55))


def build_sprite(alias: str, source: str) -> None:
    image = Image.open(ROOT / source).convert("RGBA")
    image.putalpha(background_mask(image))
    target = SPRITES / f"{alias}.png"
    image.save(target, "PNG", optimize=True)


def validate_sprite(path: Path) -> None:
    image = Image.open(path).convert("RGBA")
    alpha = image.getchannel("A")
    corners = [alpha.getpixel(point) for point in ((0, 0), (image.width - 1, 0), (0, image.height - 1), (image.width - 1, image.height - 1))]
    coverage = np.count_nonzero(np.asarray(alpha)) / (image.width * image.height)
    if any(corners) or not 0.03 < coverage < 0.85:
        raise RuntimeError(f"Unexpected cutout mask for {path.name}: corners={corners}, coverage={coverage:.3f}")


def main() -> None:
    SPRITES.mkdir(parents=True, exist_ok=True)
    SCENES.mkdir(parents=True, exist_ok=True)
    for alias, source in {**CHARACTERS, **ENEMIES}.items():
        target = SPRITES / f"{alias}.png"
        if target.exists() and target.stat().st_size > 1024:
            validate_sprite(target)
            continue
        build_sprite(alias, source)
        validate_sprite(target)
    for alias, source in SCENE_SOURCES.items():
        copy2(ROOT / source, SCENES / f"{alias}.jpg")
    print(f"Built {len(CHARACTERS) + len(ENEMIES)} transparent sprites and {len(SCENE_SOURCES)} scenes.")


if __name__ == "__main__":
    main()
