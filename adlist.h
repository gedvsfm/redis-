/* adlist.h - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __ADLIST_H__
#define __ADLIST_H__

/* Node, List, and Iterator are the only data structures used currently. */

typedef struct listNode {
    struct listNode *prev;//前驱节点
    struct listNode *next;//后继节点
    void *value;		//可以存放任意类型的信息
} listNode;

typedef struct listIter {//链表的迭代器
    listNode *next;		//迭代器当前所指向的节点（并不是下一个）
    int direction;		//迭代器的方向，可以取两个值
} listIter;

typedef struct list {
    listNode *head;		//链表头节点指针
    listNode *tail;		//链表尾节点指针
    void *(*dup)(void *ptr);	//复制链表节点所保存的value
    void (*free)(void *ptr);	//释放链表节点保存的值
    int (*match)(void *ptr, void *key);//判断链表节点所保存的值是否和传入的值key相等
    unsigned long len;		//链表的长度
} list;

/* Functions implemented as macros */
#define listLength(l) ((l)->len)					//返回链表的长度
#define listFirst(l) ((l)->head)					//返回链表的头结点
#define listLast(l) ((l)->tail)						//返回链表的尾节点
#define listPrevNode(n) ((n)->prev)					//返回节点的前驱节点
#define listNextNode(n) ((n)->next)					//返回节点的后继节点
#define listNodeValue(n) ((n)->value)				//返回节点的值

#define listSetDupMethod(l,m) ((l)->dup = (m))		//设置链表的复制函数
#define listSetFreeMethod(l,m) ((l)->free = (m))	//设置链表的释放函数
#define listSetMatchMethod(l,m) ((l)->match = (m))	//设置链表的比较函数

#define listGetDupMethod(l) ((l)->dup)				//返回链表的复制函数
#define listGetFree(l) ((l)->free)					//返回链表的释放函数
#define listGetMatchMethod(l) ((l)->match)			//返回链表的比较函数

/* Prototypes */
list *listCreate(void);								//创建一个表头
void listRelease(list *list);						//释放整个链表
void listEmpty(list *list);							//从链表中移除所有的元素但是没有释放自身
list *listAddNodeHead(list *list, void *value);		//将value添加到链表的头部
list *listAddNodeTail(list *list, void *value);		//将value添加到链表的尾部
list *listInsertNode(list *list, listNode *old_node, void *value, int after);//跟据after的值决定value插在old_node前面还是后面
void listDelNode(list *list, listNode *node);		//从链表中删除节点node
listIter *listGetIterator(list *list, int direction);//根据direction为链表创建一个迭代器
listNode *listNext(listIter *iter);					//返回迭代器当前所指向的节点指针，并且迭代器移动到下一个
void listReleaseIterator(listIter *iter);			//释放迭代器
list *listDup(list *orig);							//拷贝整个链表
listNode *listSearchKey(list *list, void *key);		//在链表中查找值为key的节点并返回该节点
listNode *listIndex(list *list, long index);		//返回下标为index的节点
void listRewind(list *list, listIter *li);			//将迭代器重置为list的头结点并且设置为正向迭代
void listRewindTail(list *list, listIter *li);		//将迭代器li重置为list的尾结点并且设置为反向迭代
void listRotate(list *list);						//将尾节点插到头结点前作为新的头节点
void listJoin(list *l, list *o);					//将链表o插入链表l的尾部，并将链表o重置为空链表

/* Directions for iterators */
#define AL_START_HEAD 0								//正向迭代：从表头向表尾进行迭代
#define AL_START_TAIL 1								//反向迭代：从表尾到表头进行迭代

#endif /* __ADLIST_H__ */
