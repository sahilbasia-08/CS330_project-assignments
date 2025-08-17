#include "buffer_api.h"
#include <stdio.h>

int main(){
	int perf_event_header_size = sizeof(struct perf_event_header);
	int sample_event_size = sizeof(struct sample_event);
	int lost_event_size = sizeof(struct lost_event);
	printf("%d %d %d\n", perf_event_header_size, sample_event_size, lost_event_size);
}
