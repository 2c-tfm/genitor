#include <genitor.h>
#include <file.h>
#include <string.h>
#include <unistd.h>
#include <syscall.h>
#include <malloc.h>

char *extract_cmdline(const char *key) {
	int fd;
	int32_t size;
	char *content;
	char *val = NULL;
	uint32_t i = 0;

	size = fsize("/proc/cmdline");

	if (size < 0) {
		log(ERROR, "cannot find /proc/cmdline");
		return (NULL);
	}

	// reading the file
	fd = open("/proc/cmdline", O_RDONLY);
	content = malloc(size + 1);

	if (content == NULL) {
		return (NULL);
	}

	memset(content, 0, size + 1);

	read(fd, content, size);
	while (content[i] != '\0') {
		while (content[i] == ' ' || content[i] == '\t')
			i++;
		if (content[i] == '\0')
			break;

		if (memcmp(key, &content[i], strlen(key)) == 0) {
			uint32_t save;
			i+= strlen(key);

			// key with no value
			if (content[i] != '=')
				break;

			i++;		// skipping the =
			save = i;
			while (content[save] != ' ' && content[save] != '\t' && 
					content[save] != '\0')
				save++;

			if (save == i)		// no value
				break;
			
			val = malloc(save - i);
			memcpy(val, &content[i], save - i);
			break;

		}
		while (content[i] != ' ' && content[i] != '\t' && 
				content[i] != '\0')
			i++;
		if (content[i] == '\0')
			break;
	}

	return (val);
}
