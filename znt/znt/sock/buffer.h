/**
 * MIT License
 *
 * Copyright (c) 2018 Z.Riemann
 * https://github.com/ZRiemann/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the Software), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM
 * , OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef _Z_BUFFER_H_
#define _Z_BUFFER_H_

/**
 * @file buffer.h
 * @brief generic buffer
 * @author Z.Riemann https://github.com/ZRiemann/
 * @date 2018-12-27 Z.Riemann found
 *
 * The malloc() and calloc() functions return a pointer to the allocated memory,
 * which  is suitably  aligned  for  any built-in type.
 *
 * @advantage
 *  1. faster than malloc()
 *  2. statistic memory allocates
 * @sample see test/tst_buffer.c
 *
 *
 * buffer header
 * |<- sizeof(void*) --->|
 * [stack-index|stack-pos][payload]
 *
 * buffer table
 * i         id0  id1 ...
 * 0  stream 8192->8192->...
 *           (busy)         pos(idle[0])          top(idle[n+1])
 * 1  2      [i,id]...[i,id][i,id][i,id]...[i,id]
 * 2  4
 * 3  8
 * 4  16
 * 5  32
 * 6  64
 * 7  128
 * 8  256
 * 9  512
 * 10 1024
 * 11 2048
 * 12 4096
 * 13 8192
 * 14 >8192  [id|pos|size][payload]
 *
 * pop buffer pos=5 top=10 max=1024
 *   stack[pos];
 *   ++pos;
 * push buffer i=5,id=3,pos=7,top=10,max=1024
 *   --pos;
 *   swap_pos(3,pos);
 */
#include <stdlib.h>
#include <zsi/base/type.h>
#include <zsi/thread/spin.h>

#define ENABLE_TRACE 0
#if ENABLE_TRACE
#include <zsi/base/trace.h>
#endif

#define ENABLE_REFER 1
#define ENABLE_DATA_POS 1
#define ENABLE_LIST 1 // buffer list for large buffers
#define BASE_HEAD 1

ZC_BEGIN

#define ZBUF_STACK_SIZE 17
#define ZBUF_MAX_VALUE 16384
#define ZBUF_STACK_MASK 0xff000000
#define ZBUF_MAX_SIZE   0x00ffffff

typedef struct zbuffer_header_s{
#if ENABLE_REFER
    zatm32_t ref; /** buffer refers */
#endif
    uint32_t pos; /** idle position*/
    uint32_t size; /* buffer size */
#if ENABLE_DATA_POS
    uint32_t data_pos; /* payload free position */
    unit32_t end_pos;
#endif
#if ENABLE_LIST
    struct zbuffer_header_s *prev;
    struct zbuffer_header_s *next;
    uint32_t list_size;
#endif
    char payload[0];
}zbuf_head_t;

zinline uint8_t zbuf_head_idx(zbuf_head_t *h){
    return h->pos >> 24;
}

zinline size_t zbuf_head_pos(zbuf_head_t *h){
    return h->pos & ZBUF_MAX_SIZE;
}

zinline void zbuf_head_set(zbuf_head_t *h, uint8_t idx, size_t pos){
    h->pos = (idx << 24) | pos;
}

typedef struct zbuffer_stack_s{
    size_t pos;
    size_t top;
    size_t max;
    zbuf_head_t** buf; /** buffer array */
    /* TODO: statistic */
}zbuf_stack_t;

typedef struct zbuffer_s{
    zbuf_stack_t stacks[ZBUF_STACK_SIZE]; /** 2 ~ 8192 */
    zspinlock_t spins[ZBUF_STACK_SIZE]; /** maybe realloc */
}zbuf_t;

zinline zbuf_t* zbuf_create(){
    zbuf_t *buf = 0;
    buf = calloc(1, sizeof(zbuf_t));
    if(buf){
        int i = 0;
        zbuf_stack_t* stack = 0;
        for(i = 0; i < ZBUF_STACK_SIZE; ++i){
           stack = &buf->stacks[i];
           stack->max = 8;
           stack->buf = calloc(stack->max, sizeof(zbuf_head_t*));
           if(!stack->buf){
               /* memory insufficient */
               while(--i > 0){
                   zspin_fini(&buf->spins[i]);
                   free(buf->stacks[i].buf);
               }
               free(buf);
               buf = 0;
               return buf;
           }
           zspin_init(&buf->spins[i]);
        }
    }
    return buf;
}

zinline void zbuf_destroy(zbuf_t* buf){
    int i = 0;
    int j = 0;
    if(buf){
        for(i=0; i < ZBUF_STACK_SIZE; ++i){
            for(j=0; j < buf->stacks[i].top; ++j){
                free(buf->stacks[i].buf[j]);
            }
            free(buf->stacks[i].buf);
            zspin_fini(&buf->spins[i]);
        }
        free(buf);
    }
}
#if BASE_HEAD
zinline zbuf_head_t *zbuf_stack_alloc(zbuf_t* buf, uint8_t idx, size_t size){
#else
zinline zptr_t zbuf_stack_alloc(zbuf_t* buf, uint8_t idx, size_t size){
#endif
    zbuf_stack_t *stack = &buf->stacks[idx];
    zbuf_head_t *head = NULL;
    zptr_t ptr = NULL;

    zspin_lock(&buf->spins[idx]);
    if((stack->top + 1) >= stack->max){
        /* extend stack size */
        int max = stack->max;
        if(ZBUF_MAX_SIZE <= max){
            zspin_unlock(&buf->spins[idx]);
            return NULL;
        }else{
            zptr_t new_stack = NULL;
            max = (max < ZBUF_MAX_VALUE) ? max << 1 : max + ZBUF_MAX_VALUE;
            if(max > ZBUF_MAX_SIZE){
                max = ZBUF_MAX_SIZE;
            }
            new_stack = realloc((zptr_t)stack->buf, max * sizeof(zptr_t));
            if(new_stack){
                stack->max = max;
                stack->buf = new_stack;
            }else{
                zspin_unlock(&buf->spins[idx]);
                return NULL;
            }
        }
    }

    if(stack->top > stack->pos){
        /* pop stack */
        head = stack->buf[stack->pos];
        ptr = head->payload;
#if ENABLE_DATA_POS
        head->data_pos = 0;
#endif
#if ENABLE_REFER
        zatm_xchg(&(head->ref), 0);
#endif
#if ENABLE_LIST
        head->prev = NULL;
        head->next = NULL;
        head->list_size = 1;
#endif
        if(size > ZBUF_MAX_VALUE){
            /* only keep 1 idle */
            if(head->size >= size){
                ++stack->pos; /* just pop stack */
            }else{
                /* allocate new memory and push it */
                zbuf_head_t *head1 = (zbuf_head_t*)calloc(1, size + sizeof(zbuf_head_t));
                if(head1){
                    head1->size = size;
                    zbuf_head_set(head1, idx, stack->pos);
                    stack->buf[stack->pos] = head1;
                    ++stack->pos;

                    zbuf_head_set(head, idx, stack->pos);
                    stack->buf[stack->pos] = head;
                    ++stack->top;
                    ptr = head1->payload;
                }
            }
        }else{
            ++stack->pos; /* pop the top idle memory */
        }
    }else{
        /* allocate memory
         * push into stack
         */
        head = calloc(1, size + sizeof(zbuf_head_t));
        if(head){
            zbuf_head_set(head, idx, stack->pos);
            head->size = size;
            ptr = head->payload;

            stack->buf[stack->pos] = head;
            ++stack->pos;
            ++stack->top;
        }
    }
    zspin_unlock(&buf->spins[idx]);
#if BASE_HEAD
    return head;
#else
    return ptr;
#endif
}

/* zinline zptr_t zbuf_alloc_trace(zbuf_t* buf, zize_t len,
                                   const char* file ...);
*/
#if BASE_HEAD
zinline zbuf_head_t *zbuf_alloc(zbuf_t* buf, size_t len){
#else
zinline zptr_t zbuf_alloc(zbuf_t* buf, size_t len){
#endif
    if(len > ZBUF_MAX_VALUE){
        /* [size|id|pos][payload] */
        return zbuf_stack_alloc(buf, ZBUF_STACK_SIZE - 1, len);
    }else{
        /* [id|pos][payload]*/
        size_t i = 0;
        size_t size = 1;
        for(i=1; i < (ZBUF_STACK_SIZE - 1); ++i){
            size <<= 1;
            if(len <= size){
                return zbuf_stack_alloc(buf, i, size);
            }
        }
    }
    return 0;
}

zinline zptr_t zbuf_realloc(zbuf_t *buf, zptr_t ptr, size_t size){
    return 0;
}
#if BASE_HEAD
zinline void zbuf_free(zbuf_t *buf, zptr_t ptr){
    /*if(!ptr || !buf){return;}*/
    zbuf_head_t *head = (zbuf_head_t*)zmember2st(zbuf_head_t, payload, ptr);
#else
zinline void zbuf_free(zbuf_t *buf, zbuf_head_t *head){
#endif
    uint8_t idx = zbuf_head_idx(head);
    zbuf_stack_t* stack = &buf->stacks[idx];
    int pos1 = zbuf_head_pos(head);
#if ENABLE_REFER
    if(-1 >= header->ref || -1 != zatm_dec(&(head->ref))){
        return; /* handle free */
    }
#endif
    zspin_lock(&buf->spins[idx]);
    if(stack->pos > 0){
        --stack->pos;
    }
    if(stack->pos > pos1){
        if(stack->pos > 0){
            /* swap pos */
            zbuf_head_set(stack->buf[stack->pos], idx, pos1);
            stack->buf[pos1] = stack->buf[stack->pos];
            zbuf_head_set(head, idx, stack->pos);
            stack->buf[stack->pos] = head;
        }
        if((idx == (ZBUF_STACK_SIZE - 1))
           && (stack->top > (stack->pos + 1))){
            --stack->top;
            free(stack->buf[stack->top]);
        }
    }
#if 0
    else if(stack->pos != pos1){
        printf("impossible, maybe something error!");
    }
#endif
    zspin_unlock(&buf->spins[idx]);
}

#if ENABLE_REFER
#if BASE_HEAD
zinline void zbuf_refer(zptr_t *buf){
    zbuf_head_t *head = (zbuf_head_t*)zmember2st(zbuf_head_t, payload, ptr);
#else
zinline void zbuf_refer(zbuf_head_t *head){
#endif
    zatm_inc(&(head->ref));
}

#if BASE_HEAD
zinline void zbuf_add_refer(zptr_t *buf, int n){
    zbuf_head_t *head = (zbuf_head_t*)zmember2st(zbuf_head_t, payload, ptr);
#else
zinline void zbuf_add_refer(zbuf_head_t *head, int n){
#endif
    zatm_add(&(head->ref), n);
}
#endif

#if ENABLE_LIST
zinline void zbuf_push(zbuf_head_t *head, zbuf_head_t *value){
    // test head,value
    zbuf_head_t *tail = head->prev ? head->prev : head;
    tail->next = value;
    value->prev = tail;
    value->next = head;
    head->prev = value;
    ++head->list_size;
    //head->prev = value;
}
zinline zbuf_head_t *zbuf_pop(zbuf_head_t **phead){
    zbuf_head_t *head = *phead;
    if(head->prev == head){
        /* only one item */
        head->prev = NULL;
        head->next = NULL;
        return head;
    }else if(head->prev = NULL){
        /* empty list */
        return NULL;
    }else{
        /* more items*/
        *phead = head->next;
        (*phead)->prev = head->prev;
        head->prev->next = (*phead);
        (*phead)->list = head->list_size - 1;
        return head;
    }
    return NULL;
}
zinline void zbuf_free_list(zbuf_t *buf, zbuf_head_t *head){
#if ENABLE_REFER
    //TODO: if(0 == zatm_dec(&(head->ref)));
#endif
    if(head->prev){
        zbuf_head_t ptr;
        head->prev->next = NULL;
        while(head){
            ptr = head->next;
            zbuf_free(buf, head);
            head = ptr;
        }
    }else{
        zbuf_free(buf, head);
    }
}
#endif
//zinline void zbuf_dump(zbuf_t *buf);
zinline zptr_t zbuf_stream_alloc(size_t size){
    return 0;
}
zinline zerr_t zbuf_stream_write(zptr_t data, size_t len){
    return ZENOT_SUPPORT;
}
zinline zerr_t zbuf_stream_read(zptr_t data, size_t len){
    return ZENOT_SUPPORT;
}
ZC_END
#endif /*_Z_BUFFER_H_*/
