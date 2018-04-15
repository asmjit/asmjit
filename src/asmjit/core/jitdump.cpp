#include "jitdump.h"

#include <cstring>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>


namespace asmjit{

uint64_t JitDump::getTimestamp() const{
	timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp); //FIXME: error handling
	return (uint64_t)tp.tv_sec * 1'000'000'000 + tp.tv_nsec;
}

void JitDump::init(){
	header head;
	head.total_size = sizeof(header);
	head.pid = getpid();
	head.timestamp = getTimestamp();

	char namebuf[1024];
	snprintf(namebuf, sizeof(namebuf), "jit-%d.dump", head.pid);
	int fdi = open(namebuf, O_CREAT | O_TRUNC | O_RDWR, 0666); //FIXME: error handling
	page_size = sysconf(_SC_PAGESIZE);
	// let perf record know about the jitdump file
	marker = mmap(nullptr, page_size, PROT_READ | PROT_EXEC, MAP_PRIVATE, fdi, 0);
	//if(marker_addr == MAP_FAILED) return -1; //FIXME: throw exception
	fd = fdopen(fdi, "wb"); //FIXME: error handling

	// write file header
	fwrite(&head, sizeof(header), 1, fd);
}

void JitDump::close(){
	munmap(marker, page_size);
	fclose(fd);
}

void JitDump::addCodeSegment(const char *fn_name, void *fn, uint64_t code_size){
	size_t name_len = strlen(fn_name);
	record_header rh;
	rh.id = JIT_CODE_LOAD;
	rh.total_size = sizeof(record_header) + sizeof(record_load) + name_len+1 + code_size;
	rh.timestamp = getTimestamp();

	record_load rl;
	rl.pid = getpid();
	rl.tid = rl.pid; //FIXME: use pthread_self() to support multi-threaded applications
	rl.vma = (uint64_t)fn;
	rl.code_addr = rl.vma;
	rl.code_size = code_size;
	rl.code_index = nextid++;

	// write records
	fwrite(&rh, sizeof(record_header), 1, fd);
	fwrite(&rl, sizeof(record_load), 1, fd);
	fwrite(fn_name, name_len+1, 1, fd);
	fwrite(fn, code_size, 1, fd);
}


}
