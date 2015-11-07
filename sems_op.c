#include "shared_mem.h"
#include "semun.h"

int main(void) {
	del_semvalue(get_sem_key(SEM_BUFFER, 1));
	del_semvalue(get_sem_key(SEM_EMPTY, 0));
	del_semvalue(get_sem_key(SEM_FULL, CIRCULAR_BUFFER_SIZE - 1));
	delete_buffers();
	return 0;
}
