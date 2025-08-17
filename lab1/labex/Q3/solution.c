#include <stdio.h>
#include <stdlib.h>
#include "buffer_api.h"
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        perror("usage: ./solution <testcase number>");
        return -1;
    }
    buffer_init(atoi(argv[1]));
    const struct ring_buffer *rb = buffer_get_base();

    char *base = (char *)rb->data_base;
    uint64_t tail = rb->data_tail;
    uint64_t head = rb->data_head;
    uint64_t size = rb->data_size;
    uint64_t unknown_events = 0;
    uint64_t lost_event = 0;
    // head = head & (size - 1);
    while (tail < head)
    {
        // since its a ring buffer () in circle we will wrap around our start point with the help of size valie
        uint64_t offset = tail % size; // tail & (size - 1);
        // now we will have a pointer for even headers to read

        struct perf_event_header *ptr = (struct perf_event_header *)(base + offset);

        if (ptr->type == PERF_RECORD_LOST)
        {
            // do work
            struct lost_event *ptr2 = (struct lost_event *)(ptr);
            lost_event += ptr2->lost;
        }
        else if (ptr->type == PERF_RECORD_SAMPLE)
        {
            // do print the things needed
            struct sample_event *ptr1 = (struct sample_event *)(ptr);
            printf("0x%llx \n", ptr1->addr);
        }
        else
        {
            // anonymous events
            unknown_events += ptr->size;
        }
        tail += ptr->size;
    }
    printf("number of lost records: %llu\n", (unsigned long long)lost_event);
    printf("unknown size: %llu\n", (unsigned long long)unknown_events);
    /* ------ YOUR CODE ENDS HERE ------*/

    /* print formats */
    // printf("0x%...."); // to print sample addresses
    // printf("number of lost records: %....");
    // printf("unknown size: %....");

    /* ------ YOUR CODE ENDS HERE ------*/
    buffer_exit();
    return 0;
}
