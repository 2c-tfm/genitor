#include <genitor.h>
#include <file.h>
#include <malloc.h>
#include <syscall.h>
#include <string.h>
#include <disk.h>

// is_mounted - given a path it opens /proc/self/mounts then checks if that 
// 		path has already something mounted into it
bool is_mounted(const char *path) {
	char *pcontent;
	char *pcontptr;
	bool mounted = false;

	pcontent = read_file("/proc/self/mounts");

	pcontptr = pcontent;
	while (*pcontptr != '\0') {
		SKIP_WHITESPACE(pcontptr);
		if (*pcontptr == '\0')
			break;

		while (*pcontptr != ' ' && *pcontptr != '\t' &&  *pcontptr != '\0')
			pcontptr++;

		if (*pcontptr == '\0')
			break;

		
		SKIP_WHITESPACE(pcontptr);
		if (*pcontptr == '\0')
			break;

		if (memcmp(path, pcontptr, strlen(path)) == 0) {
			mounted = true;
			break;
		}

		while (*pcontptr != '\n' &&  *pcontptr != '\0')
			pcontptr++;

		if (*pcontptr == '\n')
			pcontptr++;
	}
	free(pcontent);
	return mounted;
}

// normalize_path - litterally normalizes ../ and ./
static int normalize_path(const char *input, char *output, size_t output_size) {
	size_t marks[256];
	size_t depth = 0;
	size_t out = 1;

	if (!input || !output || output_size < 2 || input[0] != '/')
		return -1;

	output[0] = '/';
	output[1] = '\0';

	while (*input) {
		while (*input == '/')
			input++;

		if (*input == '\0')
			break;

		const char *start = input;

		while (*input && *input != '/')
			input++;

		size_t len = (size_t)(input - start);

		if (len == 1 && start[0] == '.')
			continue;

		if (len == 2 && start[0] == '.' && start[1] == '.') {
			if (depth > 0)
				out = marks[--depth];

			output[out] = '\0';
			continue;
		}

		if (depth >= sizeof(marks) / sizeof(marks[0]))
		    return -1;

		size_t previous = out;
		size_t separator = (out > 1) ? 1 : 0;

		if (len > output_size - out - 1 ||
			separator > output_size - out - 1 - len)
			return -1;

		if (separator)
			output[out++] = '/';

		marks[depth++] = previous;

		memcpy(output + out, start, len);
		out += len;
		output[out] = '\0';
	}

	return 0;
}

// realpath - takes a symlink or a file, normalizes it and returns it back to 
// 		the calling function.
// PS :
// make sure to set resolved to PATH_MAX or things will get ugly very quickly
char *realpath(const char *path, char *resolved) {
	char ppath[PATH_MAX];		// unsymlinked but not normalized
	char *fullpath;			// symlink and parent
	
	bzero(ppath, PATH_MAX);
	readlinkat(AT_FDCWD, path, ppath, PATH_MAX);
	if (*ppath == '\0')
		return (NULL);
	if (*ppath != '/') {		// path doesn't start from the root [eg. ../sda2]
		char path_clone[PATH_MAX];
		memcpy(path_clone, path, strlen(path));

		char *file = strrchr(path_clone, '/');

		// trimming from the last recorded directory
		if (file != NULL)
			*(file + 1) = 0;
		fullpath = strjoin(path_clone, ppath);
	} else 
		fullpath = strdup(ppath);


	normalize_path(fullpath, resolved, PATH_MAX);


	free(fullpath);
	return (resolved);
	
}

// identify_disk_label - gets the real path of /dev/disk/by_[partuuid/uuid] 
// 			and returns it back to the calling function
// PS:
// For now this method is kinda reliable, i couldn't find a distro which 
// doesn't have the same behaviour, but for support another identification 
// method would be greater
char *resolve_disk_location(char *val) {
	char *fppath;			// full partition path
	char *ident = val;
	char *fullpath;

	while (*ident != '=' && *ident != '\0') 
		ident++;

	// treating either for empty or invalid uuid var 
	if (*ident != '=' || *(ident + 1) == '\0') {
		printf("invalid disk identifier in /proc/cmdline, cannot mount root"
				" for health check");
		return NULL;
	}

	ident++;			// skipping the =

	if (memcmp("UUID", val, 4) == 0)
		fullpath = strjoin(UUID_BASEPATH, ident);
	else if (memcmp("PARTUUID", val, 8) == 0)
		fullpath = strjoin(PARTUUID_BASEPATH, ident);
	else
		return (strdup(val));		// it's a label so it's already resolved

	fppath = malloc(PATH_MAX);
	if (fppath == NULL)
		return NULL;

	if(realpath(fullpath, fppath) == NULL) {
		free(fppath);
		fppath = NULL;
	}

	free(fullpath);
	return (fppath);
}
