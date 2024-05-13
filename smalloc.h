struct _smheader {
	size_t size;
	uint8_t used;
	struct _smheader * next;
};

typedef struct _smheader smheader;
typedef struct _smheader* smheader_ptr;

typedef 
	enum {
		bestfit, worstfit, firstfit
	} 
	smmode ;

void * smalloc (size_t s) ;
void * smalloc_mode (size_t s, smmode m) ;

void sfree (void * p) ;

void * srealloc (void * p, size_t s) ;

void smcoalesce () ;

void smdump () ;

// 기존 메모리 영역 분할 함수
void SplitExistingMemory(size_t s, void* base_address, smheader_ptr current);

// mmap으로 새로 할당 받은 메모리 영역 분할 함수
void SplitNewMemory(size_t s, size_t new_size, smheader_ptr new_address, smheader_ptr current);
