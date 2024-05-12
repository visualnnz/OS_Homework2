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
	void* base_address = NULL;
	size_t page_size = (size_t)getpagesize(); // 4096
	
	if(smlist == 0x0) // data header list�� smlist�� �������� ���� ���
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

		current = smlist;

		/************* for debug *************/
		printf("[for debug] smlist: %p\n", smlist);
		/************* for debug *************/
	}

	// �ּ� 592�� ����(�̻���)
	while(current) // data header list�� ������ data header���� �ݺ�
	{
		if(current->used == 0 && current->size >= s) // data region�� unused�̸鼭 size�� s �̻��� ��� ����
		{
			if(current->size > s + 24) // unused data region�� size�� s + 24���� Ŭ ��� �޸� ������ 2���� split�ϰ� size�� s�� update
			{
				size_t old_size = current->size;

				current->size = s;
				current->used = 1;

				base_address = (void* )current + sizeof(smheader);

				smheader_ptr old_next = current->next;

				current->next = base_address + current->size;

				(current->next)->size = old_size - current->size - sizeof(smheader);
				(current->next)->used = 0;
				(current->next)->next = old_next;

				return base_address;
			}
			else // size�� s ~ s + 24�� ���
			{
				current->used = 1;
				base_address = current + sizeof(smheader);
				return base_address;
			}
		}
		if(current->next == NULL) // current�� ����Ű�°� ������ data header�� ���
		{
			break;
		}
		current = current->next; // smlist�� ���� data header�� ����Ű������
	}

	// Ž���� ��� unused data region�� �޸� �������� ��û�� �޸� ���� s���� ������ ���
	// ���� �Ҵ�Ǿ��ִ� �޸� ���� ���� 1�� �̻��� ������ ũ�⸸ŭ �޸� ���� �߰�
	size_t num_page = (s / page_size) + 1;

	if((num_page * page_size) - sizeof(smheader) < s)
	{
		num_page++;
	}

	current->next = (smheader_ptr)mmap(current + sizeof(smheader) + current->size, num_page * page_size, 
	PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	if (current->next == MAP_FAILED) 
	{
		perror("current->next mmap");
		exit(EXIT_FAILURE);
    }

	current = current->next;

	current->size = (num_page * page_size) - sizeof(smheader);
	current->used = 0;
	current->next = NULL;

	// �� while ���� �ִ� ���� ����
	if(current->size > s + 24) // unused data region�� size�� s + 24���� Ŭ ��� �޸� ������ 2���� split�ϰ� size�� s�� update
	{
		size_t old_size2 = current->size;

		current->size = s;
		current->used = 1;
		base_address = current + sizeof(smheader);

		smheader_ptr old_next2 = current->next;
		current->next = current + sizeof(smheader) + current->size;
		(current->next)->size = old_size2 - current->size - sizeof(smheader);
		(current->next)->used = 0;
		(current->next)->next = old_next2;

		return base_address;
	}
	else // size�� s ~ s + 24�� ���
	{
		current->used = 1;
		base_address = current + sizeof(smheader);

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
