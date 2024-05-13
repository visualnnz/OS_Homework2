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
	void* base_address = NULL; // data region ���� �ּ�
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
	}

		current = smlist;

	while(current) // data header list�� ������ data header���� �ݺ�
	{
		if(current->used == 0 && current->size >= s) // data region�� unused�̸鼭 size�� s �̻��� ��� ����
		{
			if(current->size > s + 24) // unused data region�� size�� s + 24���� Ŭ ��� �޸� ������ 2���� split�ϰ� size�� s�� update
			{
				base_address = (void*)current + sizeof(smheader); // **�ּ� ����**
				SplitExistingMemory(s, base_address, current);

				return base_address;
			}
			else // size�� s ~ s + 24�� ���
			{
				current->used = 1;
				base_address = (void*)current + sizeof(smheader); // **�ּ� ����**
				return base_address;
			}
		}
		if(current->next == NULL) // current�� ����Ű�°� ������ data header�� ���
		{
			break;
		}
		current = current->next; // current�� ���� data header�� ����Ű������
	}

	// Ž���� ��� unused data region�� �޸� �������� ��û�� �޸� ���� s���� ������ ���
	// ���� �Ҵ�Ǿ��ִ� �޸� ���� ���� 1�� �̻��� ������ ũ�⸸ŭ �޸� ���� �߰�
	size_t num_page = (s / page_size) + 1;

	if((num_page * page_size) - sizeof(smheader) < s)
	{
		num_page++;
	}

	size_t new_size = num_page * page_size;

	// mmap���� ���ο� �޸� ������ �Ҵ� ���� �� �� �޸� ������ ���� �ּҸ� �����ϴ� ��� ������
	smheader_ptr new_address = NULL;

	// ����(������ data region)current�� ����Ű�� �޸� ���� ���� mmap�� ���� �Ҵ���� �޸� ���� �̾� ���̱�
	new_address = mmap((void *)current + sizeof(smheader) + current->size, new_size, 
	PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0); // (!)���� �߻�: �ּ� ����

	if (new_address == MAP_FAILED) 
	{
		perror("current->next mmap");
		exit(EXIT_FAILURE);
    }

	base_address = (void*)new_address + sizeof(smheader); // **�ּ� ����**
	SplitNewMemory(s, new_size, new_address, current);

	/** �޸� ���� �������� �ٽ� ��� ����**/

	// �� �޸� ������ �߰� �Ҵ�ޱ� �� ���� �޸� ������ data region ũ��
	// size_t old_size2 = current->size;

	// current->size = current->size + num_page * page_size; // current->size�� �Ҵ� ���� �޸� ���� ��ŭ ���ؼ� ����
	// base_address = (void*)current + sizeof(smheader); // **�ּ� ����**
	// SplitNewMemory(s, old_size2, base_address, current, new_address);

	return base_address;
}

void* smalloc_mode (size_t s, smmode m)
{
	// TODO
	void* base_address = NULL; // data region ���� �ּ�

	smheader_ptr target = NULL; // Ư�� ����(bestfit, worstfit, firstfit)�� �����ϴ� �� ������ ��� ������

	size_t target_size = 0;

	current = smlist;
	while(current)
	{
		switch(m)
		{
			case bestfit:
				if(current->used == 0 && current->size >= s)
				{
					if(target_size < s) // ���ʷ� Ž���� s �̻��� �޸� ������ ���
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
					if(target_size < s) // ���ʷ� Ž���� s �̻��� �޸� ������ ���
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

					if(target_size > s + 24) // target_size�� s + 24���� Ŭ ��� �޸� ������ 2���� split�ϰ� size�� s�� update
					{
						base_address = (void*)target + sizeof(smheader); // **�ּ� ����**
						SplitExistingMemory(s, base_address, target);

						return base_address;
					}
					else // target_size�� s ~ s + 24�� ���
					{
						target->used = 1;
						base_address = (void*)target + sizeof(smheader); // **�ּ� ����**
						return base_address;
					}
				}
			break;
		}
		if(current->next = NULL) // current�� ����Ű�°� ������ data header�� ���
		{
			break;
		}
		current = current->next;
	}

	if(target_size > s + 24) // target_size�� s + 24���� Ŭ ��� �޸� ������ 2���� split�ϰ� size�� s�� update
	{
		base_address = (void*)target + sizeof(smheader); // **�ּ� ����**
		SplitExistingMemory(s, base_address, target);

		return base_address;
	}
	else if(target_size >= s && target_size <= s + 24)// target_size�� s ~ s + 24�� ���
	{
		target->used = 1;
		base_address = (void*)target + sizeof(smheader); // **�ּ� ����**
		return base_address;
	}
	else // size�� s �̻��� ������ �� ������ ã�� ������ ���
	{
		return smalloc(s);
	}
}

void sfree (void* p) 
{
	// TODO
	void* base_address = NULL; // data region ���� �ּ�
	current = smlist;

	while(current) // p�� ����Ű�� �ּҿ� smlist ������ data region�� ����Ű�� �ּҵ� �߿� ��ġ�ϴ°� �ִ��� Ȯ��
	{
		base_address = (void*)current + sizeof(smheader); // **�ּҿ���**
		if(base_address == p)
		{
			break;
		}
		else
		{
			if(current->next == NULL) // ������ data region���� Ž���ص� p�� ����Ű�� �ּҿ� ��ġ�ϴ� �ּҰ� ������ abort()
			{
				abort();
			}
			current = current->next;
		}
	}

	smheader_ptr header = p - sizeof(smheader); // **�ּҿ���**

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
// ���� �޸� ���� ���� �Լ�
void SplitExistingMemory(size_t s, void* base_address, smheader_ptr current)
{
	size_t old_size = current->size;
	smheader_ptr old_next = current->next;

	current->size = s;
	current->used = 1;
	current->next = (void *)base_address + current->size; // **�ּ� ����**

	(current->next)->size = old_size - (current->size + sizeof(smheader));
	(current->next)->used = 0;
	(current->next)->next = old_next;
}

// �޸� ������ ���� �Ҵ���� ����� �޸� ���� ���� �Լ�
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

// unused data region ���� �Լ�(���� ���� �籸�� ����)
void MergeMemory(size_t s, size_t old_size2, void* base_address, smheader_ptr current, smheader_ptr new_address)
{
	// old_size = old_size2 + num_page * page_size
	size_t old_size = current->size;

	current->size = s;
	current->used = 1;

	base_address = (void *)current + sizeof(smheader); // **�ּ� ����**

	smheader_ptr old_next = current->next;

	current->next = (void *)new_address + (current->size - old_size2); // **�ּ� ����**

	(current->next)->size = old_size - current->size - sizeof(smheader);
	(current->next)->used = 0;
	(current->next)->next = old_next;
}
