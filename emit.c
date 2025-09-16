#include "emit.h"
#include "lstr.h"
#include "lmem.h"
#include <unistd.h> /* for write */
#include <stddef.h>

/* small write wrapper */
static void fd_write(int fd, const char* buf, size_t len) {
    /* attempt a single write; production code should loop until all bytes written */
   (void)write(fd, buf, len);
}

/* helper: write nul-terminated string */
static void fd_writes(int fd, const char* s) {
    size_t l = lstrlen(s);
    fd_write(fd, s, l);
}

/* init emitter */
void emitter_init(emitter_t* e, int out_fd, scope_t* scope, void* (*alloc)(size_t), void (*free_fn)(void*)) {
    e->out_fd = out_fd;
    e->scope = scope;
    e->alloc = alloc;
    e->free_fn = free_fn;
}

/* emit header (data + text) skeleton */
static void emit_preamble(emitter_t* e) {
    fd_writes(e->out_fd, "    .section .data\n");
    /* emit variables from scope */
    for (size_t i=0;;++i) {
        sym_entry_t* ent = scope_entry_at(e->scope, i);
        if (!ent) break;
        fd_writes(e->out_fd, ent->label);
        fd_writes(e->out_fd, ":\n    .long 0\n");
    }
    fd_writes(e->out_fd, "    .section .text\n    .global _start\n_start:\n");
}

/* convert integer to string (uses litoa from lstr) */
static void emit_int_to_buf(int v, char* outbuf) { litoa(v, outbuf); }

/* emit load var into eax: mov eax, [label] */
static void emit_load_var(emitter_t* e, const char* label) {
    fd_writes(e->out_fd, "    movl ");
    fd_writes(e->out_fd, label);
    fd_writes(e->out_fd, "(%rip), %eax\n");
}

/* emit store eax -> var: mov [label], eax */
static void emit_store_var(emitter_t* e, const char* label) {
    fd_writes(e->out_fd, "    movl %eax, ");
    fd_writes(e->out_fd, label);
    fd_writes(e->out_fd, "(%rip)\n");
}

/* emit immediate into eax */
static void emit_load_imm(emitter_t* e, int imm) {
    char buf[32];
    emit_int_to_buf(imm, buf);
    fd_writes(e->out_fd, "    movl $");
    fd_writes(e->out_fd, buf);
    fd_writes(e->out_fd, ", %eax\n");
}

/* emit arithmetic op assuming left result in %eax and right in %ebx */
static void emit_binop_instr(emitter_t* e, binop_type_t op) {
    switch (op) {
        case OP_ADD: fd_writes(e->out_fd, "    addl %ebx, %eax\n"); break;
        case OP_SUB: fd_writes(e->out_fd, "    subl %ebx, %eax\n"); break;
        case OP_MUL: fd_writes(e->out_fd, "    imull %ebx, %eax\n"); break;
        case OP_DIV: /* idiv uses eax:edx / ebx; produce safe sequence (cdr) */ 
            fd_writes(e->out_fd, "    cltd\n    idivl %ebx\n");
            break;
        default: break;
    }
}

/* recursively evaluate expression and leave result in %eax */
static void emit_expr(emitter_t* e, ast_node_t* expr) {
    if (!expr) return;
    switch (expr->type) {
        case NODE_INT:
            emit_load_imm(e, expr->int_value);
            break;
        case NODE_VAR: {
            /* get label for var (create if necessary) */
            char* lbl = e->scope->entries ? NULL : NULL; /* quiet compiler; real lookup below */
            lbl = scope_get_label(e->scope, expr->name, expr->name_len);
            emit_load_var(e, lbl);
            break;
        }
        case NODE_BINOP: {
            /* Evaluate left into eax */
            emit_expr(e, expr->left);
            /* move eax -> ebx to preserve left */
            fd_writes(e->out_fd, "    movl %eax, %ebx\n");
            /* evaluate right into eax */
            emit_expr(e, expr->right);
            /* move right (in eax) -> ebx, swap so we can use convention left in eax right in ebx */
            /* But we already put left in ebx; we need right in ebx. So swap: move eax->ecx, move ebx->eax, move ecx->ebx */
            fd_writes(e->out_fd, "    movl %eax, %ecx\n    movl %ebx, %eax\n    movl %ecx, %ebx\n");
            emit_binop_instr(e, expr->op);
            break;
        }
        default:
            break;
    }
}

/* emit single statement (only assignment supported) */
static void emit_stmt(emitter_t* e, ast_node_t* stmt) {
    if (!stmt) return;
    if (stmt->type == NODE_ASSIGN) {
        /* ensure variable exists and has label */
        char* lbl = scope_get_label(e->scope, stmt->name, stmt->name_len);
        /* evaluate expression -> %eax */
        emit_expr(e, stmt->left);
        /* store %eax -> [lbl] */
        emit_store_var(e, lbl);
    }
}

/* emit program (linked list of statements) */
int emitter_emit_program(emitter_t* e, ast_node_t* prog) {
    /* build scope labels for all variables used in program by scanning AST */
    ast_node_t* cur = prog;
    while (cur) {
        if (cur->type == NODE_ASSIGN) {
            scope_get_label(e->scope, cur->name, cur->name_len);
        }
        cur = cur->next;
    }

    /* emit .data/.text and program */
    emit_preamble(e);

    cur = prog;
    while (cur) {
        emit_stmt(e, cur);
        cur = cur->next;
    }

    /* simple exit via syscall exit(0) */
    fd_writes(e->out_fd, "    movl $60, %eax\n    xorl %edi, %edi\n    syscall\n");
    return 0;
}

void emitter_close(emitter_t* e) {
    (void)e;
}

