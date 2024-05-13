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

// ���� �޸� ���� ���� �Լ�
void SplitExistingMemory(size_t s, void* base_address, smheader_ptr current);

// mmap���� ���� �Ҵ� ���� �޸� ���� ���� �Լ�
void SplitNewMemory(size_t s, size_t new_size, smheader_ptr new_address, smheader_ptr current);
