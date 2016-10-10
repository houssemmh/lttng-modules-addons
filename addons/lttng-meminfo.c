/*
 * addons/lttng-mmfree.c
 *
 * Missing tracepoint for recovering the block device request chain
 *
 * Copyright (C) 2015 Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; only
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kprobes.h>
#include <linux/blkdev.h>
#include <linux/spinlock.h>
#include <linux/rmap.h>
#include <linux/page-flags.h>
#include <linux/pagemap.h>
#include <linux/sched.h>
#include <linux/mm_types.h>
#include <linux/timer.h>

#include "../wrapper/tracepoint.h"
//#include "../wrapper/mm_info.h"
#include "../lttng-abi.h"
#define LTTNG_INSTRUMENTATION
#include "../instrumentation/events/lttng-module/addons.h"

DEFINE_TRACE(addons_process_meminfo);

extern struct task_struct init_task;
int delay = 100;
struct timer_list timer;

void getMemInfo(unsigned long data) {
	struct task_struct *p;
	long rss_anon, rss_files, rss_kb;
	struct mm_struct *mm;

	mod_timer(&timer, jiffies + msecs_to_jiffies(delay));
	rcu_read_lock();
	for_each_process(p)
	{
		mm = get_task_mm(p);
		if (mm) {
			rss_anon = atomic_long_read(&mm->rss_stat.count[MM_ANONPAGES]);
			rss_files = atomic_long_read(&mm->rss_stat.count[MM_FILEPAGES]);
			rss_kb = (rss_anon + rss_files) << (PAGE_SHIFT - 10);
			printk(KERN_INFO "process %s, pid:%d, rss: %lu\n", p->comm, p->pid,
					rss_kb);
			trace_addons_process_meminfo(p->pid, rss_kb);
		}

	}
	rcu_read_unlock();

}

//static int lttng_mmfree_probe(struct page *page, struct vm_area_struct *vma,
//		unsigned long address) {
//	printk(KERN_INFO "PROBE!\n");
//out:
//	jprobe_return();
//	return 0;
//
//}

//static struct jprobe mmfree_jprobe = { .entry = lttng_mmfree_probe, .kp = {
//		.symbol_name = "page_move_anon_rmap", }, };

static int __init lttng_addons_mmfree_init(void) {
//	int ret;
//	ret = register_jprobe(&mmfree_jprobe);

	(void) wrapper_lttng_fixup_sig(THIS_MODULE);
	printk(KERN_INFO "Hello world!\n");
	setup_timer(&timer, getMemInfo, 0);
	mod_timer(&timer, jiffies + msecs_to_jiffies(delay));
	return 0;

//
//	(void) wrapper_lttng_fixup_sig(THIS_MODULE);
//
//	if (ret < 0) {
//		printk("register_jprobe failed, returned %d\n", ret);
//		goto out;
//	}
//
//	printk("lttng-mmfree loaded\n");
//out:
//	return ret;
}

module_init(lttng_addons_mmfree_init);

static void __exit lttng_addons_mmfree_exit(void) {
	del_timer(&timer);
//	unregister_jprobe(&mmfree_jprobe);
	printk("lttng-mmfree removed\n");
}
module_exit(lttng_addons_mmfree_exit);

MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("Francis Giraldeau <francis.giraldeau@gmail.com>");
MODULE_DESCRIPTION("LTTng block elevator event");

