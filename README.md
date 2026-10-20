# ESK Reborn Kernel — xaga family (MT6895)

Custom kernel for the **POCO X4 GT / Redmi Note 11T Pro / K50i family (xaga)** —
Dimensity 8100 (MT6895), based on the Google **android12-5.10 GKI** common kernel
with MediaTek/Xiaomi vendor code.

| | |
|---|---|
| **Stable sublevel** | Linux **5.10.272** (tracked upstream) |
| **Branches** | `17.0` (Android 17 baseline) · `16.2-rebase` (Android 16.x) — every functional change and stable merge lands on **both** |
| **ROM support** | Android 16 / 17 custom ROMs (Infinity X, …) |
| **Releases** | [ESK-Kernel-Reborn-Releases](https://github.com/ENGRahsepar/ESK-Kernel-Reborn-Releases) — AnyKernel3 zips + INSTALL guide |
| **Variants** | `KSU-SUSFS` (ReSukiSU + SUSFS 2.2.0) · `KSU-SUSFS-LXC` (+ LXC containers) · `VNL` (vanilla) |

## Highlights

- **BORE scheduler** — 5.10 GKI port (`firelzrd/bore-scheduler` via Templar's working port)
- **ESK governor** — EAS-replacement CPU governor with `gaming_mode` + `prime_gaming_floor_pct` tunables on policy7
- **Fast-switch cpufreq fixes** — dedicated MTK 3-domain handling; A55 / A78 / Prime DVFS domains
- **Bypass charging** — VBUS powers the system, battery idles (see releases README for usage)
- **ReSukiSU + SUSFS 2.2.0**, optional **LXC containers**, NoMount, BBRplus

## Documentation

- **[DEVELOPMENT.md](DEVELOPMENT.md)** — full developer/handoff document: build infra, config merge order, patch sets, debug workflows, push policy
- **[INSTALL.md](https://github.com/ENGRahsepar/ESK-Kernel-Reborn-Releases/blob/main/INSTALL.md)** — end-user installation (custom recovery + adb sideload)
- **Releases** — download the AnyKernel3 zip matching your variant

> `uname -r` carries the release tag, e.g. `5.10.272-android12-…-ESK-Reborn_V0.3.2`
> (`CONFIG_LOCALVERSION="-ESK-Reborn_V0.X"` in `arch/arm64/configs/vendor/xaga.config`, bumped per release).
