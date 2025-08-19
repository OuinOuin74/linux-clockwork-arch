# Custom Linux Kernel for ClockworkPi uConsole (Arch Linux ARM)

This repository provides a **custom Linux kernel (branch 6.16)** for the **ClockworkPi uConsole**, based on the official [Raspberry Pi Linux kernel](https://github.com/raspberrypi/linux) and on the great work by [Rex](https://github.com/ak-rex/ClockworkPi-linux), from which I borrowed and adapted most of the work.  
🙏 A very warm thank you to Rex for his excellent repository, which served as the foundation of this project.

## Overview

- **Kernel version**: 6.16 (customized)
- **Base**: Raspberry Pi upstream kernel
- **Additional work**: patches, drivers, and device tree overlays for ClockworkPi uConsole
- **Target**: [Arch Linux ARM](https://archlinuxarm.org/platforms/armv8/broadcom/raspberry-pi-4)  
- **Platform**: Raspberry Pi CM4 inside the uConsole

Everything works as expected on the ClockworkPi uConsole with Arch Linux ARM.

## Differences from the Raspberry Pi kernel

Compared to the vanilla Raspberry Pi 6.16 kernel, this custom build includes:

- **GPU driver**: additional driver required for the ClockworkPi display  
- **Backlight driver**: support for uConsole screen brightness control  
- **Extra device tree overlays**: to handle uConsole-specific hardware configurations  
- **Battery management patch**: adapted AXP20x battery driver for proper power reporting  
- **Modified `defconfig`**:  
  - Landlock enabled (required by `pacman` on Arch Linux)  
  - Other small adjustments for uConsole compatibility  

## Installation

⚠️ **Important note**: Arch Linux ARM cannot boot on the uConsole “as is”.  
You must first install the kernel and supporting files **from a chroot environment (with `qemu-aarch64-static`)** or by **manually copying the package files** into the Arch Linux ARM rootfs **before the first boot on the uConsole**.  

There are two possible methods:

1. **Using a chroot with QEMU static**  
   Prepare and install the package inside an Arch Linux ARM rootfs using a `chroot` with `qemu-aarch64-static`.

2. **Manual installation**  
   Build the package on another machine and copy the files (`/boot`, kernel modules, etc.) directly into the target Arch Linux ARM system.

> Official Arch Linux ARM installation instructions for Raspberry Pi CM4:  
> [Arch Linux ARM – Raspberry Pi 4](https://archlinuxarm.org/platforms/armv8/broadcom/raspberry-pi-4)

## Features

- Based on the Raspberry Pi kernel branch 6.16.y
- Includes ClockworkPi-specific patches and drivers  
- Device tree overlays adapted for the uConsole hardware  
- Packaged as a standard Arch Linux ARM PKGBUILD for easy build & install  

## Credits

- [Raspberry Pi Foundation](https://github.com/raspberrypi/linux) for their kernel sources  
- [Rex](https://github.com/ak-rex/ClockworkPi-linux) for his amazing groundwork, which this repository heavily builds upon ❤️  
- Arch Linux ARM team for the userspace and distribution support 
