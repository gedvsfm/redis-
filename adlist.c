/* adlist.c - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdlib.h>
#include "adlist.h"
#include "zmalloc.h"

/* Create a new list. The created list can be freed with
 * AlFreeList(), but private value of every node need to be freed
 * by the user before to call AlFreeList().
 *
 * On error, NULL is returned. Otherwise the pointer to the new list. */
list *listCreate(void)//创建一个表头
{
    struct list *list;

    if ((list = zmalloc(sizeof(*list))) == NULL)//分配内存
        return NULL;
	//初始化
    list->head = list->tail = NULL;
    list->len = 0;
    list->dup = NULL;
    list->free = NULL;
    list->match = NULL;
    return list;
}

/* Remove all the elements from the list without destroying the list itself. */
void listEmpty(list *list)//从链表中移除所有的元素但是没有释放自身
{
    unsigned long len;
    listNode *current, *next;

    current = list->head;
    len = list->len;
	//遍历链表删除所有的节点
    while(len--) {
        next = current->next;
        if (list->free) list->free(current->value);//如果设置了list结构的释放函数，则调用该函数释放节点值
        zfree(current);//释放current所指向节点的内存
        current = next;
    }
    list->head = list->tail = NULL;
    list->len = 0;
}

/* Free the whole list.
 *
 * This function can't fail. */
void listRelease(list *list)//释放整个链表，分成两步
{
    listEmpty(list);
    zfree(list);
}

/* Add a new node to the list, to head, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
list *listAddNodeHead(list *list, void *value)//将value添加到链表的头部
{
    listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (list->len == 0) {//将node插入到空链表
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {			//将node插入到非空链表
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->len++;
    return list;
}

/* Add a new node to the list, to tail, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
list *listAddNodeTail(list *list, void *value)//将value添加到链表的尾部
{
    listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (list->len == 0) {//将node插入到空链表
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {			//将node插入到非空链表
        node->prev = list->tail;
        node->next = NULL;
        list->tail->next = node;
        list->tail = node;
    }
    list->len++;
    return list;
}

list *listInsertNode(list *list, listNode *old_node, void *value, int after) {//跟据after的值决定value插在old_node前面还是后面
    listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)//分配内存
        return NULL;
    node->value = value;
    if (after) {	//after为1插入到old_node的后面
        node->prev = old_node;
        node->next = old_node->next;
        if (list->tail == old_node) {//如果old_node是尾节点，node作为新的尾节点
            list->tail = node;
        }
    } else {		//after为1插入到old_node的后面
        node->next = old_node;
        node->prev = old_node->prev;
        if (list->head == old_node) {//如果old_node是头结点，node作为新的头节点
            list->head = node;
        }
    }
    if (node->prev != NULL) {//node不为头节点，则前驱节点指向node
        node->prev->next = node;
    }
    if (node->next != NULL) {//node不为尾节点，则后继节点的前驱指向node
        node->next->prev = node;
    }
    list->len++;
    return list;
}

/* Remove the specified node from the specified list.
 * It's up to the caller to free the private value of the node.
 *
 * This function can't fail. */
void listDelNode(list *list, listNode *node)//从链表中删除节点node
{
    if (node->prev)	//node不为头节点，则node的前驱节点的next指针指向node的后继节点
        node->prev->next = node->next;
    else			//node为头节点，则将node的后继节点作为头节点
        list->head = node->next;
    if (node->next)//node不为尾节点，则node的后继节点的prev指针指向node的前驱节点
        node->next->prev = node->prev;
    else			//node为尾节点，则node的前驱节点作为新的尾节点
        list->tail = node->prev;
    if (list->free) list->free(node->value);//如果定义free函数则调用free函数释放value
    zfree(node);
    list->len--;
}

/* Returns a list iterator 'iter'. After the initialization every
 * call to listNext() will return the next element of the list.
 *
 * This function can't fail. */
listIter *listGetIterator(list *list, int direction)//根据direction为链表创建一个迭代器
{
    listIter *iter;

    if ((iter = zmalloc(sizeof(*iter))) == NULL) return NULL;//分配内存
    if (direction == AL_START_HEAD)//如果是正向，则迭代器指向链表的头部
        iter->next = list->head;
    else							//如果是反向，则迭代器指向链表的尾部
        iter->next = list->tail;
    iter->direction = direction;
    return iter;
}

/* Release the iterator memory */
void listReleaseIterator(listIter *iter) {//释放迭代器
    zfree(iter);
}

/* Create an iterator in the list private iterator structure */
void listRewind(list *list, listIter *li) {//将迭代器重置为list的头结点并且设置为正向迭代
    li->next = list->head;
    li->direction = AL_START_HEAD;
}

void listRewindTail(list *list, listIter *li) {//将迭代器li重置为list的尾结点并且设置为反向迭代
    li->next = list->tail;
    li->direction = AL_START_TAIL;
}

/* Return the next element of an iterator.
 * It's valid to remove the currently returned element using
 * listDelNode(), but not to remove other elements.
 *
 * The function returns a pointer to the next element of the list,
 * or NULL if there are no more elements, so the classical usage patter
 * is:
 *
 * iter = listGetIterator(list,<direction>);
 * while ((node = listNext(iter)) != NULL) {
 *     doSomethingWith(listNodeValue(node));
 * }
 *
 * */
listNode *listNext(listIter *iter)//返回迭代器当前所指向的节点指针，并且迭代器移动到下一个
{
    listNode *current = iter->next;

    if (current != NULL) {
        if (iter->direction == AL_START_HEAD)
            iter->next = current->next;
        else
            iter->next = current->prev;
    }
    return current;
}

/* Duplicate the whole list. On out of memory NULL is returned.
 * On success a copy of the original list is returned.
 *
 * The 'Dup' method set with listSetDupMethod() function is used
 * to copy the node value. Otherwise the same pointer value of
 * the original node is used as value of the copied node.
 *
 * The original list both on success or error is never modified. */
list *listDup(list *orig)//拷贝整个链表
{
    list *copy;
    listIter iter;
    listNode *node;

    if ((copy = listCreate()) == NULL)//创建表头
        return NULL;
	//拷贝表头的dup，free，match函数
    copy->dup = orig->dup;
    copy->free = orig->free;
    copy->match = orig->match;
    listRewind(orig, &iter);//建立链表orig的一个正向迭代器
    while((node = listNext(&iter)) != NULL) {//链表的node不为NULL，调用完listNext后迭代器iter已经后移
        void *value;

        if (copy->dup) {//如果定义dup，则调用为value赋值
            value = copy->dup(node->value);
            if (value == NULL) {//如果dup失败，则将链表copy整个释放并返回NULL
                listRelease(copy);
                return NULL;
            }
        } else//如果没有定义就直接赋值
            value = node->value;
        if (listAddNodeTail(copy, value) == NULL) {//将value添加到链表copy的尾部，如果添加失败，删除整个copy链表
            listRelease(copy);
            return NULL;
        }
    }
    return copy;
}

/* Search the list for a node matching a given key.
 * The match is performed using the 'match' method
 * set with listSetMatchMethod(). If no 'match' method
 * is set, the 'value' pointer of every node is directly
 * compared with the 'key' pointer.
 *
 * On success the first matching node pointer is returned
 * (search starts from head). If no matching node exists
 * NULL is returned. */
listNode *listSearchKey(list *list, void *key)//在链表中查找值为key的节点并返回该节点
{
    listIter iter;//建立一个迭代器
    listNode *node;

    listRewind(list, &iter);//将迭代器iter重置为链表list的正向迭代器并指向链表的头部
    while((node = listNext(&iter)) != NULL) {//链表的node不为NULL，调用完listNext后迭代器iter已经后移
        if (list->match) {//如果定义match函数则调用match
            if (list->match(node->value, key)) {
                return node;
            }
        } else {	//没用定义则直接比较
            if (key == node->value) {
                return node;
            }
        }
    }
    return NULL;
}

/* Return the element at the specified zero-based index
 * where 0 is the head, 1 is the element next to head
 * and so on. Negative integers are used in order to count
 * from the tail, -1 is the last element, -2 the penultimate
 * and so on. If the index is out of range NULL is returned. */
listNode *listIndex(list *list, long index) {//返回下标为index的节点
    listNode *n;

    if (index < 0) {//如果index是负数，则表示倒数第几个，如-1表示倒数第一个
        index = (-index)-1;
        n = list->tail;
        while(index-- && n) n = n->prev;
    } else {//index大于等于0,0表示第一个即头节点
        n = list->head;
        while(index-- && n) n = n->next;
    }
    return n;
}

/* Rotate the list removing the tail node and inserting it to the head. */
void listRotate(list *list) {//将尾节点插到头结点前作为新的头节点
    listNode *tail = list->tail;

    if (listLength(list) <= 1) return;

    /* Detach current tail */
	//更新尾节点
    list->tail = tail->prev;
    list->tail->next = NULL;
    /* Move it as head */
	//插入并更新头节点
    list->head->prev = tail;
    tail->prev = NULL;
    tail->next = list->head;
    list->head = tail;
}

/* Add all the elements of the list 'o' at the end of the
 * list 'l'. The list 'other' remains empty but otherwise valid. */
void listJoin(list *l, list *o) {//将链表o插入链表l的尾部，并将链表o重置为空链表
    if (o->head)//如果链表o不为空，则将o的头结点的前驱指向链表l的尾节点
        o->head->prev = l->tail;

    if (l->tail)//如果链表l不为空，则将l的尾结点的后继指向链表o的头节点
        l->tail->next = o->head;
    else//如果为空o的头结点作为链表l的头节点
        l->head = o->head;

    if (o->tail) l->tail = o->tail;//如果链表o的尾节点存在，则更新链表l的尾节点
    l->len += o->len;

    /* Setup other as an empty list. */
	//将链表o设置为一个空链表
    o->head = o->tail = NULL;
    o->len = 0;
}
