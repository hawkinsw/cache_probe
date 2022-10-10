#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Will Hawkins");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.01");

#define PMUSERENR_EN_EL0 (1 << 0)
#define PMUSERENR_CR (1 << 2)           // How to enable the event count read.
#define PMUSERENR_ER (1 << 3)           // How to enable the event count read.
#define PMCNTENSET_EL0_ENABLE (1 << 31 | 1 << 0) // Enable the cycle count
                                                 // and the 0th perfomance
                                                 // count register.
#define PMCR_ENABLE (1 << 0)

static void enable_counters(void * _) {
  uint64_t val = 0;
  /*Enable user-mode access to counters. */
  asm volatile("msr pmuserenr_el0, %0"
               :
               : "r"((u64)PMUSERENR_EN_EL0 | PMUSERENR_ER | PMUSERENR_CR));
  /* Performance Monitors Count Enable Set register bit 30:0 disable, 31
   * enable. Can also enable other event counters here. */
  asm volatile("msr pmcntenset_el0, %0" : : "r"(PMCNTENSET_EL0_ENABLE));


  /* What we did above was say, "When we enable performance monitoring, 
   * these will be the event registers we want to use." We still
   * need to actually enable them!
   */
  asm volatile("mrs %0, pmcr_el0" : "=r"(val));
  asm volatile("msr pmcr_el0, %0" : : "r"(val | PMCR_ENABLE));
}

static int __init perfc_module_init(void) {
  on_each_cpu(enable_counters, NULL, 1);
  printk(KERN_INFO "Just finished attempting to enable perf counter access!\n");
  return 0;
}

static void __exit perfc_module_exit(void) {
  printk(KERN_INFO "Perf counter module quitting!\n");
}
module_init(perfc_module_init);
module_exit(perfc_module_exit);
