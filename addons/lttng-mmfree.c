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
#include <linux/mm_types.h>
#include <linux/rmap.h>
#include <linux/page-flags.h>

#include "../wrapper/tracepoint.h"
#include "../lttng-abi.h"
#define LTTNG_INSTRUMENTATION
#include "../instrumentation/events/lttng-module/addons.h"

DEFINE_TRACE(addons_elv_merge_requests);

static int lttng_mmfree_probe(struct page *page) {
	struct anon_vma *anon_vma = NULL;
	unsigned long anon_mapping;
	unsigned int mapcount;
	pgoff_t pgoff;
	struct anon_vma_chain *avc;
	anon_mapping = (unsigned long) page->mapping;
	// go out if the page is not anonymous
	if (!PageAnon(page)) {
		goto out;
	}
	anon_vma = (struct anon_vma *) (anon_mapping - PAGE_MAPPING_ANON);
	if (!anon_vma)
		goto out;
	mapcount = page_mapcount(page);
	pgoff = page_to_pgoff(page);

	anon_vma_interval_tree_foreach(avc, &anon_vma->rb_root, pgoff, pgoff)
	{
		struct vm_area_struct *vma = avc->vma;
		printk("%lu\n", vma->vm_start);
	}

	printk("Free page %lu, Map count %u\n", page_to_pfn(page), mapcount);
	out: jprobe_return();
	return 0;
}

static struct jprobe mmfree_jprobe = { .entry = lttng_mmfree_probe, .kp = {
		.symbol_name = "free_pages_prepare", }, };

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

