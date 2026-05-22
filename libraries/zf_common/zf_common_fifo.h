#ifndef _zf_common_fifo_h_
#define _zf_common_fifo_h_

#ifdef __cplusplus
extern "C" 
{
#endif

#include "zf_common_typedef.h"

typedef enum
{
    FIFO_SUCCESS,

    FIFO_WRITE_UNDO,
    FIFO_CLEAR_UNDO,
    FIFO_BUFFER_NULL,
    FIFO_SPACE_NO_ENOUGH,
    FIFO_DATA_NO_ENOUGH,
}fifo_state_enum;

typedef enum
{
    FIFO_IDLE       = 0x00,
    FIFO_CLEAR      = 0x01,
    FIFO_WRITE      = 0x02,
    FIFO_READ       = 0x04,
}fifo_execution_enum;

typedef enum
{
    FIFO_READ_AND_CLEAN,
    FIFO_READ_ONLY,
}fifo_operation_enum;

typedef enum
{
    FIFO_DATA_8BIT,
    FIFO_DATA_16BIT,
    FIFO_DATA_32BIT,
}fifo_data_type_enum;

typedef struct
{
    uint8               execution;                                              // 执行步骤
    fifo_data_type_enum type;                                                   // 数据类型
    void               *buffer;                                                 // 缓存指针
    uint32              head;                                                   // 缓存头指针 总是指向空的缓存
    uint32              end;                                                    // 缓存尾指针 总是指向非空缓存（缓存全空除外）
    uint32              size;                                                   // 缓存剩余大小
    uint32              max;                                                    // 缓存总大小
}fifo_struct;

fifo_state_enum fifo_clear              (fifo_struct *fifo);
uint32          fifo_used               (fifo_struct *fifo);

fifo_state_enum fifo_write_element      (fifo_struct *fifo, uint32 dat);
fifo_state_enum fifo_write_buffer       (fifo_struct *fifo, void *dat, uint32 length);
fifo_state_enum fifo_read_element       (fifo_struct *fifo, void *dat, fifo_operation_enum flag);
fifo_state_enum fifo_read_buffer        (fifo_struct *fifo, void *dat, uint32 *length, fifo_operation_enum flag);
fifo_state_enum fifo_read_tail_buffer   (fifo_struct *fifo, void *dat, uint32 *length, fifo_operation_enum flag);

fifo_state_enum fifo_init               (fifo_struct *fifo, fifo_data_type_enum type, void *buffer_addr, uint32 size);

#ifdef __cplusplus
}
#endif

#endif
