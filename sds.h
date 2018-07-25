/* SDSLib 2.0 -- A C dynamic strings library
 *
 * Copyright (c) 2006-2015, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2015, Oran Agra
 * Copyright (c) 2015, Redis Labs, Inc
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

#ifndef __SDS_H
#define __SDS_H

#define SDS_MAX_PREALLOC (1024*1024)	//Ԥ�ȷ����ڴ�����ĳ���

#include <sys/types.h>
#include <stdarg.h>
#include <stdint.h>

typedef char *sds;			//sds���ݴ�ͳC�����ַ��������ҿ��Դ��sdshdr�ṹbuf��Ա�ĵ�ַ

/* Note: sdshdr5 is never used, we just access the flags byte directly.
 * However is here to document the layout of type 5 SDS strings. */
 //����5��Ϊsds�ı�ͷ���������sds����Ϣ������д��5��Ŀ����Ϊ�˽�ʡ�ռ䡣����sdshdr5��������ʹ��
struct __attribute__ ((__packed__)) sdshdr5 {
    unsigned char flags; /* 3 lsb of type, and 5 msb of string length */
    char buf[];
};
struct __attribute__ ((__packed__)) sdshdr8 {
    uint8_t len; //buf���Ѿ�ʹ�õ��ڴ�ռ䳤��
    uint8_t alloc; //buf�ܹ�������ڴ�ռ䳤��
    unsigned char flags; //��ʾ��ͷ�ṹ������������
    char buf[];//��ʼ��sds��������ݿռ䣬���������������
};
struct __attribute__ ((__packed__)) sdshdr16 {
    uint16_t len; //buf���Ѿ�ʹ�õ��ڴ�ռ䳤��
    uint16_t alloc; //buf�ܹ�������ڴ�ռ䳤��
    unsigned char flags; //��ʾ��ͷ�ṹ������������
    char buf[];//��ʼ��sds��������ݿռ䣬���������������
};
struct __attribute__ ((__packed__)) sdshdr32 {
    uint32_t len; /* used */
    uint32_t alloc; //buf�ܹ�������ڴ�ռ䳤��
    unsigned char flags; //��ʾ��ͷ�ṹ������������
    char buf[];
};
struct __attribute__ ((__packed__)) sdshdr64 {
    uint64_t len; //buf���Ѿ�ʹ�õ��ڴ�ռ䳤��
    uint64_t alloc; //buf�ܹ�������ڴ�ռ䳤��
    unsigned char flags; //��ʾ��ͷ�ṹ������������
    char buf[];//��ʼ��sds��������ݿռ䣬���������������
};
//��������Ǳ�ͷ��flag��ȡֵ
#define SDS_TYPE_5  0
#define SDS_TYPE_8  1
#define SDS_TYPE_16 2
#define SDS_TYPE_32 3
#define SDS_TYPE_64 4
#define SDS_TYPE_MASK 7	//sds���͵����룬��Ϊ�ö�������λ�Ϳ��Ա�ʾ5�����ͣ�������7��ʾ����
#define SDS_TYPE_BITS 3
#define SDS_HDR_VAR(T,s) struct sdshdr##T *sh = (void*)((s)-(sizeof(struct sdshdr##T)));//��ȡ��ͷ�ĵ�ַ��������һ���ṹ��ָ�룬s��ʾbuf�ĵ�ַ���ṹ�����ڶ��Ϸ��������s�ڸߵ�ַ����ȥ�ṹ��ĳ��Ⱦͻ���˱�ͷ�ĵ�ַ���������鲻ռ�ṹ�Ĵ�С��
#define SDS_HDR(T,s) ((struct sdshdr##T *)((s)-(sizeof(struct sdshdr##T))))//��ȡ��ͷ�ĵ�ַ
#define SDS_TYPE_5_LEN(f) ((f)>>SDS_TYPE_BITS)

static inline size_t sdslen(const sds s) {	//����buf���ַ����ĳ���
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5:
            return SDS_TYPE_5_LEN(flags);
        case SDS_TYPE_8:
            return SDS_HDR(8,s)->len;
        case SDS_TYPE_16:
            return SDS_HDR(16,s)->len;
        case SDS_TYPE_32:
            return SDS_HDR(32,s)->len;
        case SDS_TYPE_64:
            return SDS_HDR(64,s)->len;
    }
    return 0;
}

static inline size_t sdsavail(const sds s) {	//����buf�е�δʹ�ÿռ�ĳ���
    unsigned char flags = s[-1];	//flags����Ϊ1��s[-1]Ϊs���׵�ַ��ǰƫ��һ����λ��ȡֵ
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5: {
            return 0;
        }
        case SDS_TYPE_8: {
            SDS_HDR_VAR(8,s);
            return sh->alloc - sh->len;
        }
        case SDS_TYPE_16: {
            SDS_HDR_VAR(16,s);
            return sh->alloc - sh->len;
        }
        case SDS_TYPE_32: {
            SDS_HDR_VAR(32,s);
            return sh->alloc - sh->len;
        }
        case SDS_TYPE_64: {
            SDS_HDR_VAR(64,s);
            return sh->alloc - sh->len;
        }
    }
    return 0;
}

static inline void sdssetlen(sds s, size_t newlen) {//����buf�µĳ���newlen
    unsigned char flags = s[-1];
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5:
            {
                unsigned char *fp = ((unsigned char*)s)-1;
                *fp = SDS_TYPE_5 | (newlen << SDS_TYPE_BITS);
            }
            break;
        case SDS_TYPE_8:
            SDS_HDR(8,s)->len = newlen;
            break;
        case SDS_TYPE_16:
            SDS_HDR(16,s)->len = newlen;
            break;
        case SDS_TYPE_32:
            SDS_HDR(32,s)->len = newlen;
            break;
        case SDS_TYPE_64:
            SDS_HDR(64,s)->len = newlen;
            break;
    }
}

static inline void sdsinclen(sds s, size_t inc) {//���ַ���s�ĳ�������inc
    unsigned char flags = s[-1];//flags����Ϊ1��s[-1]Ϊs���׵�ַ��ǰƫ��һ����λ��ȡֵ
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5:
            {
                unsigned char *fp = ((unsigned char*)s)-1;
                unsigned char newlen = SDS_TYPE_5_LEN(flags)+inc;
                *fp = SDS_TYPE_5 | (newlen << SDS_TYPE_BITS);
            }
            break;
        case SDS_TYPE_8:
            SDS_HDR(8,s)->len += inc;
            break;
        case SDS_TYPE_16:
            SDS_HDR(16,s)->len += inc;
            break;
        case SDS_TYPE_32:
            SDS_HDR(32,s)->len += inc;
            break;
        case SDS_TYPE_64:
            SDS_HDR(64,s)->len += inc;
            break;
    }
}

/* sdsalloc() = sdsavail() + sdslen() */
static inline size_t sdsalloc(const sds s) {//�����ܹ������˶����ڴ�ռ�
    unsigned char flags = s[-1];//flags����Ϊ1��s[-1]Ϊs���׵�ַ��ǰƫ��һ����λ��ȡֵ
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5:
            return SDS_TYPE_5_LEN(flags);
        case SDS_TYPE_8:
            return SDS_HDR(8,s)->alloc;
        case SDS_TYPE_16:
            return SDS_HDR(16,s)->alloc;
        case SDS_TYPE_32:
            return SDS_HDR(32,s)->alloc;
        case SDS_TYPE_64:
            return SDS_HDR(64,s)->alloc;
    }
    return 0;
}

static inline void sdssetalloc(sds s, size_t newlen) {//�����µ��ڴ�ռ��СΪnewlen
    unsigned char flags = s[-1];//flags����Ϊ1��s[-1]Ϊs���׵�ַ��ǰƫ��һ����λ��ȡֵ
    switch(flags&SDS_TYPE_MASK) {
        case SDS_TYPE_5:
            /* Nothing to do, this type has no total allocation info. */
            break;
        case SDS_TYPE_8:
            SDS_HDR(8,s)->alloc = newlen;
            break;
        case SDS_TYPE_16:
            SDS_HDR(16,s)->alloc = newlen;
            break;
        case SDS_TYPE_32:
            SDS_HDR(32,s)->alloc = newlen;
            break;
        case SDS_TYPE_64:
            SDS_HDR(64,s)->alloc = newlen;
            break;
    }
}

sds sdsnewlen(const void *init, size_t initlen);//����һ������Ϊinitlen���ַ���,������init�ַ����е�ֵ
sds sdsnew(const char *init);	//����һ���ַ���
sds sdsempty(void);				//����һ��ֻ�б�ͷ���ַ���Ϊ��"\0"���ַ���
sds sdsdup(const sds s);		//����һ��s�ĸ���
void sdsfree(sds s);			 //�ͷ�s�ַ����ͱ�ͷ
sds sdsgrowzero(sds s, size_t len);//��s��չ��ָ�����ȣ�����ֵΪ0
sds sdscatlen(sds s, const void *t, size_t len);//��t׷�ӵ�s��ĩβ��׷�ӳ���Ϊlen
sds sdscat(sds s, const char *t);//���ַ���tƴ�ӵ�s��ĩβ
sds sdscatsds(sds s, const sds t);//��t׷�ӵ�s��ĩβ
sds sdscpylen(sds s, const char *t, size_t len);//���ַ���t���ǵ�s��ͷ��buf�У�����len���ֽ�
sds sdscpy(sds s, const char *t);//���ַ���t���ǵ�s��ͷ��buf��

sds sdscatvprintf(sds s, const char *fmt, va_list ap);//��ӡ�������� sdscatprintf ������
#ifdef __GNUC__
sds sdscatprintf(sds s, const char *fmt, ...) //��ӡ�����������ַ�����������Щ�ַ���׷�ӵ����� sds ��ĩβ
    __attribute__((format(printf, 2, 3)));
#else
sds sdscatprintf(sds s, const char *fmt, ...);//��ӡ�����������ַ�����������Щ�ַ���׷�ӵ����� sds ��ĩβ
#endif

sds sdscatfmt(sds s, char const *fmt, ...);//��ʽ����ӡ����ַ�����������Щ�ַ���׷�ӵ����� sds ��ĩβ
sds sdstrim(sds s, const char *cset);//ȥ��sdsͷ����β���а�����cset�ַ��������ַ����ַ�
void sdsrange(sds s, ssize_t start, ssize_t end);//����start��end�����ȡ�ַ���
void sdsupdatelen(sds s);//�����ַ���s�ĳ���
void sdsclear(sds s);//���ַ������ñ���ռ䣬�����ͷ�
int sdscmp(const sds s1, const sds s2);//�Ƚ�����sds�Ĵ�С
sds *sdssplitlen(const char *s, ssize_t len, const char *sep, int seplen, int *count);//ʹ�ó���Ϊseplen��sep�ָ����Գ���Ϊlen��s���зָ����һ��sds����ĵ�ַ��*count������Ϊ����Ԫ������
void sdsfreesplitres(sds *tokens, int count);//�ͷ�tokens�е�count��Ԫ��
void sdstolower(sds s);//��sds�ַ��������ַ�ת��ΪСд
void sdstoupper(sds s);//��sds�ַ��������ַ�ת��Ϊ��д
sds sdsfromlonglong(long long value);//����long long value����һ��sds
sds sdscatrepr(sds s, const char *p, size_t len);//������Ϊlen���ַ���p�Դ�����""�ĸ�ʽ׷�ӵ�sĩβ
sds *sdssplitargs(const char *line, int *argc);//�������,��Ҫ���� config.c �ж������ļ����з�����
sds sdsmapchars(sds s, const char *from, const char *to, size_t setlen);//��s�������� from �е��ַ������滻�� to �е��ַ���
sds sdsjoin(char **argv, int argc, char *sep);//�Էָ��������ַ��������鹹���µ��ַ���
sds sdsjoinsds(sds *argv, int argc, const char *sep, size_t seplen);

/* Low level functions exposed to the user API */
sds sdsMakeRoomFor(sds s, size_t addlen);//�� sds �� buf �ĳ��Ƚ�����չ
void sdsIncrLen(sds s, ssize_t incr);//����incr���������ƶ��ַ���ĩβ��'\0'��־
sds sdsRemoveFreeSpace(sds s);//����sds�е�δʹ�ÿռ�
size_t sdsAllocSize(sds s);//���sds���з���Ŀռ�
void *sdsAllocPtr(sds s);//�Գ���Ϊseplen�ķָ���sep����һ��sds����

/* Export the allocator used by SDS to the program using SDS.
 * Sometimes the program SDS is linked to, may use a different set of
 * allocators, but may want to allocate or free things that SDS will
 * respectively free or allocate. */
 //�ڴ������ͷź���
void *sds_malloc(size_t size);
void *sds_realloc(void *ptr, size_t size);
void sds_free(void *ptr);

#ifdef REDIS_TEST
int sdsTest(int argc, char *argv[]);//���Ժ���
#endif

#endif
