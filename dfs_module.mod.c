#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x7963ddf2, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x588308d, __VMLINUX_SYMBOL_STR(unregister_filesystem) },
	{ 0x5d2366a1, __VMLINUX_SYMBOL_STR(register_filesystem) },
	{ 0xb29391a8, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xda4faf59, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xe914e41e, __VMLINUX_SYMBOL_STR(strcpy) },
	{ 0xb5419b40, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0xc671e369, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0x88db9f48, __VMLINUX_SYMBOL_STR(__check_object_size) },
	{ 0xe2d5255a, __VMLINUX_SYMBOL_STR(strcmp) },
	{ 0x4fb9968a, __VMLINUX_SYMBOL_STR(d_add) },
	{ 0x9c490ba0, __VMLINUX_SYMBOL_STR(d_make_root) },
	{ 0xe8ef0ff0, __VMLINUX_SYMBOL_STR(current_kernel_time64) },
	{ 0x5264a5c7, __VMLINUX_SYMBOL_STR(inode_init_owner) },
	{ 0x15548997, __VMLINUX_SYMBOL_STR(new_inode) },
	{ 0x7df2e45d, __VMLINUX_SYMBOL_STR(mount_bdev) },
	{ 0x50873178, __VMLINUX_SYMBOL_STR(kill_block_super) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

