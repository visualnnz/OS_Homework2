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
	void* base_address = NULL; // data region ���� �ּ�
	const size_t page_size = (size_t)getpagesize(); // 4096
	
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
	PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	if (new_address == MAP_FAILED) 
	{
		perror("current->next mmap");
		exit(EXIT_FAILURE);
    }

	base_address = (void*)new_address + sizeof(smheader); // **�ּ� ����**
	SplitNewMemory(s, new_size, new_address, current);

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
					target = current;
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
			default:
			break;
		}

		if(current->next == NULL) // current�� ����Ű�°� ������ data header�� ���
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
	else // size�� s �̻��� ������ �� ������ ã�� ������ ���(mmap�� ���� �� �޸𸮸� �Ҵ� �޾ƾ� �ϴ� ���)
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

void* srealloc (void* p, size_t s) 
{
	// TODO
	smheader_ptr header = (void *)p - sizeof(smheader); // **�ּҿ���**
	smheader_ptr prev = NULL;
	void *prev_base_address = NULL;
	smheader_ptr next = header->next;

	size_t total_size = 0;

	if(header == NULL)
	{
		abort();
	}

	if(header->size > s) // data region�� ����ϴ� ���
	{
		SplitExistingMemory(s, p, header);
		return NULL;
	}
	else if(header->size < s) // data region�� Ȯ���ϴ� ���
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
				else // mmap���� �� �޸� ���� �Ҵ�ް� ������ �޸� ���� �������� �߰�
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

		// header�� �ٷ� �տ��ִ� �޸� ������ data header ã�Ƽ� prev�� �ּ� ����
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

		if(prev->used == 0 && next->used == 0) // prev, next �Ѵ� �޸� ���� ������ ���
		{
			total_size = prev->size + (sizeof(smheader) + header->size) + (sizeof(smheader) + next->size);

			if(total_size >= s)
			{
				// prev�� header ���� �� header�� data region ������ prev�� data region���� ����
				header->used = 0;
				header->next = NULL;
				prev->used = 1;
				prev->size = prev->size + (sizeof(smheader) + header->size);
				prev->next = next;

				prev_base_address = (void *)prev + sizeof(smheader);
				memcpy(prev_base_address, p, header->size);

				// prev�� next ����
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
			else // mmap���� �� �޸� ���� �Ҵ�ް� ������ �޸� ���� �������� �߰�
			{
				InsertEndMemory(s);
			}
			return NULL;
		}
		else if(prev->used == 1 && next->used == 0) // next�� �޸� ���� ������ ���
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
			else // mmap���� �� �޸� ���� �Ҵ�ް� ������ �޸� ���� �������� �߰�
			{
				InsertEndMemory(s);
			}
			return NULL;
		}
		else if(prev->used == 0 && next->used == 1) // prev�� �޸� ���� ������ ���
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
			else // mmap���� �� �޸� ���� �Ҵ�ް� ������ �޸� ���� �������� �߰�
			{
				InsertEndMemory(s);
			}
			return NULL;
		}
		else // prev, next �Ѵ� �޸� ���� �Ұ����� ���
		{
			InsertEndMemory(s);
			return NULL;
		}
	}
	else // header->size == s�� ���(��ҵ� Ȯ�嵵 �ƴ� ���)
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

	(new_address->next)->size = new_size - (sizeof(smheader) + new_address->size + sizeof(smheader));
	(new_address->next)->used = 0;
	(new_address->next)->next = old_next;
}

// ������ �޸� ���� �������� ���� �Ҵ���� �޸� ���� �߰��ϴ� �Լ�
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

	// ������ �޸� ���� Ž���ϰ� last_header�� �ش� data header �ּ� ����
	last_header = smlist;
	while(last_header->next != NULL)
	{
		last_header = last_header->next;
	}

	SplitNewMemory(s, new_size, new_address, last_header);
}