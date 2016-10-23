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
#include <linux/hashtable.h>
#include <linux/jhash.h>

#include "../wrapper/tracepoint.h"
//#include "../wrapper/mm_info.h"
#include "../lttng-abi.h"
#define LTTNG_INSTRUMENTATION
#include "../instrumentation/events/lttng-module/addons.h"

DEFINE_TRACE(addons_process_meminfo);
#define COUNTER_MAX 1000

extern struct task_struct init_task;
int delay = 2000;
struct workqueue_struct *meminfo_queue;
static DEFINE_HASHTABLE(process_map, 3);

struct process_key_t {
  pid_t tgid;
} __attribute__((__packed__));

struct process_val_t {
  pid_t tgid;
  int counter;
  struct hlist_node hlist;
  struct rcu_head rcu;
};

static struct Workqueue_data {
	struct delayed_work delayed_getMemInfo;
	int delay;
} wq_data;

static void free_process_val_rcu(struct rcu_head *rcu)
{
  kfree(container_of(rcu, struct process_val_t, rcu));
}

static struct process_val_t*
find_process(struct process_key_t *key, u32 hash)
{
  struct process_val_t *val;

  hash_for_each_possible_rcu(process_map, val, hlist, hash) {
    if (key->tgid == val->tgid) {
      return val;
    }
  }
  return NULL;
}

static void register_process(struct task_struct *p) {
	u32 hash;
	struct process_key_t key;
	struct process_val_t *val;
	key.tgid = p->tgid;
	hash = jhash(&key, sizeof(key), 0);
	val = kzalloc(sizeof(struct process_val_t), GFP_KERNEL);
	val->tgid = key.tgid;
	val->counter = COUNTER_MAX;
	hash_add_rcu(process_map, &val->hlist, hash);
}

static void show_process_map(void) {
	struct process_val_t *process_val;
	int bkt;
	rcu_read_lock();
	hash_for_each_rcu(process_map, bkt, process_val, hlist)
	{
		printk(KERN_INFO "Hash contains process %d\n",process_val->tgid);
	}
	rcu_read_unlock();
}

static void clear_process_map(void) {
	struct process_val_t *process_val;
	int bkt;
	rcu_read_lock();
	hash_for_each_rcu(process_map, bkt, process_val, hlist)
	{
		hash_del_rcu(&process_val->hlist);
		call_rcu(&process_val->rcu, free_process_val_rcu);
	}
	rcu_read_unlock();
	synchronize_rcu();
}

static void getMemInfo(struct work_struct *work) {
	struct task_struct *p;
	long rss_anon, rss_files, rss_kb;
	struct mm_struct *mm;
	clear_process_map();
	rcu_read_lock();
	for_each_process(p)
	{
		mm = get_task_mm(p);
		if (mm) {
			register_process(p);
			rss_anon = atomic_long_read(&mm->rss_stat.count[MM_ANONPAGES]);
			rss_files = atomic_long_read(&mm->rss_stat.count[MM_FILEPAGES]);
			rss_kb = (rss_anon + rss_files) << (PAGE_SHIFT - 10);
			printk(KERN_INFO "process %s, pid:%d, rss: %lu\n", p->comm, p->pid,
					rss_kb);
			trace_addons_process_meminfo(p->pid, rss_kb);
		}

	}
	rcu_read_unlock();
	show_process_map();
	queue_delayed_work(meminfo_queue, &wq_data.delayed_getMemInfo, msecs_to_jiffies(wq_data.delay));
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

	(void) wrapper_lttng_fixup_sig(THIS_MODULE);
//	int ret;
//	ret = register_jprobe(&meminfo_jprobe);
	printk(KERN_INFO "Hello world!\n");
	meminfo_queue = create_workqueue("meminfo_queue");
	wq_data.delay = delay;
	INIT_DELAYED_WORK(&wq_data.delayed_getMemInfo, getMemInfo );
	queue_delayed_work(meminfo_queue, &wq_data.delayed_getMemInfo, msecs_to_jiffies(wq_data.delay));

	return 0;
}

module_init(lttng_addons_meminfo_init);

static void __exit lttng_addons_meminfo_exit(void) {
	cancel_delayed_work(&wq_data.delayed_getMemInfo);
	flush_workqueue( meminfo_queue );
	destroy_workqueue( meminfo_queue );
//	unregister_jprobe(&meminfo_jprobe);
	printk("lttng-meminfo removed\n");
}
module_exit(lttng_addons_meminfo_exit);

MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("Houssem Daoud <houssemmh@gmail.com>");
MODULE_DESCRIPTION("LTTng meminfo event");

