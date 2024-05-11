#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "smalloc.h"
#include <sys/mman.h>

smheader_ptr smlist = 0x0;

void* smalloc (size_t s) 
{
	// TODO
	void* base_address = NULL;
	size_t page_size = (size_t)getpagesize(); // 4096
	
	if(smlist == 0x0) // data header list가 smlist에 존재하지 않을 경우
	{
		smlist = (smheader_ptr)mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

		if (smlist == MAP_FAILED) 
		{
			perror("smlist mmap");
			exit(EXIT_FAILURE);
    	}

		smlist->size = page_size - sizeof(smheader);
		smlist->used = 0;
		smlist->next = NULL;
	}

	while(smlist) // data header list의 마지막 data header까지 반복
	{
		if(smlist->used == 0 && smlist->size >= s) // data region이 unused이면서 size가 s 이상일 경우 선택
		{
			if(smlist->size > s + 24) // unused data region의 size가 s + 24보다 클 경우 메모리 영역을 2개로 split하고 size를 s로 update
			{
				size_t old_size = smlist->size;

				smlist->size = s;
				smlist->used = 1;
				base_address = smlist + sizeof(smheader);

				smheader_ptr old_next = smlist->next;
				smlist->next = base_address + smlist->size; // 문제 발생 의심(주소 연산)
				(smlist->next)->size = old_size - smlist->size - sizeof(smheader);
				(smlist->next)->used = 0;
				(smlist->next)->next = old_next;

				return base_address;
			}
			else // size가 s ~ s + 24일 경우
			{
				smlist->used = 1;
				base_address = smlist + sizeof(smheader);
				return base_address;
			}
		}
		if(smlist->next == NULL) // smlist가 가리키는게 마지막 data header일 경우
		{
			break;
		}
		smlist = smlist->next; // smlist가 다음 data header를 가리키도록함
	}

	// 탐색한 모든 unused data region의 메모리 공간들이 요청된 메모리 공간 s보다 부족할 경우
	// 현재 할당되어있는 메모리 영역 끝에 1개 이상의 페이지 크기만큼 메모리 공간 추가
	size_t num_page = (s / page_size) + 1;

	if((num_page * page_size) - sizeof(smheader) < s)
	{
		num_page++;
	}

	smlist->next = (smheader_ptr)mmap(smlist + sizeof(smheader) + smlist->size, num_page * page_size, 
	PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	if (smlist->next == MAP_FAILED) 
	{
		perror("smlist->next mmap");
		exit(EXIT_FAILURE);
    }

	smlist = smlist->next;

	smlist->size = (num_page * page_size) - sizeof(smheader);
	smlist->used = 0;
	smlist->next = NULL;

	// 위 while 문에 있던 로직 재사용
	if(smlist->size > s + 24) // unused data region의 size가 s + 24보다 클 경우 메모리 영역을 2개로 split하고 size를 s로 update
	{
		size_t old_size2 = smlist->size;

		smlist->size = s;
		smlist->used = 1;
		base_address = smlist + sizeof(smheader);

		smheader_ptr old_next2 = smlist->next;
		smlist->next = smlist + sizeof(smheader) + smlist->size;
		(smlist->next)->size = old_size2 - smlist->size - sizeof(smheader);
		(smlist->next)->used = 0;
		(smlist->next)->next = old_next2;

		return base_address;
	}
	else // size가 s ~ s + 24일 경우
	{
		smlist->used = 1;
		base_address = smlist + sizeof(smheader);

		return base_address;
	}
}

void* smalloc_mode (size_t s, smmode m)
{
	// TODO
	return 0x0 ;
}

void sfree (void * p) 
{
	// TODO

}

void* srealloc (void * p, size_t s) 
{
	// TODO
	return 0x0 ; // erase this 
}

void smcoalesce ()
{
	//TODO
}

void smdump () 
{
	smheader_ptr itr;

	printf("==================== used memory slots ====================\n");
	int i = 0;
	for (itr = smlist; itr != 0x0; itr = itr->next) {
		if (itr->used == 0)
			continue;

		printf("%3d:%p:%8d:", i, ((void *) itr) + sizeof(smheader), (int) itr->size);

		int j;
		char* s = ((char *) itr) + sizeof(smheader);
		for (j = 0; j < (itr->size >= 8 ? 8 : itr->size); j++) {
			printf("%02x ", s[j]);
		}
		printf("\n");
		printf("[%d]itr->next: %p\n", i, itr->next); // for debug
		i++;
	}
	printf("\n");

	printf("==================== unused memory slots ====================\n") ;
	i = 0;
	for (itr = smlist ; itr != 0x0 ; itr = itr->next, i++) {
		if (itr->used == 1)
			continue ;

		printf("%3d:%p:%8d:", i, ((void *) itr) + sizeof(smheader), (int) itr->size);

		int j;
		char * s = ((char *) itr) + sizeof(smheader);
		for (j = 0; j < (itr->size >= 8 ? 8 : itr->size); j++) {
			printf("%02x ", s[j]);
		}
		printf("\n");
		i++;
	}
	printf("\n");
}
