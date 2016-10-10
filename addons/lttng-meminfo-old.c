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


#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kprobes.h>
#include <linux/blkdev.h>
#include <linux/spinlock.h>
#include <linux/mm_types.h>
#include <linux/rmap.h>
#include <linux/page-flags.h>
#include <linux/pagemap.h>

#include "../wrapper/tracepoint.h"
#include "../wrapper/mm_info.h"
#include "../lttng-abi.h"
#define LTTNG_INSTRUMENTATION
#include "../instrumentation/events/lttng-module/addons.h"

DEFINE_TRACE(addons_elv_merge_requests);

static int process_one(struct page *page, struct vm_area_struct *vma,
		     unsigned long address, void *arg)
{
	printk("Page %lu mapped in vma %p, pid %lu\n", page_to_pfn(page), vma, vma->vm_mm->owner->pid);
	return 0;
}

static int page_not_mapped(struct page *page)
{
	return !page_mapped(page);
}

void lttng_mmfree_probe(struct page *page,
		struct vm_area_struct *vma, unsigned long address) {

	struct rmap_walk_control rwc = {
			.rmap_one = process_one,
			.done = page_not_mapped,
			.anon_lock = wrapper_page_lock_anon_vma_read,

	};

	VM_BUG_ON_PAGE(!PageHuge(page) && PageTransHuge(page), page);
	VM_BUG_ON_PAGE(!PageLocked(page) || PageLRU(page), page);
	wrapper_rmap_walk(page, &rwc);


	mapcount = page_mapcount(page);
	pgoff = page->index << (PAGE_CACHE_SHIFT - PAGE_SHIFT);

	for (avc = wrapper_anon_vma_interval_tree_iter_first(&anon_vma->rb_root, pgoff, pgoff+PAGE_SIZE); \
		     avc; avc = wrapper_anon_vma_interval_tree_iter_next(avc, pgoff, pgoff))
	{
		struct vm_area_struct *vma = avc->vma;
		printk("%lu\n", vma->vm_mm->owner->pid);
	}


out:
	jprobe_return();

}

static struct jprobe mmfree_jprobe = { .entry = lttng_mmfree_probe, .kp = {
		.symbol_name = "page_move_anon_rmap", }, };

static int __init lttng_addons_mmfree_init(void) {
	int ret;

	(void) wrapper_lttng_fixup_sig(THIS_MODULE);
	ret = register_jprobe(&mmfree_jprobe);
	if (ret < 0) {
		printk("register_jprobe failed, returned %d\n", ret);
		goto out;
	}

	printk("lttng-mmfree loaded\n");
	out: return ret;
}
module_init(lttng_addons_mmfree_init);

static void __exit lttng_addons_mmfree_exit(void) {
	unregister_jprobe(&mmfree_jprobe);
	printk("lttng-mmfree removed\n");
}
module_exit(lttng_addons_mmfree_exit);

MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("Francis Giraldeau <francis.giraldeau@gmail.com>");
MODULE_DESCRIPTION("LTTng block elevator event");

*/
