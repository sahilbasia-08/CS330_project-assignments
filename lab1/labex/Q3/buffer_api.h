#ifndef BUFFER_API_H
#define BUFFER_API_H

#include <stdint.h>
#define PERF_RECORD_SAMPLE 0x1
#define PERF_RECORD_LOST 0x2
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

/*
 * Each perf_event instance in the buffer begins with a perf_event_header.
 *
 * There are two types of events: sample_event and lost_event.
 */

struct perf_event_header
{
    u32 type;
    u16 misc;
    u16 size;
};

struct sample_event
{
    struct perf_event_header header;
    u64 ip;
    u32 pid, tid;
    u64 addr;
};

struct lost_event
{
    struct perf_event_header header;
    u64 id;
    u64 lost;
};

struct ring_buffer
{
    void *data_base;
    u64 data_head;
    u64 data_tail;
    u64 data_size; // guaranteed to be a power of 2
};

// Initialize the buffer (allocate, populate, protect).
void buffer_init(int testcase);
void buffer_exit();
// Accessors students may call:
const struct ring_buffer *buffer_get_base(void);

#endif /* BUFFER_API_H */
