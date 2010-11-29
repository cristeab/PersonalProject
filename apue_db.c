/*
 * apue_db.c
 *
 *  Created on: Nov 29, 2010
 *      Author: bogdan
 */

#include "apue_db.h"
#include <sys/stat.h>
#include <string.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/syslog.h>
#include <fcntl.h>
#include <stdarg.h>
#include <malloc.h>
#include <errno.h>

#define IDXLEN_SZ 4
#define SEP ':'
#define SPACE ' '
#define NEWLINE '\n'

#define PTR_SZ 6
#define PTR_MAX 999999
#define NHASH_DEF 137 /* default hash table size */
#define FREE_OFF 0
#define HASH_OFF PTR_SZ

typedef unsigned long DBHASH;
typedef unsigned long COUNT;

/* private representation of the database */
typedef struct {
	int idxfd;
	int datfd;
	char* idxbuf;
	char* datbuf;
	char* name;
	off_t idxoff;
	size_t idxlen;
	off_t datoff;
	size_t datlen;
	off_t ptrval;
	off_t ptroff;
	off_t chainoff;
	off_t hashoff;
	DBHASH nhash;
	COUNT cnt_delok;
	COUNT cnt_delerr;
	COUNT cnt_fetchlok;
	COUNT cnt_fetcherr;
	COUNT cnt_nextrec;
	COUNT cnt_stor1;
	COUNT cnt_stor2;
	COUNT cnt_stor3;
	COUNT cnt_stor4;
	COUNT cnt_storerr;
} DB;

int lock_reg(int, int, int, off_t, int, off_t);
#define readw_lock(fd, offset, whence, len) \
	lock_reg((fd), F_SETLKW, F_RDLCK, (offset), (whence), (len))
#define writew_lock(fd, offset, whence, len) \
	lock_reg((fd), F_SETLKW, F_WRLCK, (offset), (whence), (len))
#define un_lock(fd, offset, whence, len) \
	lock_reg((fd), F_SETLK, F_UNLCK, (offset), (whence), (len))

/* Internal functions */
static DB *_db_alloc(int);
static void _db_dodelete(DB*);
static int _db_find_and_lock(DB*, const char*, int);
static int _db_findfree(DB*, int, int);
static void _db_free(DB*);
static DBHASH _db_hash(DB*, const char*);
static char* _db_readdat(DB*);
static off_t _db_readidx(DB*, off_t);
static off_t _db_readptr(DB*, off_t);
static void _db_writedat(DB*, const char*, off_t, int);
static void _db_writeidx(DB*, const char*, off_t, int, off_t);
static void _db_writeptr(DB*, off_t, off_t);

DBHANDLE db_open(const char *pathname, int oflag, ...)
{
	DB *db;
	int len, mode;
	size_t i;
	char asciiptr[PTR_SZ+1], hash[(NHASH_DEF+1)*PTR_SZ+2];
	struct stat statbuf;

	len = strlen(pathname);
	if (NULL == (db = _db_alloc(len)))
	{
		fprintf(stderr, "%s : _db_alloc error\n", __func__);
		return NULL;
	}
	db->nhash = NHASH_DEF;
	db->hashoff = HASH_OFF;
	strcpy(db->name, pathname);
	strcat(db->name, ".idx");

	if (oflag & O_CREAT)
	{
		va_list ap;

		va_start(ap, oflag);
		mode = va_arg(ap, int);
		va_end(ap);

		db->idxfd = open(db->name, oflag, mode);
		strcpy(db->name+len, ".dat");
		db->datfd = open(db->name, oflag, mode);
	} else
	{
		db->idxfd = open(db->name, oflag);
		strcpy(db->name+len, ".dat");
		db->datfd = open(db->name, oflag);
	}
	if (db->idxfd < 0 || db->datfd < 0)
	{
		_db_free(db);
		return NULL;
	}

	if ((O_CREAT | O_TRUNC) == (oflag & (O_CREAT | O_TRUNC)))
	{
		if (0 > writew_lock(db->idxfd, 0, SEEK_SET, 0))
		{
			fprintf(stderr, "%s : writew_lock error\n", __func__);
			return NULL;
		}
		if (0 > fstat(db->idxfd, &statbuf))
		{
			fprintf(stderr, "%s : fstat error\n", __func__);
			return NULL;
		}
		if (0 == statbuf.st_size)
		{
			sprintf(asciiptr, "%*d", PTR_SZ, 0);
			hash[0] = 0;
			for (i = 0; i < NHASH_DEF+1; ++i)
			{
				strcat(hash, asciiptr);
			}
			strcat(hash, "\n");
			i = strlen(hash);
			if (i != write(db->idxfd, hash, i))
			{
				fprintf(stderr, "%s : write error\n", __func__);
				return NULL;
			}
		}
		if (0 > un_lock(db->idxfd, 0, SEEK_SET, 0))
		{
			fprintf(stderr, "%s : un_lock error\n", __func__);
			return NULL;
		}
	}
	db_rewind(db);
	return db;
}

static DB* _db_alloc(int namelen)
{
	DB *db;

	if (NULL == calloc(1, sizeof(DB)))
	{
		fprintf(stderr, "%s : calloc error\n", __func__);
		return NULL;
	}
	db->idxfd = db->datfd = -1;
	if (NULL == (db->name = malloc(namelen+5)))
	{
		fprintf(stderr, "%s : malloc error\n", __func__);
		return NULL;
	}
	if (NULL == (db->idxbuf = malloc(IDXLEN_MAX+2)))
	{
		fprintf(stderr, "%s : malloc error\n", __func__);
		return NULL;
	}
	if (NULL == (db->datbuf = malloc(DATLEN_MAX+2)))
	{
		fprintf(stderr, "%s : malloc error\n", __func__);
		return NULL;
	}
	return db;
}

void db_close(DBHANDLE h)
{
	_db_free((DB*)h);
}

static void _db_free(DB *db)
{
	if (0 <= db->idxfd)
	{
		close(db->idxfd);
	}
	if (0 <= db->datfd)
	{
		close(db->datfd);
	}
	if (NULL != db->idxbuf)
	{
		free(db->idxbuf);
	}
	if (NULL != db->datbuf)
	{
		free(db->datbuf);
	}
	if (NULL != db->name)
	{
		free(db->name);
	}
	free(db);
}

char* db_fetch(DBHANDLE h, const char *key)
{
	DB *db = h;
	char *ptr;

	if (0 > _db_find_and_lock(db, key, 0))
	{
		ptr = NULL;
		++db->cnt_fetcherr;
	} else
	{
		ptr = _db_readdat(db);
		++db->cnt_fetchlok;
	}
	if (0 > un_lock(db->idxfd, db->chainoff, SEEK_SET, 1))
	{
		fprintf(stderr, "%s : un_lock error\n", __func__);
		return NULL;
	}
	return ptr;
}

static int _db_find_and_lock(DB *db, const char *key, int writelock)
{
	off_t offset, nextoffset;

	db->chainoff = (_db_hash(db, key)*PTR_SZ)+db->hashoff;
	db->ptroff = db->chainoff;

	if (writelock)
	{
		if (0 > writew_lock(db->idxfd, db->chainoff, SEEK_SET, 1))
		{
			fprintf(stderr, "%s : writew_lock error\n", __func__);
			return -1;
		}
	} else
	{
		if (0 > readw_lock(db->idxfd, db->chainoff, SEEK_SET, 1))
		{
			fprintf(stderr, "%s : readw_lock error\n", __func__);
			return -2;
		}
	}
	offset = _db_readptr(db, db->ptroff);

	while (0 != offset)
	{
		nextoffset = _db_readidx(db, offset);
		if (0 == strcmp(db->idxbuf, key))
		{
			break;
		}
		db->ptroff = offset;
		offset = nextoffset;
	}
	return (0 == offset)?-3:0;
}

static DBHASH _db_hash(DB *db, const char *key)
{
	DBHASH hval = 0;
	char c;
	int i;

	for (i = 1; (c = *key++) != 0; ++i)
	{
		hval += c*i;
	}
	return hval%db->nhash;
}

static off_t _db_readptr(DB *db, off_t offset)
{
	char asciiptr[PTR_SZ+1];

	if (-1 == lseek(db->idxfd, offset, SEEK_SET))
	{
		fprintf(stderr, "%s : lseek error\n", __func__);
		return -1;
	}
	if (PTR_SZ != read(db->idxfd, asciiptr, PTR_SZ))
	{
		fprintf(stderr, "%s : read error\n", __func__);
		return -1;
	}
	asciiptr[PTR_SZ] = 0;
	return atol(asciiptr);
}

static off_t _db_readidx(DB *db, off_t offset)
{
	ssize_t i;
	char *ptr1, *ptr2;
	char asciiptr[PTR_SZ+1], asciilen[IDXLEN_SZ+1];
	struct iovec iov[2];

	if (-1 == (db->idxoff = lseek(db->idxfd, offset, 0 == offset?SEEK_CUR:SEEK_SET)))
	{
		fprintf(stderr, "%s : lseek error\n", __func__);
		return -1;
	}

	iov[0].iov_base = asciiptr;
	iov[0].iov_len = PTR_SZ;
	iov[1].iov_base = asciilen;
	iov[1].iov_len = IDXLEN_SZ;

	if ((PTR_SZ+IDXLEN_SZ) != (i = readv(db->idxfd, iov, 2)))
	{
		if (0 == i && 0 == offset)
		{
			return -1;
		}
		fprintf(stderr, "%s : readv error\n", __func__);
		return -1;
	}

	asciiptr[PTR_SZ] = 0;
	if (IDXLEN_MIN > (db->idxlen = atoi(asciilen)) || IDXLEN_MAX < db->idxlen)
	{
		fprintf(stderr, "%s : invalid length\n", __func__);
		return -1;
	}

	if (db->idxlen != (i = read(db->idxfd, db->idxbuf, db->idxlen)))
	{
		fprintf(stderr, "%s : read error\n", __func__);
		return -1;
	}
	if (NEWLINE != db->idxbuf[db->idxlen-1])
	{
		fprintf(stderr, "%s : missing newline\n", __func__);
		return -1;
	}
	db->idxbuf[db->idxlen-1] = 0;

	if (NULL == (ptr1 = strchr(db->idxbuf, SEP)))
	{
		fprintf(stderr, "%s : missing first separator\n", __func__);
		return -1;
	}
	*ptr1++ = 0;

	if (NULL == (ptr2 = strchr(ptr1, SEP)))
	{
		fprintf(stderr, "%s : missing second separator\n", __func__);
		return -1;
	}
	*ptr2++ = 0;

	if (NULL != strchr(ptr2, SEP))
	{
		fprintf(stderr, "%s : too many separators\n");
		return -1;
	}

	if (0 > (db->datoff = atol(ptr1)))
	{
		fprintf(stderr, "%s : negative starting offset\n", __func__);
		return -1;
	}
	if (0 >= (db->datlen = atol(ptr2)) ||DATLEN_MAX < db->datlen)
	{
		fprintf(stderr, "%s : invalid length\n", __func__);
		return -1;
	}
	return db->ptrval;
}

static char* _db_readdat(DB *db)
{
	if (-1 == lseek(db->datfd, db->datoff, SEEK_SET))
	{
		fprintf(stderr, "%s : lseek error\n", __func__);
		return NULL;
	}
	if (db->datlen != read(db->datfd, db->datbuf, db->datlen))
	{
		fprintf(stderr, "%s : read error\n", __func__);
		return NULL;
	}
	if (NEWLINE != db->datbuf[db->datlen-1])
	{
		fprintf(stderr, "%s : missing newline\n", __func__);
		return NULL;
	}
	db->datbuf[db->datlen-1] = 0;
	return db->datbuf;
}

int db_delete(DBHANDLE h, const char *key)
{
	DB *db = h;
	int rc = 0;

	if (0 == _db_find_and_lock(db, key, 1))
	{
		_db_dodelete(db);
		++db->cnt_delok;
	} else
	{
		rc = -1;
		++db->cnt_delerr;
	}
	if (0 > un_lock(db->idxfd, db->chainoff, SEEK_SET, 1))
	{
		fprintf(stderr, "%s : un_lock error\n", __func__);
		return -1;
	}
	return rc;
}

static void _db_dodelete(DB *db)
{
	int i;
	char *ptr;
	off_t freeptr, saveptr;

	for (ptr = db->datbuf, i = 0; i < db->datlen-1; ++i)
	{
		*ptr++ = SPACE;
	}
	*ptr = 0;
	ptr = db->idxbuf;
	while (*ptr)
	{
		*ptr++ = SPACE;
	}
	if (0 > writew_lock(db->idxfd, FREE_OFF, SEEK_SET, 1))
	{
		fprintf(stderr, "%s : writew_lock error\n", __func__);
		return;
	}
	_db_writedat(db, db->datbuf, db->datoff, SEEK_SET);

	freeptr = _db_readptr(db, FREE_OFF);
	saveptr = db->ptrval;

	_db_writeidx(db, db->idxbuf, db->idxoff, SEEK_SET, freeptr);
	_db_writeptr(db, FREE_OFF, db->idxoff);
	_db_writeptr(db, db->ptroff, saveptr);

	if (0 > un_lock(db->idxfd, FREE_OFF, SEEK_SET, 1))
	{
		fprintf(stderr, "%s : un_lock error\n", __func__);
	}
}

static void _db_writedat(DB *db, const char *data, off_t offset, int whence)
{
	struct iovec iov[2];
	static char newline = NEWLINE;

	if (SEEK_SET == whence)
	{
		if (0 > writew_lock(db->datfd, 0, SEEK_SET, 0))
		{
			fprintf(stderr, "%s : writew_lock error\n", __func__);
			return;
		}
	}

	if (-1 == (db->datoff = lseek(db->datfd, offset, whence)))
	{
		fprintf(stderr, "%s : lseek error\n", __func__);
		return;
	}
	db->datlen = strlen(data)+1;

	iov[0].iov_base = (char*)data;
	iov[0].iov_len = db->datlen-1;
	iov[1].iov_base = &newline;
	iov[1].iov_len = 1;
	if (db->datlen != writev(db->datfd, iov, 2))
	{
		fprintf(stderr, "%s : writev error\n");
		return;
	}
	if (SEEK_END == whence)
	{
		if (0 > un_lock(db->datfd, 0, SEEK_SET, 0))
		{
			fprintf(stderr, "%s : un_lock error\n", __func__);
			return;
		}
	}
}

static void _db_writeidx(DB *db, const char *key, off_t offset, int whence, off_t ptrval)
{
	struct iovec iov[2];
	char asciiptrmem[PTR_SZ+IDXLEN_SZ+1];
	int len;
	char *fmt;

	if (0 > (db->ptrval = ptrval) || ptrval > PTR_MAX)
	{
		fprintf(stderr, "%s : invalid ptr\n", __func__);
		return;
	}
	if (sizeof(off_t) == sizeof(long long))
	{
		fmt = "%s%c%lld%c%d\n";
	} else
	{
		fmt = "%s%c%ld%c%d\n";
	}
	sprintf(db->idxbuf, fmt, key, SEP, db->datoff, SEP, db->datlen);
	if (IDXLEN_MIN > (len = strlen(db->idxbuf)) || IDXLEN_MAX < len)
	{
		fprintf(stderr, "%s : invalid length\n", __func__);
		return;
	}
	sprintf(asciiptrmem, "%*ld%*d", PTR_SZ, ptrval, IDXLEN_SZ, len);

	if (SEEK_END == whence)
	{
		if (0 > writew_lock(db->idxfd, ((db->nhash+1)*PTR_SZ)+1, SEEK_SET, 0))
		{
			fprintf(stderr, "%s : writew_lock error\n", __func__);
			return;
		}
	}

	if (-1 == (db->idxoff = lseek(db->idxfd, offset, whence)))
	{
		fprintf(stderr, "%s : lseek error\n", __func__);
		return;
	}

	iov[0].iov_base = asciiptrmem;
	iov[0].iov_len = PTR_SZ+IDXLEN_SZ;
	iov[1].iov_base = db->idxbuf;
	iov[1].iov_len = len;
	if (PTR_SZ+IDXLEN_SZ+len != writev(db->idxfd, iov, 2))
	{
		fprintf(stderr, "%s : writev error\n", __func__);
		return;
	}
	if (SEEK_END == whence)
	{
		if (0 > un_lock(db->idxfd, ((db->nhash+1)*PTR_SZ)+1, SEEK_SET, 0))
		{
			fprintf(stderr, "%s : un_lock error\n", __func__);
			return;
		}
	}
}

static void _db_writeptr(DB *db, off_t offset, off_t ptrval)
{
	char asciiptr[PTR_SZ+1];

	if (0 > ptrval || PTR_MAX < ptrval)
	{
		fprintf(stderr, "%s : invalid ptr\n", __func__);
		return;
	}
	sprintf(asciiptr, "%*ld", PTR_SZ, ptrval);

	if (-1 == lseek(db->idxfd, offset, SEEK_SET))
	{
		fprintf(stderr, "%s : lseek error\n", __func__);
		return;
	}
	if (PTR_SZ != write(db->idxfd, asciiptr, PTR_SZ))
	{
		fprintf(stderr, "%s : write error\n");
	}
}

int db_store(DBHANDLE h, const char *key, const char *data, int flag)
{
	DB *db = h;
	int rc, keylen, datlen;
	off_t ptrval;

	if (DB_INSERT != flag && DB_REPLACE != flag && DB_STORE != flag)
	{
		errno = EINVAL;
		return -1;
	}
	keylen = strlen(key);
	datlen = strlen(data)+1;
	if (DATLEN_MIN > datlen || DATLEN_MAX < datlen)
	{
		fprintf(stderr, "%s : invalid data length\n", __func__);
		return -1;
	}

	if (0 > _db_find_and_lock(db, key, 1))
	{
		if (DB_REPLACE == flag)
		{
			rc = -1;
			++db->cnt_storerr;
			errno = ENOENT;
			goto doreturn;
		}
		ptrval = _db_readptr(db, db->chainoff);

		if (0 > _db_findfree(db, keylen, datlen))
		{
			_db_writedat(db, data, 0, SEEK_END);
			_db_writeidx(db, key, 0, SEEK_END, ptrval);
			_db_writeptr(db, db->chainoff, db->idxoff);
			++db->cnt_stor1;
		} else
		{
			_db_writedat(db, data, db->datoff, SEEK_SET);
			_db_writeidx(db, key, db->idxoff, SEEK_SET, ptrval);
			_db_writeptr(db, db->chainoff, db->idxoff);
			++db->cnt_stor2;
		}
	} else
	{
		if (DB_INSERT == flag)
		{
			rc = 1;
			++db->cnt_storerr;
			goto doreturn;
		}
		if (datlen != db->datlen)
		{
			_db_dodelete(db);
			ptrval = _db_readptr(db, db->chainoff);
			_db_writedat(db, data, 0, SEEK_END);
			_db_writeidx(db, key, 0, SEEK_END, ptrval);
			_db_writeptr(db, db->chainoff, db->idxoff);
			++db->cnt_stor3;
		} else
		{
			_db_writedat(db, data, db->datoff, SEEK_SET);
			++db->cnt_stor4;
		}
	}
	rc = 0;

	doreturn:
	if (0 > un_lock(db->idxfd, db->chainoff, SEEK_SET, 1))
	{
		fprintf(stderr, "un_lock error\n");
	}
	return rc;
}

static int _db_findfree(DB *db, int keylen, int datlen)
{
	int rc;
	off_t offset, nextoffset, saveoffset;

	if (0 > writew_lock(db->idxfd, FREE_OFF, SEEK_SET, 1))
	{
		fprintf(stderr, "%s : writew_lock error\n", __func__);
		return -1;
	}
	saveoffset = FREE_OFF;
	offset = _db_readptr(db, saveoffset);

	while (0 != offset)
	{
		nextoffset = _db_readidx(db, offset);
		if (keylen == strlen(db->idxbuf) && datlen == db->datlen)
		{
			break;
		}
		saveoffset = offset;
		offset = nextoffset;
	}

	if (0 == offset)
	{
		rc = -1;
	} else
	{
		_db_writeptr(db, saveoffset, db->ptrval);
		rc = 0;
	}
	if (0 > un_lock(db->idxfd, FREE_OFF, SEEK_SET, 1))
	{
		fprintf(stderr, "%s : un_lock error\n");
		return -1;
	}
	return rc;
}

void db_rewind(DBHANDLE h)
{
	DB *db = h;
	off_t offset;

	offset = (db->nhash+1)*PTR_SZ;
	if (-1 == (db->idxoff = lseek(db->idxfd, offset+1, SEEK_SET)))
	{
		fprintf(stderr, "%s : lseek error\n", __func__);
	}
}

char* db_nextrec(DBHANDLE h, char *key)
{
	DB *db = h;
	char c;
	char *ptr;

	if (0 > readw_lock(db->idxfd, FREE_OFF, SEEK_SET, 1))
	{
		fprintf(stderr, "%s : readw_lock error\n", __func__);
		return NULL;
	}
	do
	{
		if (0 > _db_readidx(db, 0))
		{
			ptr = NULL;
			goto doreturn;
		}
		ptr = db->idxbuf;
		while (0 != (c = *ptr++) && c == SPACE);
	} while (0 == c);

	if (NULL != key)
	{
		strcpy(key, db->idxbuf);
	}
	ptr = _db_readdat(db);
	++db->cnt_nextrec;

	doreturn:
	if (0 > un_lock(db->idxfd, FREE_OFF, SEEK_SET, 1))
	{
		fprintf(stderr, "%s : un_lock error\n", __func__);
		return NULL;
	}
	return ptr;
}

int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
	struct flock lock;

	lock.l_type = type;
	lock.l_start = offset;
	lock.l_whence = whence;
	lock.l_len = len;
	return fcntl(fd, cmd, &lock);
}
