/*
 * addons/lttng-meminfo.c
 *
 * Generate events for memory utilization of processes.
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
#include <linux/rmap.h>
#include <linux/page-flags.h>
#include <linux/pagemap.h>
#include <linux/sched.h>
#include <linux/mm_types.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#include "../wrapper/tracepoint.h"
//#include "../wrapper/mm_info.h"
#include "../lttng-abi.h"
#define LTTNG_INSTRUMENTATION
#include "../instrumentation/events/lttng-module/addons.h"

DEFINE_TRACE(addons_process_meminfo);

extern struct task_struct init_task;
int delay = 2000;
struct workqueue_struct *meminfo_queue;

static void getMemInfo(struct work_struct *work) {
	struct task_struct *p;
	long rss_anon, rss_files, rss_kb;
	struct mm_struct *mm;

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

//static int lttng_meminfo_probe(struct page *page, struct vm_area_struct *vma,
//		unsigned long address) {
//	printk(KERN_INFO "PROBE!\n");
//out:
//	jprobe_return();
//	return 0;
//
//}

//static struct jprobe meminfo_jprobe = { .entry = lttng_meminfo_probe, .kp = {
//		.symbol_name = "page_move_anon_rmap", }, };

static int __init lttng_addons_meminfo_init(void) {

	struct delayed_work *showinfo_work;
	(void) wrapper_lttng_fixup_sig(THIS_MODULE);
//	int ret;
//	ret = register_jprobe(&meminfo_jprobe);
	showinfo_work = kmalloc( sizeof(struct delayed_work), GFP_KERNEL);
	printk(KERN_INFO "Hello world!\n");
	meminfo_queue = create_workqueue("meminfo_queue");
	INIT_DELAYED_WORK( showinfo_work, getMemInfo );
	queue_delayed_work(meminfo_queue, showinfo_work, msecs_to_jiffies(delay));
	return 0;
}

module_init(lttng_addons_meminfo_init);

static void __exit lttng_addons_meminfo_exit(void) {
	flush_workqueue( meminfo_queue );
	destroy_workqueue( meminfo_queue );
//	unregister_jprobe(&meminfo_jprobe);
	printk("lttng-meminfo removed\n");
}
module_exit(lttng_addons_meminfo_exit);

MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("Houssem Daoud <houssemmh@gmail.com>");
MODULE_DESCRIPTION("LTTng meminfo event");

