#include <genitor.h>
#include <malloc.h>
#include <disk.h>
#include <dir.h>
#include <file.h>
#include <algo.h>
#include <unistd.h>
#include <printf.h>
#include <string.h>

#define DIR_BUF_SIZE	4096

static char *modules_dep = NULL;
static uint64_t modules_dep_size = 0;


// kmod_prepare_modules_dep - opens modules.dep once and for all
static int kmod_prepare_modules_dep(const char *modroot) {
	char *modprobe_builtins;
	int fd;

	modprobe_builtins = strjoin(modroot, "modules.dep");
	log(DEBUG, "opening %s", modprobe_builtins);
	fd = open(modprobe_builtins, O_RDONLY);
	if (fd < 0) {
		close(fd);
		return -1;
	}
	modules_dep_size = fdsize(fd);
	if (modules_dep_size <= 0) {
		close(fd);
		return -1;
	}
	modules_dep = mmap(0, modules_dep_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (modules_dep == NULL)
		return -1;

	return 0;
}

static int kmod_load_module(const char *path) {
	int kfd;

	kfd = open(path, O_RDONLY | O_CLOEXEC);
	if (kfd < 0) {
		log(ERROR, "Cannot open module [%s]", path);
		return (-1);
	}

	int ret = finit_module(kfd, "", 0);
	if (ret < 0) {
		log(ERROR, "Cannot load module [%s]", path);
		return (-1);
	}
	log(SUCCESS, "Module [%s] is loaded", path);
	return (0);
}

static const char *module_load_paths[] = {"/etc/modules-load.d/", 
	"/run/modules-load.d/", "/usr/local/lib/modules-load.d/",
	"/lib/modules-load.d", "/usr/lib/modules-load.d/"};

static uint64_t modload_paths_size = sizeof(module_load_paths) / 
			sizeof(module_load_paths[0]);

static const char *find_modload_conf_path() {
	uint32_t i = 0;
	dir_iterator *dfd ;

	while (i < modload_paths_size) {
		dfd = dir_open(module_load_paths[i]);

		if (dfd != NULL) {
			dir_close(dfd);
			return (module_load_paths[i]);
		}

		dir_close(dfd);
		i++;
	}
	return NULL;
}

static void kmod_load_deps(const char *modroot, char *deps) {
	char *deps_ptr;
	char module_path[PATH_MAX + 1];
	char *module;
	uint32_t modroot_len;
	

	modroot_len = strlen(modroot);
	memcpy(module_path, modroot, modroot_len);
	module = module_path + modroot_len;
	while (*deps != '\n' && *deps != '\0') {
		SKIP_WHITESPACE(deps);

		if (*deps == '\n')
			break;

		deps_ptr = deps;
		while(*deps_ptr != ' ' && *deps_ptr != '\n' && *deps_ptr != '\0')
			deps_ptr++;
		uint32_t module_size =  deps_ptr - deps;
		memcpy(module, deps, module_size);
		module[module_size] = '\0';
		kmod_load_module(module_path);
		memset(module, 0, module_size);
		deps += module_size;
	}
}

// kmod_search_load - searches in
static void kmod_search_load(const char *modroot, char *kmod_name) {
	char *path_match;

	if (kmod_name == NULL)
		return ;
	path_match = (char *) find_bytes((const unsigned char *)modules_dep, 
			modules_dep_size, (const unsigned char *)kmod_name, strlen(kmod_name));
	if (path_match == NULL) {
		log(INFO, "Module %s cannot be found", kmod_name);
		return ;
	}

	// loading dependencies
	kmod_load_deps(modroot, strchr(path_match, ':') + 1);

	// now loading the module
	while (*path_match != '\n' && path_match > modules_dep)
		path_match--;

	if (*path_match == '\n')
		path_match++;

	char *eomod = strchr(path_match, ':');		// endofmodule
	uint32_t modsize = eomod - path_match;
	uint32_t fmodsize = modsize + strlen(modroot) + 1;
	char *mod_path = malloc(fmodsize);
	memcpy(mod_path, modroot, strlen(modroot));
	memcpy(mod_path + strlen(modroot), path_match, modsize);
	mod_path[fmodsize] = '\0';
	kmod_load_module(mod_path);
	free(mod_path);
}


static void kmod_parse_conf_load(const char *modroot, char *path) {
	char *cfd_content;
	char *kmod_name;
	char *cfd_cptr;

	cfd_content = read_file(path);
	cfd_cptr = cfd_content;
	
	while (*cfd_cptr != '\0') {
		SKIP_WHITESPACE(cfd_cptr);
		if (*cfd_cptr == '#' || *cfd_cptr == '\n'){
			SKIP_LINE(cfd_cptr);
			continue;
		}

		// we hit a kernel mod name, extract it
		uint32_t kmod_size = 0;
		char *kmod_ptr = cfd_cptr;
		while (*kmod_ptr != ' ' && *kmod_ptr != '\n' && *kmod_ptr != '\0')
			kmod_ptr++;
		kmod_size = kmod_ptr - cfd_cptr;
		kmod_name = malloc(kmod_size + 1);
		memcpy(kmod_name, cfd_cptr, kmod_size);
		kmod_search_load(modroot, kmod_name);
		free(kmod_name);
		SKIP_LINE(cfd_cptr);
	}
	free(cfd_content);
}

// load_udef_mods - load user defined modules
static void load_udef_mods(const char *modroot, const char *modload_path) {
	dir_iterator *dfd ;
	dirent *centry;

	dfd = dir_open(modload_path);
	if (dfd == NULL) {
		log(DEBUG, "Cannot open %s", modroot);
		return;
	}

	while ((centry = dir_read(dfd)) != NULL) {
		log(DEBUG, "entry %s %s", modload_path, centry->d_name);
		char *kmod_fpath = malloc(strlen(modload_path) + strlen(centry->d_name) + 2);
		snprintf(kmod_fpath, strlen(modload_path) + strlen(centry->d_name) + 2, 
				"%s/%s", modload_path, centry->d_name);

		kmod_parse_conf_load(modroot, kmod_fpath);

		free(kmod_fpath);
		free(centry);
	}
	dir_close(dfd);
}

void kmod_load(void) {
	const char *modload_path ;
	char *modspath;
	char *krelease;
	uint32_t modspath_len;

	krelease = read_file("/proc/sys/kernel/osrelease");
	if (krelease == NULL) {
		log(ERROR, "Cannot figure out kernel version");
		return;
	}
	if (strchr(krelease, '\n') != NULL)
		*strchr(krelease, '\n') = '\0';
	log(DEBUG, "krelease %s", krelease);
	modspath_len = strlen("/lib/modules/") + strlen(krelease) + 2;
	modspath = malloc(modspath_len + 1);
	snprintf(modspath, modspath_len, "/lib/modules/%s/", krelease);
	free(krelease);
	log(INFO, "loading kernel modules from %s", modspath);

	// searching for user defined module paths
	modload_path = find_modload_conf_path();
	if (modload_path == NULL) {
		log(ERROR, "No kernel module load paths has been found");
		return ;
	}
	log(DEBUG, "taking %s as a module load path", modload_path);
	if (kmod_prepare_modules_dep(modspath) < 0) {
		log(ERROR, "Failed locating where kernel modules are");
		return;
	}
	load_udef_mods(modspath, modload_path);
	free(modspath);
	munmap(modules_dep, modules_dep_size);
}
