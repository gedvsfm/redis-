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

#define SDS_MAX_PREALLOC (1024*1024)	//预先分配内存的最大的长度

#include <sys/types.h>
#include <stdarg.h>
#include <stdint.h>

typedef char *sds;			//sds兼容传统C风格的字符串，并且可以存放sdshdr结构buf成员的地址

/* Note: sdshdr5 is never used, we just access the flags byte directly.
 * However is here to document the layout of type 5 SDS strings. */
 //以下5个为sds的表头，用来存放sds的信息，这样写了5个目的是为了节省空间。其中sdshdr5几乎不被使用
struct __attribute__ ((__packed__)) sdshdr5 {
    unsigned char flags; /* 3 lsb of type, and 5 msb of string length */
    char buf[];
};
struct __attribute__ ((__packed__)) sdshdr8 {
    uint8_t len; //buf中已经使用的内存空间长度
    uint8_t alloc; //buf总共分配的内存空间长度
    unsigned char flags; //表示表头结构体是那种类型
    char buf[];//初始化sds分配的数据空间，看清楚是柔性数组
};
struct __attribute__ ((__packed__)) sdshdr16 {
    uint16_t len; //buf中已经使用的内存空间长度
    uint16_t alloc; //buf总共分配的内存空间长度
    unsigned char flags; //表示表头结构体是那种类型
    char buf[];//初始化sds分配的数据空间，看清楚是柔性数组
};
struct __attribute__ ((__packed__)) sdshdr32 {
    uint32_t len; /* used */
    uint32_t alloc; //buf总共分配的内存空间长度
    unsigned char flags; //表示表头结构体是那种类型
    char buf[];
};
struct __attribute__ ((__packed__)) sdshdr64 {
    uint64_t len; //buf中已经使用的内存空间长度
    uint64_t alloc; //buf总共分配的内存空间长度
    unsigned char flags; //表示表头结构体是那种类型
    char buf[];//初始化sds分配的数据空间，看清楚是柔性数组
};
//以下五个是表头中flag的取值
#define SDS_TYPE_5  0
#define SDS_TYPE_8  1
#define SDS_TYPE_16 2
#define SDS_TYPE_32 3
#define SDS_TYPE_64 4
#define SDS_TYPE_MASK 7	//sds类型的掩码，因为用二进制三位就可以表示5种类型，所以用7表示掩码
#define SDS_TYPE_BITS 3
#define SDS_HDR_VAR(T,s) struct sdshdr##T *sh = (void*)((s)-(sizeof(struct sdshdr##T)));//获取表头的地址，并建立一个结构体指针，s表示buf的地址，结构体是在堆上分配的所以s在高地址，减去结构体的长度就获得了表头的地址（柔性数组不占结构的大小）
#define SDS_HDR(T,s) ((struct sdshdr##T *)((s)-(sizeof(struct sdshdr##T))))//获取表头的地址
#define SDS_TYPE_5_LEN(f) ((f)>>SDS_TYPE_BITS)

static inline size_t sdslen(const sds s) {	//计算buf中字符串的长度
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

static inline size_t sdsavail(const sds s) {	//计算buf中的未使用空间的长度
    unsigned char flags = s[-1];	//flags长度为1，s[-1]为s的首地址向前偏移一个单位的取值
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

static inline void sdssetlen(sds s, size_t newlen) {//设置buf新的长度newlen
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

static inline void sdsinclen(sds s, size_t inc) {//将字符串s的长度增加inc
    unsigned char flags = s[-1];//flags长度为1，s[-1]为s的首地址向前偏移一个单位的取值
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
static inline size_t sdsalloc(const sds s) {//计算总共分配了多少内存空间
    unsigned char flags = s[-1];//flags长度为1，s[-1]为s的首地址向前偏移一个单位的取值
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

static inline void sdssetalloc(sds s, size_t newlen) {//设置新的内存空间大小为newlen
    unsigned char flags = s[-1];//flags长度为1，s[-1]为s的首地址向前偏移一个单位的取值
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

sds sdsnewlen(const void *init, size_t initlen);//创建一个长度为initlen的字符串,并保存init字符串中的值
sds sdsnew(const char *init);	//创建一个字符串
sds sdsempty(void);				//建立一个只有表头，字符串为空"\0"的字符串
sds sdsdup(const sds s);		//拷贝一份s的副本
void sdsfree(sds s);			 //释放s字符串和表头
sds sdsgrowzero(sds s, size_t len);//将s扩展到指定长度，并赋值为0
sds sdscatlen(sds s, const void *t, size_t len);//将t追加到s的末尾，追加长度为len
sds sdscat(sds s, const char *t);//将字符串t拼接到s的末尾
sds sdscatsds(sds s, const sds t);//将t追加到s的末尾
sds sdscpylen(sds s, const char *t, size_t len);//将字符串t覆盖到s表头的buf中，拷贝len个字节
sds sdscpy(sds s, const char *t);//将字符串t覆盖到s表头的buf中

sds sdscatvprintf(sds s, const char *fmt, va_list ap);//打印函数，被 sdscatprintf 所调用
#ifdef __GNUC__
sds sdscatprintf(sds s, const char *fmt, ...) //打印任意数量个字符串，并将这些字符串追加到给定 sds 的末尾
    __attribute__((format(printf, 2, 3)));
#else
sds sdscatprintf(sds s, const char *fmt, ...);//打印任意数量个字符串，并将这些字符串追加到给定 sds 的末尾
#endif

sds sdscatfmt(sds s, char const *fmt, ...);//格式化打印多个字符串，并将这些字符串追加到给定 sds 的末尾
sds sdstrim(sds s, const char *cset);//去除sds头部和尾部中包含有cset字符串出现字符的字符
void sdsrange(sds s, ssize_t start, ssize_t end);//根据start和end区间截取字符串
void sdsupdatelen(sds s);//更新字符串s的长度
void sdsclear(sds s);//将字符串重置保存空间，懒惰释放
int sdscmp(const sds s1, const sds s2);//比较两个sds的大小
sds *sdssplitlen(const char *s, ssize_t len, const char *sep, int seplen, int *count);//使用长度为seplen的sep分隔符对长度为len的s进行分割，返回一个sds数组的地址，*count被设置为数组元素数量
void sdsfreesplitres(sds *tokens, int count);//释放tokens中的count个元素
void sdstolower(sds s);//将sds字符串所有字符转换为小写
void sdstoupper(sds s);//将sds字符串所有字符转换为大写
sds sdsfromlonglong(long long value);//根据long long value创建一个sds
sds sdscatrepr(sds s, const char *p, size_t len);//将长度为len的字符串p以带引号""的格式追加到s末尾
sds *sdssplitargs(const char *line, int *argc);//参数拆分,主要用于 config.c 中对配置文件进行分析。
sds sdsmapchars(sds s, const char *from, const char *to, size_t setlen);//将s中所有在 from 中的字符串，替换成 to 中的字符串
sds sdsjoin(char **argv, int argc, char *sep);//以分隔符连接字符串子数组构成新的字符串
sds sdsjoinsds(sds *argv, int argc, const char *sep, size_t seplen);

/* Low level functions exposed to the user API */
sds sdsMakeRoomFor(sds s, size_t addlen);//对 sds 中 buf 的长度进行扩展
void sdsIncrLen(sds s, ssize_t incr);//根据incr的正负，移动字符串末尾的'\0'标志
sds sdsRemoveFreeSpace(sds s);//回收sds中的未使用空间
size_t sdsAllocSize(sds s);//获得sds所有分配的空间
void *sdsAllocPtr(sds s);//以长度为seplen的分隔符sep连接一个sds数组

/* Export the allocator used by SDS to the program using SDS.
 * Sometimes the program SDS is linked to, may use a different set of
 * allocators, but may want to allocate or free things that SDS will
 * respectively free or allocate. */
 //内存分配和释放函数
void *sds_malloc(size_t size);
void *sds_realloc(void *ptr, size_t size);
void sds_free(void *ptr);

#ifdef REDIS_TEST
int sdsTest(int argc, char *argv[]);//测试函数
#endif

#endif
