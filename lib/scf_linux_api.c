intptr_t scf_syscall(intptr_t number, ...);

const intptr_t SCF_read     = 0;
const intptr_t SCF_write    = 1;
const intptr_t SCF_open     = 2;
const intptr_t SCF_close    = 3;
const intptr_t SCF_stat     = 4;
const intptr_t SCF_fstat    = 5;
const intptr_t SCF_lstat    = 6;
const intptr_t SCF_poll     = 7;
const intptr_t SCF_lseek    = 8;
const intptr_t SCF_mmap     = 9;
const intptr_t SCF_mprotect = 10;
const intptr_t SCF_munmap   = 11;

const int      O_RDONLY     = 0;
const int      O_WRONLY     = 1;
const int      O_RDWR       = 2;
const int      O_CREAT      = 0x40;
const int      O_TRUNC      = 0x200;
const int      O_NONBLOCK   = 0x800;

const int      EBADF        = 9;
const int      EAGAIN       = 11;
const int      ENOMEM       = 12;
const int      EINVAL       = 22;

const int      PROT_READ    = 1;
const int      PROT_WRITE   = 2;
const int      PROT_EXEC    = 4;

const int      MAP_SHARED   = 1;
const int      MAP_PRIVATE  = 2;

struct timespec
{
	uint64_t  tv_sec;        /* seconds */
	intptr_t  tv_nsec;       /* nanoseconds */
};

struct stat
{
	uint64_t  st_dev;         /* ID of device containing file */
	uint64_t  st_ino;         /* Inode number */
	uint32_t  st_mode;        /* File type and mode */
	uint64_t  st_nlink;       /* Number of hard links */
	uint32_t  st_uid;         /* User ID of owner */
	uint32_t  st_gid;         /* Group ID of owner */
	uint64_t  st_rdev;        /* Device ID (if special file) */
	uint64_t  st_size;        /* Total size, in bytes */
	uint64_t  st_blksize;     /* Block size for filesystem I/O */
	uint64_t  st_blocks;      /* Number of 512B blocks allocated */

	struct timespec st_atim;  /* Time of last access */
	struct timespec st_mtim;  /* Time of last modification */
	struct timespec st_ctim;  /* Time of last status change */
};

