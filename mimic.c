#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <time.h>
#include <math.h>

struct chunk * list[0x30] = { 0 };


struct chunk
{
	int size;
	void* ptr;
};

int read_n(char* ptr, int size)
{
	for (int i=0;i<size;i++)
	{
		if (read(0, ptr, 1) == -1)
		{
			puts("read error!");
			exit(0);
		}
		if (*ptr == '\n')
		{
			*ptr = '\0';
			break;
		}
        ptr++;
	}
	return 0;
}
int init()
{
	setbuf(stdout, 0);
	setbuf(stdin, 0);
	setbuf(stderr, 0);
	puts("It is funnier that man exceeds the speed of light than fish start living in the land.");
	puts("It can be said that this is an final ultimatum from the god to the people who can fight.");
	puts("");
	sleep(1);
	puts("Try to deceive the world");
	puts("Lonely... observer");
	return 0;
}

int main()
{
	int cmd;
	init();
	while (1)
	{
		menu();
		cmd = getint();
		switch (cmd)
		{
		case 1:
			add();
			break;
		case 2:
			dele();
			break;
		case 3:
			show();
			break;
        case 4:
            edit();
            break;
		default:
			break;
		}
	}
}

int getint()
{
	int num;
	if(scanf("%d",&num) != 1)
		num = 0;
	while(getchar()!='\n');
	return num;
}


int add()
{
	int i = 0;
	int sz = 0;
	puts("index?");
	puts(">>");
	i = getint();
	if (list[i] != 0 || i >= 0x30 || i < 0)
	{
		puts("error!");
		return 0;
	}

	puts("size?");
	puts(">>"); 
	sz = getint();
	if (sz > 0 && sz <= 0x400)
	{
		struct chunk *note = malloc(sizeof(struct chunk));
		note->size = sz;
		note->ptr = malloc(sz);
		list[i] = note;
		puts("content:");
		read_n(note->ptr,note->size);
		puts("done");
	}
	else
	{
		puts("too big!");
	}
	return 0;
}



int dele()
{
	int idx;
	puts("index?");
	puts(">>");
	idx = getint();
	if (list[idx] == 0)
	{
		puts("cant find");
		return 0;
	}
	free(list[idx]->ptr);
	// free(list[idx]);
	puts("delete successly");
	return 0;
}

int show()
{
	int idx;
	puts("index?");
	puts(">>");
	idx = getint();
	if (idx >= 0x30)
	{
		puts("out of range");
		return 0;
	}
	if (list[idx] == 0)
	{
		puts("cant find");
		return 0;
	}
	printf("content:");
	write(1,list[idx]->ptr,list[idx]->size);
	return 0;
}

int edit()
{
	int idx;
	puts("index?");
	puts(">>");
	idx = getint();
	if (idx >= 0x30)
	{
		puts("out of range");
		return 0;
	}
	if (list[idx] == 0)
	{
		puts("cant find");
		return 0;
	}
	printf("content:");
	read_n(list[idx]->ptr, list[idx]->size);
    puts("done!");
	return 0;
}

int menu()
{
	puts("1.add");
	puts("2.dele");
	puts("3.view");
	puts("4.edit");
	puts(">>");
	return 0;
}