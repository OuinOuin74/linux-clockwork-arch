# Custom Linux Kernel for ClockworkPi uConsole (Arch Linux ARM)

This repository provides a **custom Linux kernel (branch 6.16)** for the **ClockworkPi uConsole**, based on the official [Raspberry Pi Linux kernel](https://github.com/raspberrypi/linux) and on the great work by [Rex](https://github.com/ak-rex/ClockworkPi-linux), from which I borrowed and adapted most of the work.  
🙏 A very warm thank you to Rex for his excellent repository, which served as the foundation of this project.

## Overview

- **Kernel version**: 6.16 (customized)  
- **Base**: Raspberry Pi upstream kernel  
- **Additional work**: patches, drivers, and device tree overlays for ClockworkPi uConsole  
- **Target**: [Arch Linux ARM](https://archlinuxarm.org/platforms/armv8/broadcom/raspberry-pi-4)  
- **Platform**: Raspberry Pi CM4 inside the uConsole  

## Features

- Based on the Raspberry Pi kernel branch 6.16.y  
- Includes ClockworkPi-specific patches and drivers  
- Device tree overlays adapted for the uConsole hardware  
- Automatic **speaker cutoff when headphones are plugged in**  
- Ability to **specify battery design capacity (µAh) and nominal voltage (µV)** in `config.txt`  
  - Example: `dtoverlay=clockworkpi-uconsole,batt_uah=7000000,vnom_uv=3700000`  
- Ability to **overclock the SD card to 100 MHz** via overlay  
  - Example: `dtparam=sd_overclock=100`  
- Packaged as a standard Arch Linux ARM PKGBUILD for easy build & install

## Differences from the Raspberry Pi kernel

Compared to the vanilla Raspberry Pi 6.16 kernel, this custom build includes:

- **GPU driver**: additional driver required for the ClockworkPi display  
- **Backlight driver**: support for uConsole screen brightness control  
- **Extra device tree overlays**: to handle uConsole-specific hardware configurations  
- **Battery management patch**: adapted AXP20x battery driver for proper power reporting  
- **Headphone amplifier switch driver**: handles automatic speaker cutoff when headphones are plugged in  
- **Modified `defconfig`**:  
  - Landlock enabled (required by `pacman` on Arch Linux)  
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
initramfs initramfs-linux-rpi-clockwork.img followkernel
```

#### B) “Raw copy” (manual file copy)

If you cannot chroot, you can manually copy the package contents into the target rootfs:

Extract the package(s) on your PC:
```bash
bsdtar -xvf linux-uconsole-<ver>-<rel>-aarch64.pkg.tar.zst -C /mnt/alarma64
```

This places `/boot/*` (kernel, overlays, dtbs) and `/usr/lib/modules/*` in the target.  
With this method, no initramfs is generated yet.

### 3) Boot firmware configuration

Use the `config.txt` provided in the package (located under `/boot`).  
Typical content for the uConsole is:

```ini
...
otg_mode=1
dtparam=ant2

# Load initramfs built by mkinitcpio
#initramfs initramfs-linux-rpi-clockwork.img followkernel
...
```

If you installed via chroot, pacman generates the initramfs.  
In that case, uncomment the line:
```ini
initramfs initramfs-linux-rpi-clockwork.img followkernel
```

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

- Edit `/boot/config.txt` and uncomment (or add) the initramfs line:
```ini
initramfs initramfs-linux-rpi-clockwork.img followkernel
```

Reboot.

## Credits

- [Raspberry Pi Foundation](https://github.com/raspberrypi/linux) for their kernel sources  
- [Rex](https://github.com/ak-rex/ClockworkPi-linux) for his amazing groundwork, which this repository heavily builds upon ❤️  
- [PotatoMania](https://github.com/PotatoMania/uconsole-cm3) for the uConsole audio driver and automatic speaker cutoff 🙏  
- Arch Linux ARM team for the userspace and distribution support  
