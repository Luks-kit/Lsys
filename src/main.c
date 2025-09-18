#include "lmem.h"
#include "lstr.h"
#include "lexer.h"
#include "parser.h"
#include "scope.h"
#include "emit.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* minimal syscall-based read of file into mmap */
static char* read_file_to_buffer(const char* path, size_t* out_len) {
    int fd = openat(AT_FDCWD, path, O_RDONLY);
    if (fd < 0) return NULL;
    struct stat st;
    if (fstat(fd, &st) < 0) { close(fd); return NULL; }
    size_t len = (size_t) st.st_size;
    if (len == 0) { close(fd); *out_len = 0; return NULL; }
    void* map = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (map == (void*) -1) return NULL;
    *out_len = len;
    return (char*) map;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        const char* msg = "usage: clearsysc <input.cs> <output.s>\n";
        (void)write(2, msg, lstrlen(msg));
        return 1;
    }
    size_t src_len = 0;
    char* src = read_file_to_buffer(argv[1], &src_len);
    if (!src) {
        const char* msg = "failed to open input\n";
        (void)write(2, msg, lstrlen(msg));
        return 1;
    }

    /* init lexer */
    lexer_t lx;
    lexer_init(&lx, src, src_len);

    /* init parser with lmalloc allocator */
    parser_t p;
    parser_init(&p, &lx, lmalloc);

    /* parse */
    ast_node_t* prog = parser_parse_program(&p);

    /* init scope */
    scope_t sc;
    scope_init(&sc, lmalloc, lfree);

    /* open output file for assembly */
    int outfd = openat(AT_FDCWD, argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outfd < 0) {
        const char* msg = "failed to open output\n";
        (void)write(2, msg, lstrlen(msg));
        return 1;
    }

    /* emitter */
    emitter_t em;
    emitter_init(&em, outfd, &sc, lmalloc, lfree);
    emitter_emit_program(&em, prog);
    emitter_close(&em);

    close(outfd);

    /* munmap source */
    munmap(src, src_len);

    return 0;
}

