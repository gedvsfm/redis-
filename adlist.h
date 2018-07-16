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
    struct listNode *prev;//ǰ���ڵ�
    struct listNode *next;//��̽ڵ�
    void *value;		//���Դ���������͵���Ϣ
} listNode;

typedef struct listIter {//����ĵ�����
    listNode *next;		//��������ǰ��ָ��Ľڵ㣨��������һ����
    int direction;		//�������ķ��򣬿���ȡ����ֵ
} listIter;

typedef struct list {
    listNode *head;		//����ͷ�ڵ�ָ��
    listNode *tail;		//����β�ڵ�ָ��
    void *(*dup)(void *ptr);	//��������ڵ��������value
    void (*free)(void *ptr);	//�ͷ�����ڵ㱣���ֵ
    int (*match)(void *ptr, void *key);//�ж�����ڵ��������ֵ�Ƿ�ʹ����ֵkey���
    unsigned long len;		//����ĳ���
} list;

/* Functions implemented as macros */
#define listLength(l) ((l)->len)					//��������ĳ���
#define listFirst(l) ((l)->head)					//���������ͷ���
#define listLast(l) ((l)->tail)						//���������β�ڵ�
#define listPrevNode(n) ((n)->prev)					//���ؽڵ��ǰ���ڵ�
#define listNextNode(n) ((n)->next)					//���ؽڵ�ĺ�̽ڵ�
#define listNodeValue(n) ((n)->value)				//���ؽڵ��ֵ

#define listSetDupMethod(l,m) ((l)->dup = (m))		//��������ĸ��ƺ���
#define listSetFreeMethod(l,m) ((l)->free = (m))	//����������ͷź���
#define listSetMatchMethod(l,m) ((l)->match = (m))	//��������ıȽϺ���

#define listGetDupMethod(l) ((l)->dup)				//��������ĸ��ƺ���
#define listGetFree(l) ((l)->free)					//����������ͷź���
#define listGetMatchMethod(l) ((l)->match)			//��������ıȽϺ���

/* Prototypes */
list *listCreate(void);								//����һ����ͷ
void listRelease(list *list);						//�ͷ���������
void listEmpty(list *list);							//���������Ƴ����е�Ԫ�ص���û���ͷ�����
list *listAddNodeHead(list *list, void *value);		//��value��ӵ������ͷ��
list *listAddNodeTail(list *list, void *value);		//��value��ӵ������β��
list *listInsertNode(list *list, listNode *old_node, void *value, int after);//����after��ֵ����value����old_nodeǰ�滹�Ǻ���
void listDelNode(list *list, listNode *node);		//��������ɾ���ڵ�node
listIter *listGetIterator(list *list, int direction);//����directionΪ������һ��������
listNode *listNext(listIter *iter);					//���ص�������ǰ��ָ��Ľڵ�ָ�룬���ҵ������ƶ�����һ��
void listReleaseIterator(listIter *iter);			//�ͷŵ�����
list *listDup(list *orig);							//������������
listNode *listSearchKey(list *list, void *key);		//�������в���ֵΪkey�Ľڵ㲢���ظýڵ�
listNode *listIndex(list *list, long index);		//�����±�Ϊindex�Ľڵ�
void listRewind(list *list, listIter *li);			//������������Ϊlist��ͷ��㲢������Ϊ�������
void listRewindTail(list *list, listIter *li);		//��������li����Ϊlist��β��㲢������Ϊ�������
void listRotate(list *list);						//��β�ڵ�嵽ͷ���ǰ��Ϊ�µ�ͷ�ڵ�
void listJoin(list *l, list *o);					//������o��������l��β������������o����Ϊ������

/* Directions for iterators */
#define AL_START_HEAD 0								//����������ӱ�ͷ���β���е���
#define AL_START_TAIL 1								//����������ӱ�β����ͷ���е���

#endif /* __ADLIST_H__ */
