#ifndef _LTTNG_WRAPPER_GENHD_H
#define _LTTNG_WRAPPER_GENHD_H

/*
 * wrapper/mm_info.h
 *
 * wrapper around memory functions and data structres. Using
 * KALLSYMS to get its address when available, else we need to have a
 * kernel that exports this function to GPL modules.
 *
 * Copyright (C) 2011-2014 Mathieu Desnoyers <mathieu.desnoyers@efficios.com>
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

#include <linux/genhd.h>

#ifdef CONFIG_KALLSYMS

#include <linux/kallsyms.h>
#include <wrapper/kallsyms.h>

static inline
struct anon_vma_chain *wrapper_anon_vma_interval_tree_iter_first(struct rb_root *root,
		  unsigned long first, unsigned long last)
{
	struct anon_vma_chain *(*anon_vma_interval_tree_iter_first_sym)(struct rb_root *root,
			  unsigned long first, unsigned long last);

	anon_vma_interval_tree_iter_first_sym = (void *) kallsyms_lookup_funcptr("anon_vma_interval_tree_iter_first");
	if (anon_vma_interval_tree_iter_first_sym) {
		return anon_vma_interval_tree_iter_first_sym(root, first, last);
	} else {
		printk(KERN_WARNING "LTTng: anon_vma_interval_tree_iter_first symbol lookup failed.\n");
		return NULL;
	}
}

static inline
struct anon_vma_chain *wrapper_anon_vma_interval_tree_iter_next(struct anon_vma_chain *node,
		 unsigned long first, unsigned long last)
{
	struct anon_vma_chain *(*anon_vma_interval_tree_iter_next_sym)(struct anon_vma_chain *node,
			 unsigned long first, unsigned long last);

	anon_vma_interval_tree_iter_next_sym = (void *) kallsyms_lookup_funcptr("anon_vma_interval_tree_iter_next");
	if (anon_vma_interval_tree_iter_next_sym) {
		return anon_vma_interval_tree_iter_next_sym(node, first, last);
	} else {
		printk(KERN_WARNING "LTTng: anon_vma_interval_tree_iter_next symbol lookup failed.\n");
		return NULL;
	}
}

static inline
int wrapper_rmap_walk(struct page *page, struct rmap_walk_control *rwc)
{
	int (*rmap_walk_sym)(struct page *page, struct rmap_walk_control *rwc);

	rmap_walk_sym = (void *) kallsyms_lookup_funcptr("rmap_walk");
	if (rmap_walk_sym) {
		return rmap_walk_sym(page, rwc);
	} else {
		printk(KERN_WARNING "LTTng: rmap_walk_sym symbol lookup failed.\n");
		return 0;
	}
}

static inline
struct anon_vma *wrapper_page_lock_anon_vma_read(struct page *page)
{
	struct anon_vma *(*page_lock_anon_vma_read_sym)(struct page *page);

	page_lock_anon_vma_read_sym = (void *) kallsyms_lookup_funcptr("page_lock_anon_vma_read");
	if (page_lock_anon_vma_read_sym) {
		return page_lock_anon_vma_read_sym(page);
	} else {
		printk(KERN_WARNING "LTTng: page_lock_anon_vma_read symbol lookup failed.\n");
		return NULL;
	}
}

#else

static inline
char *wrapper_anon_vma_interval_tree_iter_first(struct rb_root *root,
		  unsigned long first, unsigned long last)
{
	return NULL;
}

static inline
char *wrapper_anon_vma_interval_tree_iter_next(struct anon_vma_chain *node,
		 unsigned long first, unsigned long last)
{
	return NULL;
}

static inline
int wrapper_rmap_walk(struct page *page, struct rmap_walk_control *rwc)
{
	return 0;
}

static inline
struct anon_vma *wrapper_page_lock_anon_vma_read(struct page *page)
{
	return NULL;
}

#endif


#endif /* _LTTNG_WRAPPER_GENHD_H */
