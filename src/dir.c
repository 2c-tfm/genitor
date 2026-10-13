#include <genitor.h>
#include <syscall.h>
#include <malloc.h>
#include <disk.h>
#include <unistd.h>
#include <dir.h>
#include <string.h>

dir_iterator *dir_open(const char *path) {
	dir_iterator *it = NULL;

	it = malloc(sizeof(dir_iterator));
	if (it == NULL)
		return NULL;
	it->fd = openat(AT_FDCWD, path, O_RDONLY | O_DIRECTORY | O_CLOEXEC, 0);
	if (it->fd < 0) {
		free(it);
		return NULL;
	}
	it->nread = 0;
	it->pos = 0;
	it->initialized = 0;
	
	return it;
}

void dir_close(dir_iterator *it) {
	if (it == NULL)
		return;

	close(it->fd);
	free(it);
}

dirent *dir_read (dir_iterator *it) {

	if (it == NULL || it->fd < 0)
		return NULL;

	while (true) {
		if (it->pos >= it->nread) {
			it->nread = getdents64(it->fd, it->buf, sizeof(it->buf));

			it->pos = 0;
			if (it->nread <= 0)
				return NULL;
		}

		if (it->pos < 0 || it->pos > it->nread)
			return NULL;


		struct linux_dirent64 *cd = (struct linux_dirent64 *) (it->buf + it->pos);
		size_t name_offset = offsetof(struct linux_dirent64, d_name);

		if (cd->d_reclen < name_offset + 1 ||
			cd->d_reclen > (size_t)(it->nread - it->pos))
			return NULL;

		size_t name_capacity = cd->d_reclen - name_offset;
		size_t len = strnlen(cd->d_name, name_capacity);

		if (len == name_capacity)
			return NULL;

        	it->pos += cd->d_reclen;
		if ((len == 1 && cd->d_name[0] == '.') ||
			(len == 2 && cd->d_name[0] == '.' && cd->d_name[1] == '.'))
			continue;

		dirent *entry = malloc(sizeof(*entry) + len + 1);
		if (entry == NULL)
			return NULL;

		entry->d_ino = cd->d_ino;
		entry->d_off = cd->d_off;
		entry->d_reclen = sizeof(*entry) + len + 1;
		entry->d_type = cd->d_type;
		memcpy(entry->d_name, cd->d_name, len + 1);

		return entry;

		
	}
}
