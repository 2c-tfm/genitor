#include <genitor.h>
#include <fs.h>
#include <fstab.h>
#include <disk.h>
#include <malloc.h>
#include <file.h>
#include <unistd.h>
#include <string.h>


fstab_entry *fstab = NULL;

// fstab_extract_entry - skips whitespaces and allocate then extract the value
// we want
char *fstab_extract_entry(char **fstab_cpptr) {
	char *fstab_cptr = *fstab_cpptr;
	char *next_ws;
	uint32_t entry_size;

	next_ws = fstab_cptr;
	while (*next_ws != ' ' && *next_ws != '\t' && *next_ws != '\0')	// finding the next white space
		next_ws++;
	if (next_ws == fstab_cptr)
		return (NULL);
	entry_size = next_ws - fstab_cptr;
	char *entry = malloc(entry_size + 1);
	if (entry == NULL)
		return (NULL);
	memcpy(entry, fstab_cptr, entry_size);
	entry[entry_size] = 0;
	*fstab_cpptr += entry_size;
	return entry;
}

// fstab_disk_resolve - extracts the uuid from fstab string then returns 
// the resolved disk path
static char *fstab_disk_resolve(char **fstab_cpptr) {
	char *fstab_cptr = *fstab_cpptr;
	char *resolved_disk;


	if (memcmp("UUID=", fstab_cptr, 5) == 0) {	// starting with uuid or device driver
		char *uuid = fstab_extract_entry(fstab_cpptr);
		resolved_disk = resolve_disk_location(uuid);
		free(uuid);
		return resolved_disk;
	}	
	// for disks (eg. /dev/sda2)
	resolved_disk = fstab_extract_entry(fstab_cpptr);
	return resolved_disk;
}

// read_fstab - opens and reads fstab in an allocated space
static char *read_fstab(void) {
	char *fstab_content = NULL;
	int32_t fstabfd;
	uint32_t fstabsize;

	fstabfd = open("/etc/fstab", O_RDONLY);
	// fstabfd = open("/tmp/fstab", O_RDONLY);
	if (fstabfd < 0)
		return NULL;
	fstabsize = fdsize(fstabfd);
	fstab_content = malloc(fstabsize + 1);
	if (fstab_content == NULL)
		return NULL;

	read(fstabfd, fstab_content, fstabsize);
	fstab_content[fstabsize] = '\0';
	close(fstabfd);


	return (fstab_content);
}


// fstab_populate - reads /etc/fstab and parses it then returns ouput as a 
// 			linked list of fstab_entries
// PS: expects the root directory to be mounted
fstab_entry *fstab_populate(void) {
	fstab_entry *head = NULL;
	fstab_entry *current;
	char *fstab_content = NULL;
	char *fstab_cptr = NULL;

	fstab_content = read_fstab();
	fstab_cptr = fstab_content;

	if (fstab_content == NULL)
		return (NULL);

	// populating the fstab
	while (*fstab_cptr != '\0') {
		SKIP_WHITESPACE(fstab_cptr);

		if (*fstab_cptr == '\n') {		// empty line
			fstab_cptr++;
			continue;
		}

		if (*fstab_cptr == '\0')
			break;

		if (*fstab_cptr == '#') {		// skipping comments
			SKIP_LINE(fstab_cptr);
			continue;
		}

		// allocating fstab entry
		if (head == NULL) {
			head = malloc(sizeof(*head));
			bzero(head, sizeof(*head));
			current = head;
		} else {
			current->next = malloc(sizeof(*current));
			bzero(current->next, sizeof(*head));
			current = current->next;
		}

		// populating the fstab entry
		current->path = fstab_disk_resolve(&fstab_cptr);
		SKIP_WHITESPACE(fstab_cptr);
		current->mount_point = fstab_extract_entry(&fstab_cptr);
		SKIP_WHITESPACE(fstab_cptr);
		current->fstype = fstab_extract_entry(&fstab_cptr);
		SKIP_WHITESPACE(fstab_cptr);
		current->options = fstab_extract_entry(&fstab_cptr);
		current->next = NULL;

		// skipping to the newline 

		SKIP_LINE(fstab_cptr);
		printf("shit %s\n", current->path);
	}

	free(fstab_content);
	return (head);
}


