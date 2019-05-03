/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "unsquashfs.h"

static int restore_directory(int dirfd, tree_node_t *n, compressor_t *cmp,
			     size_t block_size, frag_reader_t *frag,
			     int sqfsfd)
{
	int fd;

	for (n = n->data.dir->children; n != NULL; n = n->next) {
		switch (n->mode & S_IFMT) {
		case S_IFDIR:
			if (mkdirat(dirfd, n->name, n->mode) &&
			    errno != EEXIST) {
				fprintf(stderr, "mkdir %s: %s\n",
					n->name, strerror(errno));
				return -1;
			}

			fd = openat(dirfd, n->name, O_RDONLY | O_DIRECTORY);
			if (fd < 0) {
				fprintf(stderr, "open dir %s: %s\n",
					n->name, strerror(errno));
				return -1;
			}

			if (restore_directory(fd, n, cmp, block_size,
					      frag, sqfsfd)) {
				close(fd);
				return -1;
			}

			close(fd);
			break;
		case S_IFLNK:
			if (symlinkat(n->data.slink_target, dirfd, n->name)) {
				fprintf(stderr, "ln -s %s %s: %s\n",
					n->data.slink_target, n->name,
					strerror(errno));
				return -1;
			}
			break;
		case S_IFSOCK:
		case S_IFIFO:
			if (mknodat(dirfd, n->name, n->mode, 0)) {
				fprintf(stderr, "creating %s: %s\n",
					n->name, strerror(errno));
				return -1;
			}
			break;
		case S_IFBLK:
		case S_IFCHR:
			if (mknodat(dirfd, n->name, n->mode, n->data.devno)) {
				fprintf(stderr, "creating device %s: %s\n",
					n->name, strerror(errno));
				return -1;
			}
			break;
		case S_IFREG:
			fd = openat(dirfd, n->name,
				    O_WRONLY | O_CREAT | O_EXCL, n->mode);
			if (fd < 0) {
				fprintf(stderr, "creating %s: %s\n",
					n->name, strerror(errno));
				return -1;
			}

			if (extract_file(n->data.file, cmp, block_size,
					 frag, sqfsfd, fd)) {
				close(fd);
				return -1;
			}

			close(fd);
			break;
		default:
			break;
		}
	}

	return 0;
}

int restore_fstree(const char *rootdir, tree_node_t *root, compressor_t *cmp,
		   size_t block_size, frag_reader_t *frag, int sqfsfd)
{
	int dirfd;

	if (mkdir_p(rootdir))
		return -1;

	dirfd = open(rootdir, O_RDONLY | O_DIRECTORY);
	if (dirfd < 0) {
		perror(rootdir);
		return -1;
	}

	if (restore_directory(dirfd, root, cmp, block_size, frag, sqfsfd)) {
		close(dirfd);
		return -1;
	}

	close(dirfd);
	return 0;
}