#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/smp.h>
#include <linux/cpumask.h>
#include <linux/kallsyms.h>
#include <linux/string.h>
#include <asm/cpu.h>
#include <asm/percpu.h>

static char *cpu_name_ptr = NULL;
static char *machine_name_ptr = NULL;

// 默认伪装成 Kirin 8020 测试版，你可以随时在 insmod 时修改
static char *fake_processor = "Kirin 8020 伪装测试";
static char *fake_hardware = "HUAWEI Kirin8020";
static unsigned int fake_implementer = 0x48;
static unsigned int fake_part_primary = 0xD23;
static unsigned int fake_part_secondary = 0xD46;
static unsigned int fake_variant = 0x2;
static unsigned int fake_revision = 1;

module_param(fake_processor, charp, 0);
module_param(fake_hardware, charp, 0);
module_param(fake_implementer, uint, 0);
module_param(fake_part_primary, uint, 0);
module_param(fake_part_secondary, uint, 0);
module_param(fake_variant, uint, 0);
module_param(fake_revision, uint, 0);

static int __init cpufake_init(void)
{
    int cpu;
    cpu_name_ptr = (char *)kallsyms_lookup_name("cpu_name");
    machine_name_ptr = (char *)kallsyms_lookup_name("machine_name");
    if (!cpu_name_ptr || !machine_name_ptr) {
        pr_err("cpufake: 找不到必要内核符号\n");
        return -ENOENT;
    }

    strncpy(cpu_name_ptr, fake_processor, 63);
    strncpy(machine_name_ptr, fake_hardware, 63);
    pr_info("cpufake: Processor=%s  Hardware=%s\n", cpu_name_ptr, machine_name_ptr);

    for_each_possible_cpu(cpu) {
        struct cpuinfo_arm64 *cpuinfo = per_cpu_ptr(&cpu_data, cpu);
        if (!cpuinfo) continue;

        u32 midr = cpuinfo->reg_midr;
        u32 part = (cpu <= 3) ? fake_part_primary : fake_part_secondary;

        midr &= ~(0xFF000000 | 0x00F00000 | 0x0000FFF0 | 0x0000000F);
        midr |= (fake_implementer & 0xFF) << 24;
        midr |= (fake_variant & 0xF) << 20;
        midr |= (part & 0xFFF) << 4;
        midr |= (fake_revision & 0xF);

        cpuinfo->reg_midr = midr;
    }
    pr_info("cpufake: MIDR 全核伪装完成\n");
    return 0;
}

static void __exit cpufake_exit(void)
{
    pr_info("cpufake: 卸载，伪装仍在内存中直到重启\n");
}

module_init(cpufake_init);
module_exit(cpufake_exit);
MODULE_LICENSE("GPL");
