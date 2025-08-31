# Custom Linux Kernel for ClockworkPi uConsole (Arch Linux ARM)

This repository provides a **custom Linux kernel (branch 6.16)** for the **ClockworkPi uConsole**, based on the official [Raspberry Pi Linux kernel](https://github.com/raspberrypi/linux) and on the great work by [Rex](https://github.com/ak-rex/ClockworkPi-linux), from which I borrowed and adapted most of the work.
🙏 A very warm thank you to Rex for his excellent repository, which served as the foundation of this project.

## Overview

- **Kernel version**: 6.16 (customized)
- **Base**: Raspberry Pi upstream kernel
- **Additional work**: patches, drivers, and device tree overlays for ClockworkPi uConsole
- **Target**: [Arch Linux ARM](https://archlinuxarm.org/platforms/armv8/broadcom/raspberry-pi-4)
- **Platforms**: Raspberry Pi CM4 **and CM5** inside the uConsole

## Features

- Based on the Raspberry Pi kernel branch 6.16.y
- Includes ClockworkPi-specific patches and drivers
- Device tree overlays adapted for the uConsole hardware
- Automatic **speaker cutoff when headphones are plugged in**
- Ability to **specify battery design capacity (µAh) and nominal voltage (µV)** in `config.txt`
  - Example: `dtoverlay=clockworkpi-uconsole-cm4(-cm5),batt_uah=7000000,vnom_uv=3700000`  
- Ability to **overclock the SD card to 100 MHz** via overlay (CM4 only)
  - Example: `dtparam=sd_overclock=100`
- Packaged as a standard Arch Linux ARM PKGBUILD for easy build & install
- Provided in **two flavours**: one build for CM4 and one build for CM5 (download the correct package depending on your uConsole module)

## Differences from the Raspberry Pi kernel

Compared to the vanilla Raspberry Pi 6.16 kernel, this custom build includes:

- **GPU driver**: additional driver required for the ClockworkPi display
- **Backlight driver**: support for uConsole screen brightness control
- **Extra device tree overlays**: to handle uConsole-specific hardware configurations
- **Battery management patch**: adapted AXP20x battery driver for proper power reporting
- **Headphone amplifier switch driver**: handles automatic speaker cutoff when headphones are plugged in
- **Modified `defconfig`**:
  - Landlock enabled (required by `pacman` on Arch Linux)
  - F2FS compression enabled (lz4 + zstd)
  - Other small adjustments for uConsole compatibility

## Installation

### 1) Install Arch Linux ARM (base system)
Start by preparing a standard Arch Linux ARM system for Raspberry Pi CM4:
- Follow the official guide: **[Arch Linux ARM – Raspberry Pi 4](https://archlinuxarm.org/platforms/armv8/broadcom/raspberry-pi-4)**
- Write the root filesystem and boot files to your uConsole’s storage (e.g., microSD).
- Do **not** boot the uConsole yet.

> Arch Linux ARM cannot boot on the uConsole “as is” with the stock kernel.
> You must install this custom kernel **before** the first boot.

### 2) Install this kernel (pick ONE method)

#### A) In a chroot using QEMU (recommended)
Set up a chroot environment on your x86_64 host and install the kernel package directly inside the target rootfs.
See the ArchWiki for detailed instructions:
👉 [Chrooting into arm/arm64 environment from x86_64](https://wiki.archlinux.org/title/QEMU#Chrooting_into_arm/arm64_environment_from_x86_64)

**Once inside the chroot :**

- Remove default Arch ARM kernel/boot packages (if present):
```bash
pacman -Rns linux-aarch64 uboot-raspberrypi
```
> This project boots via the Raspberry Pi firmware + custom kernel;
> the stock `linux-aarch64` and `uboot-raspberrypi` are not needed and can
> conflict with files in `/boot` or initramfs hooks.

- Then install the packages normally:
```bash
pacman -U /path/to/linux-uconsole-<ver>-<rel>-aarch64.pkg.tar.zst \
         /path/to/linux-uconsole-headers-<ver>-<rel>-aarch64.pkg.tar.zst
```

- Edit `/boot/config.txt` and uncomment (or add) the initramfs line:
```ini
initramfs initramfs-linux-rpi-clockwork-cm4.img followkernel
```
(or `...-cm5.img` depending on your module)

#### B) “Raw copy” (manual file copy)

If you cannot chroot, you can manually copy the package contents into the target rootfs:

Extract the package(s) on your PC:
```bash
bsdtar -xvf linux-uconsole-<ver>-<rel>-aarch64.pkg.tar.zst -C /mnt/alarma64
```

This places `/boot/*` (kernel, overlays, dtbs) and `/usr/lib/modules/*` in the target.
With this method, no initramfs is generated yet.

⚠️ **Important step for manual installs**:
You must set the correct `PARTUUID` in `/boot/cmdline.txt` before booting.

Default line:
```
root=PARTUUID=<root_PARTUUID> rw rootwait console=serial0,115200 console=tty3 usbhid.mousepoll=8 audit=0 fbcon=rotate:1 psi=1
```

Update `<root_PARTUUID>` with the actual PARTUUID of your root partition:
```bash
blkid -s PARTUUID -o value /dev/mmcblk0p2
```

Replace `/dev/mmcblk0p2` with the actual root partition device (e.g., eMMC or SD).

### 3) Boot firmware configuration

Use the `config.txt` provided in the package (located under `/boot`).
Typical content for the uConsole is:

```ini
# ============================================================
# ClockworkPi uConsole - boot configuration
# ============================================================

[all]
# Common settings (both CM4/CM5)
dtparam=spi=on
enable_uart=1
ignore_lcd=1
max_framebuffers=2
disable_overscan=1
dtparam=audio=on
otg_mode=1
dtparam=ant2

# ============================================================
# CM4 (Compute Module 4)
# ============================================================
[cm4]
# uConsole overlay for CM4 (select one)
dtoverlay=clockworkpi-uconsole-cm4
# Set battery capacity: (see notes below)
#dtoverlay=clockworkpi-uconsole-cm4,batt_uah=7000000
#dtoverlay=clockworkpi-uconsole-cm4,batt_uah=7000000,vnom_uv=3700000

# KMS
dtoverlay=vc4-kms-v3d-pi4,cma-384

# Audio
dtoverlay=audremap,pins_12_13

# Kernel + initramfs (cm4)
kernel=kernel8-cm4.img
#initramfs initramfs-linux-rpi-clockwork-cm4.img followkernel

# The firmware cannot auto-detect 64-bit mode. We MUST force it
arm_64bit=1

# Overclock SD (optional)
#dtparam=sd_overclock=100

# ============================================================
# CM5 (Compute Module 5)
# ============================================================
[cm5]
# uConsole overlay for CM5 (select one)
dtoverlay=clockworkpi-uconsole-cm5
# Set battery capacity: (see notes below)
#dtoverlay=clockworkpi-uconsole-cm5,batt_uah=7000000
#dtoverlay=clockworkpi-uconsole-cm5,batt_uah=7000000,vnom_uv=3700000

# KMS
dtoverlay=vc4-kms-v3d-pi5,cma-384

# Audio
dtoverlay=audremap-pi5,pins_12_13

# Kernel + initramfs (cm5)
kernel=kernel8-cm5.img
#initramfs initramfs-linux-rpi-clockwork-cm5.img followkernel
```

If you installed via chroot, pacman generates the initramfs.
In that case, uncomment the line for your module (CM4 or CM5).

If you installed via raw copy, no initramfs exists yet.
Leave the line commented (`#initramfs ...`) for the first boot.

### 4) First boot and post-boot fix (only for raw copy installs)

If you used the raw copy method:

Boot the uConsole (the kernel should start without initramfs).

- On the first boot, remove default Arch ARM kernel/boot packages (if present):
```bash
sudo pacman -Rns linux-aarch64 uboot-raspberrypi
```

- Then install the custom kernel package properly with pacman to generate the initramfs:
```bash
sudo pacman -U /path/to/linux-uconsole-<ver>-<rel>-aarch64.pkg.tar.zst
```

- Edit `/boot/config.txt` and uncomment (or add) the initramfs line for your module:
```ini
initramfs initramfs-linux-rpi-clockwork-cm4.img followkernel
```
or
```ini
initramfs initramfs-linux-rpi-clockwork-cm5.img followkernel
```

Reboot.

## Credits

- [Raspberry Pi Foundation](https://github.com/raspberrypi/linux) for their kernel sources  
- [Rex](https://github.com/ak-rex/ClockworkPi-linux) for his amazing groundwork, which this repository heavily builds upon ❤️  
- [PotatoMania](https://github.com/PotatoMania/uconsole-cm3) for the uConsole audio driver and automatic speaker cutoff 🙏  
- Arch Linux ARM team for the userspace and distribution support  
