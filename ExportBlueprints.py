# UE5 蓝图导出脚本
# 使用方法：在 UE5 编辑器中打开项目，然后在输出窗口运行此脚本
# 或者保存为 .py 文件，通过命令行运行

import unreal

# 导出目录
export_dir = "D:/GraduationProject/MPRoguelike/ExportedBlueprints/"
unreal.SystemLibrary.create_directory(export_dir)

# 要导出的蓝图路径列表
blueprint_paths = [
    "/Game/Blueprints/Characters/BP_CharacterPlayer",
    "/Game/Blueprints/Characters/BP_EnemyBase",
    "/Game/Blueprints/Characters/BP_Mage",
    "/Game/Blueprints/Characters/BP_Warrior",
    "/Game/Blueprints/Core/BP_GameInstance",
    "/Game/Blueprints/Core/BP_GameplayGM",
    "/Game/Blueprints/Core/BP_MyGameState",
    "/Game/Blueprints/Lobby/BP_LobbyGameMode",
    "/Game/Blueprints/Lobby/BP_LobbyPawn",
    "/Game/Blueprints/Lobby/BP_PC_Lobby",
    "/Game/Blueprints/Lobby/BP_PlayerSlot",
    "/Game/Blueprints/Projectiles/BP_Fireball",
    "/Game/Blueprints/Projectiles/BP_TrackingStarMissile",
    "/Game/Blueprints/Systems/BP_EnemySpawner",
    "/Game/Blueprints/Systems/BP_ExpOrb",
    "/Game/Blueprints/UI/BP_DamageActor",
]

# 导出每个蓝图
for bp_path in blueprint_paths:
    try:
        # 加载对象
        obj = unreal.load_object(None, bp_path)
        if obj:
            # 导出为文本
            export_path = export_dir + bp_path.split("/")[-1] + ".txt"
            unreal.SystemLibrary.write_file(export_path, str(obj))
            print(f"Exported: {bp_path} -> {export_path}")
        else:
            print(f"Not found: {bp_path}")
    except Exception as e:
        print(f"Error exporting {bp_path}: {e}")

print("\n=== Export Complete ===")
print(f"Export directory: {export_dir}")
