# Linux Kernel Module - Buildroot Package

A Linux kernel module project for Raspberry Pi 5, developed in C using Buildroot.

This is an ongoing project where I gradually extend the kernel module with new features and test them on the Raspberry Pi.

The module is maintained in GitHub, and Buildroot uses a specific Git commit to download and build the selected version.

## 1. Buildroot Package

I created a Buildroot package for my Linux kernel module.

The package includes:

- `Config.in` - Package configuration.
- `mymodule.mk` - Buildroot package definition.
- `Makefile` - Kernel module build instructions.
- `mymodule.c` - Kernel module source code.

I enabled the package in Buildroot and successfully compiled the module for Raspberry Pi 5.

## 2. Character Device Registration

I extended the module to register a character device.

- Used `alloc_chrdev_region()` to dynamically allocate a device number.
- Used `cdev_init()` and `cdev_add()` to register the character device.
- Used `cdev_del()` and `unregister_chrdev_region()` for cleanup.

I tested the module on the Raspberry Pi using `insmod`, `lsmod`, `rmmod`, and `dmesg`.

The device was assigned major number 240 during testing.

## 3. Automatic Device File Creation

I added automatic device file creation.

- Used `class_create()` to create a device class.
- Used `device_create()` to create `/dev/mymodule`.
- Used `device_destroy()` and `class_destroy()` when removing the module.
- Added error handling for device registration failures.

I verified that loading the module created:

- `/dev/mymodule`
- `/sys/class/mymodule`

After removing the module, I verified that both were removed successfully.

## 4. Device Open and Release

I implemented the `open` and `release` functions using `file_operations`.

- Used `container_of()` to access the module's private data structure.
- Used `file->private_data` to store the device data.
- Added counters for total opens, total releases, and active connections.
- Used `printk()` to display the counters in the kernel log.

I created a user-space test program to open and close the device and verified the results using `dmesg`.

## 5. Circular Buffer - Read and Write

I added a circular buffer inside the module's private data structure.

The buffer contains 128 bytes and uses two indexes:

- `head` - Position where the next byte is written.
- `tail` - Position where the next byte is read.

I implemented:

- `mymod_write()` using `copy_from_user()`.
- `mymod_read()` using `copy_to_user()`.
- Circular indexing using the modulo operator (`% BUFFER_SIZE`).
- Detection of an empty buffer when `head == tail`.
- Detection of a full buffer when the next head position equals the tail.

One byte is reserved to distinguish a full buffer from an empty buffer, so the maximum usable capacity is 127 bytes.

Both functions return the actual number of bytes transferred, which may be smaller than the requested amount.

## 6. Circular Buffer Testing

I created a user-space test program named `test_circular_buffer.c`.

The program allows the user to:

- Enter text to write to the device.
- Choose the number of bytes to read.
- Display the number of bytes actually written and read.
- Display the data returned from the device.

I tested writing 10 bytes and requesting 12 bytes for reading.

The module correctly returned 10 bytes.

I also tested writing and reading 127 bytes and verified that the buffer indexes wrapped around correctly.

During testing, I used kernel log messages to monitor the `head` and `tail` positions.

## 7. Module Cleanup

I tested removing the module using:

`rmmod mymodule`

I verified that:

- The module printed "Bye Bye World".
- The device file `/dev/mymodule` was removed.
- The device class `/sys/class/mymodule` was removed.
- The module no longer appeared in `lsmod`.

## 8. GitHub Version Management

I moved the ongoing project into a separate GitHub repository.

Instead of building the module from a local source directory, Buildroot now downloads the source code from GitHub using a specific commit hash.

The package uses:

```makefile
MYMODULE_VERSION = b01673d8637a35d9a20dc0efe24d158848e77eaa
MYMODULE_SITE = $(call github,LavyCo,trivmod-buildroot-package,$(MYMODULE_VERSION))

$(eval $(kernel-module))
$(eval $(generic-package))
```

I tested the new configuration using:

```bash
make mymodule-dirclean
make mymodule
```

The build completed successfully and generated `mymodule.ko`.

For future versions, I will create a new Git commit and update `MYMODULE_VERSION` in Buildroot.

## 9. Project Structure

```text
trivmod-buildroot-package/
├── Makefile
├── mymodule.c
└── package/
    └── mymodule/
        ├── Config.in
        └── mymodule.mk
```

The separate repository for kernel exercises and user-space tests is:

[linux-kernel-mymodule](https://github.com/LavyCo/linux-kernel-mymodule)

## Development Environment

- Raspberry Pi 5
- Buildroot 2026.08
- Linux Kernel 6.12
- C programming
- Linux kernel modules
- GCC cross-compilation
- Git and GitHub
- SSH, SCP, and Minicom
