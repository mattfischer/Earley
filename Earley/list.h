#ifndef LIST_H
#define LIST_H

#include <stddef.h>

struct ListHead {
	struct ListHead *prev;
	struct ListHead *next;
};

#define list_init(item) \
{ \
	(item).next = &(item); \
	(item).prev = &(item); \
}

#define list_add_head(list, item) \
{ \
	(item).next = (list).next; \
	(item)prev = &(list); \
	(list).next->prev = &(item); \
	(list).next = &(item); \
}

#define list_add_tail(list, item) \
{ \
	(item).next = &(list); \
	(item).prev = (list).prev; \
	(list).prev->next = &(item); \
	(list).prev = &(item); \
}

#define list_add_list_head(target, list) \
	if((list).next != &(list)) { \
		(list).prev->next = (target).next; \
		(list).next->prev = &(target); \
		(target).next->prev = (list).prev; \
		(target).next = (list).next; \
	}

#define list_add_list_tail(target, list) \
	if((list).next != &(list)) { \
		(list).prev->next = &(target); \
		(list).next->prev = (target).prev; \
		(target).prev->next = (list).next; \
		(target).prev = (list).prev; \
	}

#define list_head(type, member, list) \
	((list).next == &(list)) ? NULL : container_of(type, member, (list).next)

#define list_tail(type, member, list) \
	((list).prev == &(list)) ? NULL : container_of(type, member, (list).prev)

#define list_remove(item) \
{ \
	(item).prev->next = (item).next; \
	(item).next->prev = (item).prev; \
}

#define list_empty(list) ((list).next == &(list))

#define container_of(type, member, ptr) \
	(type *)((char*)ptr - offsetof(type, member))

#define list_foreach(type, member, cursor, list) \
	for(cursor = container_of(type, member, (list).next); &cursor->member != &(list); cursor = container_of(type, member, cursor->member.next))

#define list_foreach_ex(type, member, cursor, extra, list) \
	for(cursor = container_of(type, member, (list).next), extra = container_of(type, member, cursor->member.next); &cursor->member != &(list); cursor = extra, extra = container_of(type, member, cursor->member.next))

#endif