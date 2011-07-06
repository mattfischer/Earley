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

#define list_remove(item) \
{ \
	(item).prev->next = (item).next; \
	(item).next->prev = (item).prev; \
}

#define container_of(type, member, ptr) \
	(type *)((char*)ptr - offsetof(type, member))

#define list_foreach(type, cursor, member, list) \
	for(cursor = container_of(type, member, (list).next); &cursor->member != &(list); cursor = container_of(type, member, cursor->member.next))
		
#endif