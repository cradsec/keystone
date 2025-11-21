######################
## QEMU run targets ##
######################

KEYSTONE_PORT   ?= 9821
QEMU_DBG_PORT   ?= $(shell echo $$(( $(KEYSTONE_PORT) + 1)) )
QEMU_DEBUG      := -gdb tcp::$(QEMU_DBG_PORT) -S

QEMU_MEM_UBUNTU ?= 6G
QEMU_SMP        ?=  $(shell nproc)

UBUNTU_KERNEL ?= /opt/riscv/ubuntu/vmlinuz-6.14.0-24-generic
QEMU_FLAGS_UBUNTU := -m $(QEMU_MEM_UBUNTU) -smp $(QEMU_SMP) -nographic \
	        -machine virt,rom=$(BUILDROOT_BUILDDIR)/images/bootrom.bin \
		-bios $(BUILDROOT_BUILDDIR)/images/fw_jump.elf \
                -kernel $(UBUNTU_KERNEL) \
                -drive file=/opt/riscv/ubuntu/ubuntu.img,format=raw,id=hd0 \
                -device virtio-blk-device,drive=hd0 \
		 -drive file=$(BUILDROOT_BUILDDIR)/images/rootfs.ext2,format=raw,id=hd1 \
                -device virtio-blk-device,drive=hd1 \
                -append "console=ttyS0 ro root=/dev/vda1" \
                -netdev user,id=net0,net=192.168.100.1/24,dhcpstart=192.168.100.128,hostfwd=tcp::$(KEYSTONE_PORT)-:22 \
                -device virtio-net-device,netdev=net0 \
                -device virtio-rng-pci \
		-virtfs local,path=/opt/riscv,mount_tag=host,security_model=none

ifneq ($(KEYSTONE_DEBUG),)
        QEMU_FLAGS_UBUNTU += $(QEMU_DEBUG)
endif

run-ubuntu:
	$(call log,info,Starting QEMU)
	$(BUILDROOT_BUILDDIR)/host/bin/qemu-system-riscv$(KEYSTONE_BITS) $(QEMU_FLAGS_UBUNTU)
