#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "smalloc.h"
#include <sys/mman.h>

smheader_ptr smlist = 0x0;
smheader_ptr current = 0x0;

void* smalloc (size_t s)
{
	// TODO
	void* base_address = NULL; // data region 시작 주소
	const size_t page_size = (size_t)getpagesize(); // 4096
	
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
	PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	if (new_address == MAP_FAILED) 
	{
		perror("current->next mmap");
		exit(EXIT_FAILURE);
    }

	base_address = (void*)new_address + sizeof(smheader); // **주소 연산**
	SplitNewMemory(s, new_size, new_address, current);

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
					target = current;
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
			default:
			break;
		}

		if(current->next == NULL) // current가 가리키는게 마지막 data header일 경우
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
	else // size가 s 이상인 적절한 빈 공간을 찾지 못했을 경우(mmap을 통해 새 메모리를 할당 받아야 하는 경우)
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

void* srealloc (void* p, size_t s) 
{
	// TODO
	smheader_ptr header = (void *)p - sizeof(smheader); // **주소연산**
	smheader_ptr prev = NULL;
	void *prev_base_address = NULL;
	smheader_ptr next = header->next;

	size_t total_size = 0;

	if(header == NULL)
	{
		abort();
	}

	if(header->size > s) // data region을 축소하는 경우
	{
		SplitExistingMemory(s, p, header);
		return NULL;
	}
	else if(header->size < s) // data region을 확장하는 경우
	{
		if(header == smlist)
		{
			if(next->used == 0)
			{
				total_size = header->size + (sizeof(smheader) + next->size);

				if(total_size >= s)
				{
					next->size = 0;

					header->size = total_size;
					header->next = next->next;
					next->next = NULL;

					if(header->size > s + 24)
					{
						SplitExistingMemory(s, p, header);
					}
				}
				else // mmap으로 새 메모리 영역 할당받고 마지막 메모리 영역 다음으로 추가
				{
					InsertEndMemory(s);
				}
			}
			else
			{
				InsertEndMemory(s);
			}
			return NULL;
		}

		// header의 바로 앞에있는 메모리 영역의 data header 찾아서 prev에 주소 저장
		prev = smlist;

		while(prev->next != header && prev->next != NULL)
		{
			prev = prev->next;
		}

		if(prev->next == NULL)
		{
			perror("prev in srealloc");
			exit(EXIT_FAILURE);
		}

		if(prev->used == 0 && next->used == 0) // prev, next 둘다 메모리 병합 가능한 경우
		{
			total_size = prev->size + (sizeof(smheader) + header->size) + (sizeof(smheader) + next->size);

			if(total_size >= s)
			{
				// prev와 header 병합 후 header의 data region 내용을 prev의 data region으로 복사
				header->used = 0;
				header->next = NULL;
				prev->used = 1;
				prev->size = prev->size + (sizeof(smheader) + header->size);
				prev->next = next;

				prev_base_address = (void *)prev + sizeof(smheader);
				memcpy(prev_base_address, p, header->size);

				// prev와 next 병합
				next->size = 0;

				prev->size = prev->size + (sizeof(smheader) + next->size);
				prev->next = next->next;
				next->next = NULL;

				p = prev_base_address;

				if(total_size > s + 24)
				{
					SplitExistingMemory(s, p, prev);
				}
			}
			else // mmap으로 새 메모리 영역 할당받고 마지막 메모리 영역 다음으로 추가
			{
				InsertEndMemory(s);
			}
			return NULL;
		}
		else if(prev->used == 1 && next->used == 0) // next만 메모리 병합 가능한 경우
		{
			total_size = header->size + (sizeof(smheader) + next->size);

			if(total_size >= s)
			{
				next->size = 0;

				header->size = total_size;
				header->next = next->next;
				next->next = NULL;

				if(header->size > s + 24)
				{
					SplitExistingMemory(s, p, header);
				}
			}
			else // mmap으로 새 메모리 영역 할당받고 마지막 메모리 영역 다음으로 추가
			{
				InsertEndMemory(s);
			}
			return NULL;
		}
		else if(prev->used == 0 && next->used == 1) // prev만 메모리 병합 가능한 경우
		{
			total_size = prev->size + (sizeof(smheader) + header->size);

			if(total_size >= s)
			{
				header->used = 0;
			
				prev->used = 1;
				prev->size = total_size;
				prev->next = header->next;

				header->next = NULL;

				prev_base_address = (void *)prev + sizeof(smheader);
				memcpy(prev_base_address, p, header->size);

				p = prev_base_address;

				if(prev->size > s + 24)
				{
					SplitExistingMemory(s, p, prev);
				}
			}
			else // mmap으로 새 메모리 영역 할당받고 마지막 메모리 영역 다음으로 추가
			{
				InsertEndMemory(s);
			}
			return NULL;
		}
		else // prev, next 둘다 메모리 병합 불가능한 경우
		{
			InsertEndMemory(s);
			return NULL;
		}
	}
	else // header->size == s인 경우(축소도 확장도 아닌 경우)
	{
		printf("Choose other data size except for %ld\n", header->size);
		return NULL;
	}
	return NULL;
}

void smcoalesce ()
{
	//TODO
	smheader_ptr current = smlist;
	smheader_ptr next = current->next;

	size_t total_size = 0;

	while(next)
	{
		if(current->used == 0 && next->used == 0)
		{
			total_size = current->size + (sizeof(smheader) + next->size);
			current->size = total_size;
			
			current->next = next->next;

			next->size = 0;
			next->next = NULL;

			next = current->next;
		}
		else
		{
			current = current->next;
			next = next->next;
		}
	}
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

	(new_address->next)->size = new_size - (sizeof(smheader) + new_address->size + sizeof(smheader));
	(new_address->next)->used = 0;
	(new_address->next)->next = old_next;
}

// 마지막 메모리 영역 다음으로 새로 할당받은 메모리 영역 추가하는 함수
void InsertEndMemory(size_t s)
{
	size_t num_page = 0;
	const size_t page_size = (size_t)getpagesize(); // 4096
	size_t new_size = 0;
	smheader_ptr new_address = NULL;
	smheader_ptr last_header = NULL;

	num_page = (s / page_size) + 1;

	if((num_page * page_size) - sizeof(smheader) < s)
	{
		num_page++;
	}
	new_size = num_page * page_size;

	new_address = mmap((void *)current + sizeof(smheader) + current->size, new_size, 
	PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	// 마지막 메모리 영역 탐색하고 last_header에 해당 data header 주소 저장
	last_header = smlist;
	while(last_header->next != NULL)
	{
		last_header = last_header->next;
	}

	SplitNewMemory(s, new_size, new_address, last_header);
}