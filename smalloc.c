#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "smalloc.h"
#include <sys/mman.h>

smheader_ptr smlist = 0x0;
smheader_ptr current = 0x0;

void* smalloc (size_t s) 
{
	// TODO
	void* base_address = NULL; // data region 시작 주소
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

		current = smlist;

	while(current) // data header list의 마지막 data header까지 반복
	{
		if(current->used == 0 && current->size >= s) // data region이 unused이면서 size가 s 이상일 경우 선택
		{
			if(current->size > s + 24) // unused data region의 size가 s + 24보다 클 경우 메모리 영역을 2개로 split하고 size를 s로 update
			{
				base_address = (void*)current + sizeof(smheader); // **주소 연산**
				SplitExistingMemory(s, base_address, current);

				return base_address;
			}
			else // size가 s ~ s + 24일 경우
			{
				current->used = 1;
				base_address = (void*)current + sizeof(smheader); // **주소 연산**
				return base_address;
			}
		}
		if(current->next == NULL) // current가 가리키는게 마지막 data header일 경우
		{
			break;
		}
		current = current->next; // current가 다음 data header를 가리키도록함
	}

	// 탐색한 모든 unused data region의 메모리 공간들이 요청된 메모리 공간 s보다 부족할 경우
	// 현재 할당되어있는 메모리 영역 끝에 1개 이상의 페이지 크기만큼 메모리 공간 추가
	size_t num_page = (s / page_size) + 1;

	if((num_page * page_size) - sizeof(smheader) < s)
	{
		num_page++;
	}

	size_t new_size = num_page * page_size;

	// mmap으로 새로운 메모리 공간을 할당 받을 때 새 메모리 영역의 시작 주소를 저장하는 헤더 포인터
	smheader_ptr new_address = NULL;

	// 현재(마지막 data region)current가 가리키는 메모리 영역 끝에 mmap을 통해 할당받은 메모리 공간 이어 붙이기
	new_address = mmap((void *)current + sizeof(smheader) + current->size, new_size, 
	PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0); // (!)문제 발생: 주소 연산

	if (new_address == MAP_FAILED) 
	{
		perror("current->next mmap");
		exit(EXIT_FAILURE);
    }

	base_address = (void*)new_address + sizeof(smheader); // **주소 연산**
	SplitNewMemory(s, new_size, new_address, current);

	/** 메모리 병합 로직에서 다시 사용 예정**/

	// 새 메모리 영역을 추가 할당받기 전 기존 메모리 영역의 data region 크기
	// size_t old_size2 = current->size;

	// current->size = current->size + num_page * page_size; // current->size를 할당 받은 메모리 공간 만큼 더해서 갱신
	// base_address = (void*)current + sizeof(smheader); // **주소 연산**
	// SplitNewMemory(s, old_size2, base_address, current, new_address);

	return base_address;
}

void* smalloc_mode (size_t s, smmode m)
{
	// TODO
	void* base_address = NULL; // data region 시작 주소

	smheader_ptr target = NULL; // 특정 조건(bestfit, worstfit, firstfit)을 만족하는 빈 공간의 헤더 포인터

	size_t target_size = 0;

	current = smlist;
	while(current)
	{
		switch(m)
		{
			case bestfit:
				if(current->used == 0 && current->size >= s)
				{
					if(target_size < s) // 최초로 탐색된 s 이상의 메모리 공간일 경우
					{
						target_size = current->size;
						target = current;
					}
					else
					{
						if(current->size < target_size)
						{
							target_size = current->size;
							target = current;
						}
					}
				}
			break;
			case worstfit:
				if(current->used == 0 && current->size >= s)
				{
					if(target_size < s) // 최초로 탐색된 s 이상의 메모리 공간일 경우
					{
						target_size = current->size;
						target = current;
					}
					else
					{
						if(current->size > target_size)
						{
							target_size = current->size;
							target = current;
						}
					}
				}
			break;
			case firstfit:
				if(current->used == 0 && current->size >= s)
				{
					target_size = current->size;

					if(target_size > s + 24) // target_size가 s + 24보다 클 경우 메모리 영역을 2개로 split하고 size를 s로 update
					{
						base_address = (void*)target + sizeof(smheader); // **주소 연산**
						SplitExistingMemory(s, base_address, target);

						return base_address;
					}
					else // target_size가 s ~ s + 24일 경우
					{
						target->used = 1;
						base_address = (void*)target + sizeof(smheader); // **주소 연산**
						return base_address;
					}
				}
			break;
		}
		if(current->next = NULL) // current가 가리키는게 마지막 data header일 경우
		{
			break;
		}
		current = current->next;
	}

	if(target_size > s + 24) // target_size가 s + 24보다 클 경우 메모리 영역을 2개로 split하고 size를 s로 update
	{
		base_address = (void*)target + sizeof(smheader); // **주소 연산**
		SplitExistingMemory(s, base_address, target);

		return base_address;
	}
	else if(target_size >= s && target_size <= s + 24)// target_size가 s ~ s + 24일 경우
	{
		target->used = 1;
		base_address = (void*)target + sizeof(smheader); // **주소 연산**
		return base_address;
	}
	else // size가 s 이상인 적절한 빈 공간을 찾지 못했을 경우
	{
		return smalloc(s);
	}
}

void sfree (void* p) 
{
	// TODO
	void* base_address = NULL; // data region 시작 주소
	current = smlist;

	while(current) // p가 가리키는 주소와 smlist 내에서 data region을 가리키는 주소들 중에 일치하는게 있는지 확인
	{
		base_address = (void*)current + sizeof(smheader); // **주소연산**
		if(base_address == p)
		{
			break;
		}
		else
		{
			if(current->next == NULL) // 마지막 data region까지 탐색해도 p가 가리키는 주소와 일치하는 주소가 없으면 abort()
			{
				abort();
			}
			current = current->next;
		}
	}

	smheader_ptr header = p - sizeof(smheader); // **주소연산**

	header->used = 0;
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
	for (itr = smlist; itr != 0x0; itr = itr->next, i++) {
		if (itr->used == 0)
			continue;

		printf("%3d:%p:%8d:", i, ((void *) itr) + sizeof(smheader), (int) itr->size);

		int j;
		char* s = ((char *) itr) + sizeof(smheader);
		for (j = 0; j < (itr->size >= 8 ? 8 : itr->size); j++) {
			printf("%02x ", s[j]);
		}
		printf("\n");
	}
	printf("\n");

	printf("==================== unused memory slots ====================\n") ;
	i = 0;
	for (itr = smlist; itr != 0x0; itr = itr->next, i++) {
		if (itr->used == 1)
			continue ;

		printf("%3d:%p:%8d:", i, ((void *) itr) + sizeof(smheader), (int) itr->size);

		int j;
		char * s = ((char *) itr) + sizeof(smheader);
		for (j = 0; j < (itr->size >= 8 ? 8 : itr->size); j++) {
			printf("%02x ", s[j]);
		}
		printf("\n");
	}
	printf("\n");
}
// 기존 메모리 영역 분할 함수
void SplitExistingMemory(size_t s, void* base_address, smheader_ptr current)
{
	size_t old_size = current->size;
	smheader_ptr old_next = current->next;

	current->size = s;
	current->used = 1;
	current->next = (void *)base_address + current->size; // **주소 연산**

	(current->next)->size = old_size - (current->size + sizeof(smheader));
	(current->next)->used = 0;
	(current->next)->next = old_next;
}

// 메모리 영역을 새로 할당받을 경우의 메모리 영역 분할 함수
void SplitNewMemory(size_t s, size_t new_size, smheader_ptr new_address, smheader_ptr current)
{
	void* base_address = (void *)new_address + sizeof(smheader);
	smheader_ptr old_next = current->next;

	current->next = new_address;

	new_address->size = s;
	new_address->used = 1;
	new_address->next = (void *)base_address + new_address->size;

	(new_address->next)->size = new_size - (new_address->size + sizeof(smheader));
	(new_address->next)->used = 0;
	(new_address->next)->next = old_next;
}

// unused data region 병합 함수(추후 로직 재구성 예정)
void MergeMemory(size_t s, size_t old_size2, void* base_address, smheader_ptr current, smheader_ptr new_address)
{
	// old_size = old_size2 + num_page * page_size
	size_t old_size = current->size;

	current->size = s;
	current->used = 1;

	base_address = (void *)current + sizeof(smheader); // **주소 연산**

	smheader_ptr old_next = current->next;

	current->next = (void *)new_address + (current->size - old_size2); // **주소 연산**

	(current->next)->size = old_size - current->size - sizeof(smheader);
	(current->next)->used = 0;
	(current->next)->next = old_next;
}
