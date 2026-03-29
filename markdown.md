# UE5-GAS-Multiplayer-Roguelike 


本项目是一款基于 **Unreal Engine 5** 和 **Gameplay Ability System (GAS)** 开发的 3D 四人联机动作 Roguelike (割草) 游戏。采用 Client-Server 权威网络架构，核心目标是在高并发弹幕、海量 AI 同屏的多人联机场景下，探索 C++ 底层性能优化、网络同步策略以及极致的 ACT 战斗打击感。


## 🌟 核心技术与架构 (Key Features)

### 1. 极致性能优化与内存管理 (Performance)
针对“割草”类游戏海量实体引发的 CPU 满载与 GC 卡顿，在底层进行了深度重构：
* **经验球统一结构体管理器 (`AExpOrbManager`)**：
  * **重构逻辑**：彻底废弃将经验球作为独立 Actor 生成并挂载 Tick 的低效方案。
  * **底层实现**：在服务器端引入统一管理器，使用结构体数组 (`TArray<FExpOrbData>`) 接管全图掉落物。在统一的 C++ Tick 中进行距离平方检测与磁吸插值运算，实现 O(N) 级别的高效判定。
  * **网络优化**：通过 `NetMode` 隔离网络性能，并使用 `Reliable` RPC 确保掉落物拾取核心数据的绝对同步。
* **高并发对象池 (Object Pool)**：
  * **实体复用**：针对怪物 (`EnemyPoolManager`)、弹幕 (`AEclipseFireball`) 和伤害飘字 (`BP_DamageActor`)，手写 C++ 唤醒/休眠 (`WakeUp/GoToSleep`) 机制，彻底废弃高频的 `Spawn/Destroy` 操作。
  * **安全熔断**：为对象池新增同屏数量硬上限锁，防止极端情况下的内存溢出。

### 2. GAS 战斗系统与网络同步 (Combat & Sync)
* **复杂技能管线定制**：
  * 完整打通法师专属技能（如星蚀、奥术涡流、追踪星弹）的 GAS 链路，包含多阶段爆发、持续伤害 (DoT) 与终结爆炸。
  * **弹道数学制导**：在 `AEclipseFireball` 中，底层利用极坐标系与向心加速度计算，实现弹幕的星环散布与螺旋轨迹，并加入终点悬停滞留机制 (`StopAndLinger`)。
* **网络多播调优**：
  * 针对满级 12 发星蚀火球齐射造成的帧率暴跌与网络拥堵，剥离蓝图 Tick，将释放表现与物理重置降级为 `Unreliable` 多播，仅在服务器保留权威判定，实现零卡顿输出。
* **状态过滤与拦截**：
  * 在 `BasicAttributeSet` 中利用 Tag 机制（如 `State.GameOver`）设置“绝对防御阀门”，精准拦截结算期间的额外伤害；通过区分 `Damage.DoT` 标签，防止怪物因持续毒伤触发鬼畜后退。

### 3. ACT 打击感与视听反馈 (Game Feel)
* **物理反馈**：废弃生硬的材质闪白，集成基于 `LaunchCharacter` 的带方向物理击退、受击顿帧 (Hit Stop / Custom Time Dilation) 及屏幕震动 (Camera Shake)。
* **视觉渲染**：编写动态材质 `M_UI_CooldownMask` 处理扇形冷却遮罩；实装预乘透明度矩形遮罩的边缘渗血暗角滤镜 (`M_BloodVignette`)。
* **音频性能保护**：实装 3D 空间发声的沉重受击音效。通过引入并发限制 (Sound Concurrency) 与随机音高 (Random Pitch) 机制，彻底解决多段并发伤害同时命中时的爆音与吞音问题。

### 4. 联机游戏系统与智能 AI (Systems & AI)
* **局内 0 带宽浪费升级**：重构 `UUpgradeManagerComponent`，通过双参数 (`SkillID`, `NewLevel`) 的本地多播委托 (`FOnSkillUpgradedSignature`)，配合 TMap 对象池，实现服务器极简同步与客户端 UI 动态装配。
* **高性能索敌 AI**：
  * 废弃蓝图全局扫描，使用 C++ Timer 配合控制器迭代器重构 AI 雷达。
  * 独立法球智能索敌算法：目标死亡后，飞行物将在自身周围动态寻找新目标，彻底消除伤害溢出 (Overkill)。
  * 结合 Navigation Mesh 与 RVO 避让系统，解决同屏怪物海的物理死锁问题。
* **Session 联机大厅**：接入 Steam 接口，实现好友邀请、角色选择与 `ServerTravel` 无缝流转。

## 📂 核心代码目录 (Source Structure)
* `Source/MPRoguelike/GameplayAbilitySystem/` 
  * 包含自定义的 `AttributeSet` (伤害拦截与广播)、`CharacterBase` (暴露 GAS 数据至蓝图) 及 `MPPassiveAbilityBase` (被动技能基类)。
* `Source/MPRoguelike/Managers/` 
  * 包含所有底层性能优化核心模块，如 `AExpOrbManager` (经验球结构体管理)、`EnemyPoolManager` (怪物池)、`UpgradeManagerComponent` (升级与抽卡同步机制)。

## ⚙️ 环境依赖 (Requirements)
* **Unreal Engine**: 5.7
* **IDE**: Visual Studio 2022 / JetBrains Rider
* **插件依赖**: Gameplay Ability System, Online Subsystem Steam, Advanced Sessions