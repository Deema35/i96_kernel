## i96_kernel
Kernel files for OrangePI i96

You need add this files to kernell directory. 

I use gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf for compilation.

After that:

```
make ARCH=arm CROSS_COMPILE=/"path to toolchain"/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf- defconfig
make ARCH=arm CROSS_COMPILE=/"path to toolchain"/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf- menuconfig
```

Set options:

1. System type-->RDA Micro SoCs
2. Device drivers-->Character devices-->Enable TTY-->Serial Drivers-->RDA Micro serial port support-->Console on RDA Micr serial port
3. Device drivers-->GPIO Support-->Memory Mapped GPIO drivers-->RDA Micro GPIO controller support
4. Device drivers-->RDA support  (You need set all options in "RDA support" submenu)


make ARCH=arm CROSS_COMPILE=/"path to toolchain"/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf- -j16

In releases you can find Image compilled on 6.5.1 kernell and Busybox 1.36.1. Login:root password:root

#Wi-Fi

If you want wi-fi you need create wi-fi acsses point with ssid="i96" password:123456789. For change ap you need edit file etc/wpa_supplicant.conf, use for it command:

`wpa_passphrase i96 123456789`

Or you can modify Image from PC:

```
sudo mount -o loop,offset=2097152 NewImg.img /tmp/tmp (for modify boot partition)
sudo mount -o loop,offset=54525952 NewImg.img /tmp/tmp (for modify root system partition)
```

[Habrahabr.ru](https:)