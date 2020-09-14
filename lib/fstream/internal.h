/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * internal.h
 *
 * Copyright (C) 2019 David Oberhollenzer <goliath@infraroot.at>
 */
#ifndef INTERNAL_H
#define INTERNAL_H

#include "config.h"
#include "compat.h"
#include "fstream.h"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#define BUFSZ (262144)

typedef struct ostream_comp_t {
	ostream_t base;

	ostream_t *wrapped;

	size_t inbuf_used;

	sqfs_u8 inbuf[BUFSZ];
	sqfs_u8 outbuf[BUFSZ];

	int (*flush_inbuf)(struct ostream_comp_t *ostrm, bool finish);

	void (*cleanup)(struct ostream_comp_t *ostrm);
} ostream_comp_t;

#ifdef __cplusplus
extern "C" {
#endif

SQFS_INTERNAL ostream_comp_t *ostream_gzip_create(const char *filename);

SQFS_INTERNAL ostream_comp_t *ostream_xz_create(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* INTERNAL_H */
