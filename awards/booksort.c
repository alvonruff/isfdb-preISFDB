#ident "%W%     %G% %Q%"
/*
 *     (C) COPYRIGHT 1994 Al von Ruff
 *         ALL RIGHTS RESERVED
 *
 */

#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

typedef struct entry {
	char entry_buf[1024];
	struct entry *entry_next;
	struct entry *entry_prev;
} entry_t;

static struct entry *head = NULL;
static struct entry *tail = NULL;

static void
insert(current)
	struct entry *current;
{
	struct entry *tmp;
	char *newstring;
	char *oldstring;

	if (head == NULL) {
		head = tail = current;
		current->entry_next = NULL;
		current->entry_prev = NULL;
		return;
	}
	
	newstring = current->entry_buf;
	if ((newstring[0] == 'A') || (newstring[0] == 'T') ||
	    (newstring[0] == 'a') || (newstring[0] == 't')) {
		if (strncmp(newstring,"The ", 4) == 0)
			newstring += 4;
		else if (strncmp(newstring,"A ", 2) == 0)
			newstring += 2;
		else if (strncmp(newstring,"An ", 3) == 0)
			newstring += 3;
		else if (strncmp(newstring,"the ", 4) == 0)
			newstring += 4;
		else if (strncmp(newstring,"a ", 2) == 0)
			newstring += 2;
		else if (strncmp(newstring,"an ", 3) == 0)
			newstring += 3;
	}

	tmp = head;
	while (tmp) {

		oldstring = tmp->entry_buf;
		if ((oldstring[0] == 'A') || (oldstring[0] == 'T') ||
		    (oldstring[0] == 'a') || (oldstring[0] == 't')) {
			if (strncmp(oldstring,"The ", 4) == 0)
				oldstring += 4;
			else if (strncmp(oldstring,"A ", 2) == 0)
				oldstring += 2;
			else if (strncmp(oldstring,"An ", 3) == 0)
				oldstring += 3;
			else if (strncmp(oldstring,"the ", 4) == 0)
				oldstring += 4;
			else if (strncmp(oldstring,"a ", 2) == 0)
				oldstring += 2;
			else if (strncmp(oldstring,"an ", 3) == 0)
				oldstring += 3;
		}

		if (strcmp(newstring, oldstring) < 0) {
			if (tmp->entry_prev == NULL) {
				current->entry_next = tmp;
				current->entry_prev = tmp->entry_prev;
				current->entry_next->entry_prev = current;
				head = current;
			} else {
				current->entry_next = tmp;
				current->entry_prev = tmp->entry_prev;
				current->entry_prev->entry_next = current;
				current->entry_next->entry_prev = current;
			}
			return;
		}
		tmp = tmp->entry_next;
	}
	tail->entry_next = current;
	current->entry_next = NULL;
	current->entry_prev = tail;
	tail = current;

}

static void
dumplist()
{
	struct entry *tmp;

	tmp = head;
	while(tmp) {
		(void)printf("%s\n", tmp->entry_buf);
		tmp = tmp->entry_next;
	}
}

int
buffered_read(fd, buf, size)
	int fd;
	char *buf;
	int size;
{
	static char private_buf[4096];
	static int  bytes_left = 0;
	static char *current_ptr;

	if (bytes_left == 0) {
		bytes_left = read(fd, private_buf, 4096);
		if (bytes_left == 0)
			return(0);
		current_ptr = private_buf;
	} 

	*buf = *current_ptr;
	current_ptr++;
	bytes_left--;
	return(1);
}

int
main(argc, argv)
	int argc;
	char *argv[];
{
	struct entry *current;
	char *tmp;
	int fd;

	if (argc != 2) {
		printf("Usage: booksort filename\n");
		exit(1);
	}

	fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror("Couldn't open database");
		exit(1);
	}

	/*CONSTCOND*/
	while(1) {
		current = (struct entry *)malloc(sizeof(struct entry));
		tmp = current->entry_buf;
		if (buffered_read(fd, tmp, 1) == 0)
			break;
		while(*tmp != '\n') {
			(void)buffered_read(fd, ++tmp, 1);
		}
		*tmp = 0;
		insert(current);
	}
	dumplist();
	exit(0);
	return(0);
}
