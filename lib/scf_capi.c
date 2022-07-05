
int   printf(const char* fmt, ...);

void* malloc (uintptr_t size);
void* calloc (uintptr_t n, uintptr_t size);
void* realloc(void* p,     uintptr_t size);
int   free   (void* p);

void  scf__release_pt (void* objdata);

void* scf__auto_malloc(uintptr_t size);
void  scf__auto_ref(void* data);

void  scf__auto_freep (void** pp, scf__release_pt* release);
void  scf__auto_freep_array(void** pp, int nb_pointers, scf__release_pt* release);
void  scf__auto_free_array (void** pp, int size, int nb_pointers, scf__release_pt* release);

uintptr_t strlen (const char *s);
int       strcmp (const char *s1, const char *s2);
int       strncmp(const char *s1, const char *s2,  uintptr_t n);
char*     strncpy(char *dst,      const char *src, uintptr_t n);
int       memcpy (void* dst,      const void* src, uintptr_t n);
int       memcmp (void* dst,      const void* src, uintptr_t n);

