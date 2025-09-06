_commit=1ac513c9cd241360fafe57861d98b65a89cd5e5a
_srcname=linux-${_commit}
pkgver=6.16.4
pkgrel=4
pkgdesc='Linux'
url="https://github.com/raspberrypi/linux"
arch=(aarch64)
license=(GPL-2.0-only)
makedepends=(
  bc
  kmod
  inetutils
  util-linux
)
options=('!strip')
source=("linux-$pkgver-${_commit:0:10}.tar.gz::https://github.com/raspberrypi/linux/archive/${_commit}.tar.gz"
        cmdline.txt
        config.txt
        "patches.zip"
        "overlays.zip"
        "drivers.zip"
        linux.preset
)
md5sums=('bbf31a0d9ce5763c99f8dc900d5b9163'
         'e46eff7b6e8682b472459355e26ed645'
         '71ba2c5e8ef21ca87933a53984b34067'
         '1ba0b22f58e5663046c55b88dfe7d3fc'
         '4d00e976b844a652bd8ed75758128ca6'
         'f8374f44275c8ee9f44316c7a1fd8118'
         '5019cc9c926c7300ce46999beb3be5c8')

# --- BEGIN: model selector (cm4/cm5) -----------------------------------------
: "${MODEL:=cm4}"   # Default to cm4 if not set

# Root pkgbase without model suffix
_pkgbase_root="linux-rpi-clockwork"

case "$MODEL" in
  cm4)
    pkgbase="${_pkgbase_root}-cm4"
    _defconfig="bcm2711_defconfig"
    _localver="-cm4"
    ;;
  cm5)
    pkgbase="${_pkgbase_root}-cm5"
    _defconfig="bcm2712_defconfig"
    _localver="-cm5"
    ;;
  *)
    echo "Unknown MODEL='$MODEL' (use: cm4 | cm5)" >&2
    exit 2
    ;;
esac
# --- END: model selector -----------------------------------------------------

# setup vars
_kernel="kernel8-${MODEL}.img" KARCH=arm64 _image=Image

prepare() {
  cd "${srcdir}/${_srcname}"

  # apply patches
  for patch in "$srcdir"/patches/*.patch; do
    echo "Applying patch $patch"
    patch -p1 -i "$patch"
  done

  echo "==> Copying drivers..."
  cp -rv "${srcdir}"/drivers/* drivers/
  
  echo "==> Copying Clockworkpi overlays..."
  cp -v "$srcdir"/overlays/* arch/arm/boot/dts/overlays/

  make mrproper
  make "${_defconfig}"

  echo "Setting version..."
  echo "-$pkgrel" > localversion.10-pkgrel
  echo "${pkgbase#linux}" > localversion.20-pkgname

  make -s kernelrelease > version
  echo "Prepared $pkgbase version $(<version)"
}

build() {
  cd "${srcdir}/${_srcname}"

  make -j$(nproc) "$_image" modules dtbs
}

_package() {
  pkgdesc="Linux kernel and modules (RPi Foundation fork) custom for Clockwokpi uConsole"
  depends=(
    coreutils
    firmware-raspberrypi
    kmod
    'mkinitcpio>=0.7'
    raspberrypi-bootloader
  )
  optdepends=(
    'linux-firmware: firmware images needed for some devices'
    'wireless-regdb: to set the correct wireless channels of your country'
  )
  provides=(
    linux="${pkgver}"
    KSMBD-MODULE
    WIREGUARD-MODULE
  )
  conflicts=(
    linux
    linux-rpi-16k
    uboot-raspberrypi
    linux-rpi-clockwork-cm4
    linux-rpi-clockwork-cm5
  )
  install=linux-rpi-clockwork.install
  backup=(
    boot/config.txt
    boot/cmdline.txt
  )

  cd "${srcdir}/${_srcname}"
  local modulesdir="$pkgdir/usr/lib/modules/$(<version)"

  # Used by mkinitcpio to name the kernel
  echo "$pkgbase" | install -Dm644 /dev/stdin "$modulesdir/pkgbase"

  echo "Installing modules..."
  make INSTALL_MOD_PATH="$pkgdir/usr" INSTALL_MOD_STRIP=1 \
    DEPMOD=/doesnt/exist modules_install  # Suppress depmod

  # remove build link
  rm -f "$modulesdir"/build

  echo "Installing Arch ARM specific stuff..."
  mkdir -p "${pkgdir}"/boot
  make INSTALL_DTBS_PATH="${pkgdir}/boot" dtbs_install

  # drop hard-coded devicetree=foo.dtb in /boot/config.txt for
  # autodetected load of supported of models at boot
  find "${pkgdir}/boot/broadcom" -type f -print0 | xargs -0 mv -t "${pkgdir}/boot"
  rmdir "${pkgdir}/boot/broadcom"

  cp arch/$KARCH/boot/$_image "${pkgdir}/boot/$_kernel"
  cp arch/$KARCH/boot/dts/overlays/README "${pkgdir}/boot/overlays"
  install -m644 ../config.txt "${pkgdir}/boot"
  install -m644 ../cmdline.txt "${pkgdir}/boot"

  # CM5-only: brcmfmac workarounds via modprobe.d
  if [[ "$MODEL" == "cm5" ]]; then
    echo "options brcmfmac roamoff=1 feature_disable=0x282000" |
      install -Dm644 /dev/stdin "${pkgdir}/usr/lib/modprobe.d/99-brcmfmac-cm5.conf"
  fi
  
  # sed expression for following substitutions
  local _subst="
    s|%PKGBASE%|${pkgbase}|g
    s|%KERNVER%|$(<version)|g
  "

  # install mkinitcpio preset file
  sed "${_subst}" ../linux.preset |
    install -Dm644 /dev/stdin "${pkgdir}/etc/mkinitcpio.d/${pkgbase}.preset"

  # rather than use another hook (90-linux.hook) rely on mkinitcpio's 90-mkinitcpio-install.hook
  # which avoids a double run of mkinitcpio that can occur
  touch "${pkgdir}/usr/lib/modules/$(<version)/vmlinuz"
}

_package-headers() {
  pkgdesc="Headers and scripts for building modules for Linux kernel"
  provides=("linux-headers=${pkgver}")
  conflicts=('linux-headers' 'linux-rpi-16k-headers')

  cd ${_srcname}
  local builddir="$pkgdir/usr/lib/modules/$(<version)/build"

  echo "Installing build files..."
  install -Dt "$builddir" -m644 .config Makefile Module.symvers System.map \
    localversion.* version
  install -Dt "$builddir/kernel" -m644 kernel/Makefile
  install -Dt "$builddir/arch/$KARCH" -m644 "arch/$KARCH/Makefile"
  cp -t "$builddir" -a scripts

  # add xfs and shmem for aufs building
  mkdir -p "$builddir"/{fs/xfs,mm}

  echo "Installing headers..."
  cp -t "$builddir" -a include
  cp -t "$builddir/arch/$KARCH" -a "arch/$KARCH/include"
  install -Dt "$builddir/arch/$KARCH/kernel" -m644 "arch/$KARCH/kernel/asm-offsets.s"

  install -Dt "$builddir/drivers/md" -m644 drivers/md/*.h
  install -Dt "$builddir/net/mac80211" -m644 net/mac80211/*.h

  # https://bugs.archlinux.org/task/13146
  install -Dt "$builddir/drivers/media/i2c" -m644 drivers/media/i2c/msp3400-driver.h

  # https://bugs.archlinux.org/task/20402
  install -Dt "$builddir/drivers/media/usb/dvb-usb" -m644 drivers/media/usb/dvb-usb/*.h
  install -Dt "$builddir/drivers/media/dvb-frontends" -m644 drivers/media/dvb-frontends/*.h
  install -Dt "$builddir/drivers/media/tuners" -m644 drivers/media/tuners/*.h

  # https://bugs.archlinux.org/task/71392
  install -Dt "$builddir/drivers/iio/common/hid-sensors" -m644 drivers/iio/common/hid-sensors/*.h

  echo "Installing KConfig files..."
  find . -name 'Kconfig*' -exec install -Dm644 {} "$builddir/{}" \;

  echo "Removing unneeded architectures..."
  local _arch
  for _arch in "$builddir"/arch/*/; do
    if [[ $CARCH == "aarch64" ]]; then
      [[ $_arch = */"$KARCH"/ || $_arch == */arm/ ]] && continue
    else
      [[ $_arch = */"$KARCH"/ ]] && continue
    fi
    echo "Removing $(basename "$_arch")"
    rm -r "$_arch"
  done

  echo "Symlinking common aliases..."
  # https://archlinuxarm.org/forum/viewtopic.php?f=60&t=16354
  ln -sr arm "$builddir/arch/armv7h"
  ln -sr arm "$builddir/arch/armv7l"
  ln -sr arm64 "$builddir/arch/aarch64"

  echo "Removing documentation..."
  rm -r "$builddir/Documentation"

  echo "Removing broken symlinks..."
  find -L "$builddir" -type l -printf 'Removing %P\n' -delete

  echo "Removing loose objects..."
  find "$builddir" -type f -name '*.o' -printf 'Removing %P\n' -delete

  echo "Stripping build tools..."
  local file
  while read -rd '' file; do
    case "$(file -Sib "$file")" in
      application/x-sharedlib\;*)      # Libraries (.so)
        strip -v $STRIP_SHARED "$file" ;;
      application/x-archive\;*)        # Libraries (.a)
        strip -v $STRIP_STATIC "$file" ;;
      application/x-executable\;*)     # Binaries
        strip -v $STRIP_BINARIES "$file" ;;
      application/x-pie-executable\;*) # Relocatable binaries
        strip -v $STRIP_SHARED "$file" ;;
    esac
  done < <(find "$builddir" -type f -perm -u+x -print0)

  echo "Adding symlink..."
  mkdir -p "$pkgdir/usr/src"
  ln -sr "$builddir" "$pkgdir/usr/src/$pkgbase"
}

pkgname=("${pkgbase}" "${pkgbase}-headers")
for _p in ${pkgname[@]}; do
  eval "package_${_p}() {
    _package${_p#${pkgbase}}
  }"
done

# vim:set ts=8 sts=2 sw=2 et:
