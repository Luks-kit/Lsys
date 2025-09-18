// emit.c  -- clean Intel-syntax 64-bit emitter for ClearSys
#include "emit.h"
#include "lstr.h"
#include "scope.h"
#include "parser.h"   // provides ast_node_t
#include "lmem.h"

#include <unistd.h>
#include <stddef.h>
#include <stdint.h>

/* --- robust write helpers --- */
static void fd_write_all(int fd, const char* buf, size_t len) {
    size_t off = 0;
    while (off < len) {
        ssize_t wr = write(fd, buf + off, len - off);
        if (wr <= 0) return; /* give up on error (freestanding) */
        off += (size_t)wr;
    }
}

static void fd_writes(int fd, const char* s) {
    fd_write_all(fd, s, lstrlen(s));
}

/* write three pieces in sequence (convenience) */
static void fd_writes3(int fd, const char* a, const char* b, const char* c) {
    fd_writes(fd, a);
    fd_writes(fd, b);
    fd_writes(fd, c);
}

/* integer -> ascii helper (uses your litoa) */
static void write_int_buf(int v, char* out) { litoa(v, out); }

/* --- emitter context functions --- */
void emitter_init(emitter_t* e, int out_fd, scope_t* scope, void* (*alloc)(size_t), void (*free_fn)(void*)) {
    e->out_fd = out_fd;
    e->scope = scope;
    e->alloc = alloc;
    e->free_fn = free_fn;
}

/* --- low-level Intel templates (64-bit qword / rip-relative) --- */
static void emit_preamble(emitter_t* e) {
    fd_writes(e->out_fd, ".intel_syntax noprefix\n");
    fd_writes(e->out_fd, "section .data\n");
    /* emit variables in order they exist in scope */
    for (size_t i = 0;; ++i) {
        sym_entry_t* ent = scope_entry_at(e->scope, i);
        if (!ent) break;
        fd_writes3(e->out_fd, ent->label, ":\n    dq 0\n", "");
    }
    fd_writes(e->out_fd, "section .text\n");
    fd_writes(e->out_fd, "global _start\n_start:\n");
}

static void emit_mov_reg_imm(emitter_t* e, const char* reg, int imm) {
    char buf[32]; write_int_buf(imm, buf);
    fd_writes3(e->out_fd, "    mov ", reg, ", ");
    fd_writes(e->out_fd, buf);
    fd_writes(e->out_fd, "\n");
}

static void emit_mov_reg_mem_rip(emitter_t* e, const char* reg, const char* label) {
    fd_writes3(e->out_fd, "    mov ", reg, ", qword ptr [rip + ");
    fd_writes3(e->out_fd, label, "]\n", "");
}

static void emit_mov_mem_rip_reg(emitter_t* e, const char* label, const char* reg) {
    fd_writes3(e->out_fd, "    mov qword ptr [rip + ", label, "], ");
    fd_writes3(e->out_fd, reg, "\n", "");
}

static void emit_add_mem_rip_imm(emitter_t* e, const char* label, int imm) {
    char buf[32]; write_int_buf(imm, buf);
    fd_writes3(e->out_fd, "    add qword ptr [rip + ", label, "], ");
    fd_writes(e->out_fd, buf);
    fd_writes(e->out_fd, "\n");
}

static void emit_sub_mem_rip_imm(emitter_t* e, const char* label, int imm) {
    char buf[32]; write_int_buf(imm, buf);
    fd_writes3(e->out_fd, "    sub qword ptr [rip + ", label, "], ");
    fd_writes(e->out_fd, buf);
    fd_writes(e->out_fd, "\n");
}

/* forward declaration */
static void emit_expr(emitter_t* e, ast_node_t* expr);

/* try simple optimization: var = var + imm  (or var = var - imm) */
static int try_emit_simple_mem_binop_optim(emitter_t* e, ast_node_t* stmt) {
    if (!stmt || stmt->type != NODE_ASSIGN) return 0;
    ast_node_t* rhs = stmt->left; /* expression */
    if (!rhs || rhs->type != NODE_BINOP) return 0;
    /* RHS left must be var, RHS right must be int, and var must match LHS target */
    if (!rhs->left || rhs->left->type != NODE_VAR) return 0;
    if (!rhs->right || rhs->right->type != NODE_INT) return 0;
    /* compare names */
    if (stmt->name_len != rhs->left->name_len) return 0;
    if (lstrncmp(stmt->name, rhs->left->name, stmt->name_len) != 0) return 0;

    if (rhs->op == OP_ADD) {
        emit_add_mem_rip_imm(e, scope_get_label(e->scope, stmt->name, stmt->name_len), rhs->right->int_value);
        return 1;
    }
    if (rhs->op == OP_SUB) {
        emit_sub_mem_rip_imm(e, scope_get_label(e->scope, stmt->name, stmt->name_len), rhs->right->int_value);
        return 1;
    }
    /* for mul/div, skip optimization for now */
    return 0;
}

/* emit expression into RAX.
   Uses push/pop to protect temporaries for nested expressions; keeps register interference minimal.
*/
static void emit_expr(emitter_t* e, ast_node_t* expr) {
    if (!expr) return;

    switch (expr->type) {
        case NODE_INT: {
            emit_mov_reg_imm(e, "rax", expr->int_value);
            return;
        }
        case NODE_VAR: {
            char* lbl = scope_get_label(e->scope, expr->name, expr->name_len);
            emit_mov_reg_mem_rip(e, "rax", lbl);
            return;
        }
        case NODE_BINOP: {
            /* Evaluate left into rax */
            emit_expr(e, expr->left);
            /* save left */
            fd_writes(e->out_fd, "    push rax\n");
            /* evaluate right into rax */
            emit_expr(e, expr->right);
            /* pop left into rbx */
            fd_writes(e->out_fd, "    pop rbx\n");
            /* now: rax = right, rbx = left */
            switch (expr->op) {
                case OP_ADD:
                    /* rax = right + left (commutative) */
                    fd_writes(e->out_fd, "    add rax, rbx\n");
                    break;
                case OP_SUB:
                    /* compute left - right into rax */
                    fd_writes(e->out_fd, "    mov rcx, rbx\n    sub rcx, rax\n    mov rax, rcx\n");
                    break;
                case OP_MUL:
                    /* rax = right * left */
                    fd_writes(e->out_fd, "    imul rax, rbx\n");
                    break;
                case OP_DIV:
                    /* left / right : move left into rax, divisor in rcx */
                    fd_writes(e->out_fd, "    mov rcx, rax\n    mov rax, rbx\n    cqo\n    idiv rcx\n");
                    break;
                default:
                    fd_writes(e->out_fd, "    ; error: unsupported binop\n");
                    break;
            }
            return;
        }
        default:
            fd_writes(e->out_fd, "    ; error: unsupported expr node\n");
            return;
    }
}

/* emit one statement (assignment only) */
static void emit_assign_stmt(emitter_t* e, ast_node_t* stmt) {
    if (!stmt || stmt->type != NODE_ASSIGN) return;

    /* try in-place mem optim */
    if (try_emit_simple_mem_binop_optim(e, stmt)) return;

    /* general: evaluate RHS into rax, then store into [label] */
    emit_expr(e, stmt->left);
    char* lbl = scope_get_label(e->scope, stmt->name, stmt->name_len);
    emit_mov_mem_rip_reg(e, lbl, "rax");
}

/* public entry: emit whole program */
int emitter_emit_program(emitter_t* e, ast_node_t* prog) {
    /* register variables in scope in **program order** (top-level statements only) */
    ast_node_t* cur = prog;
    while (cur) {
        if (cur->type == NODE_ASSIGN) {
            scope_get_label(e->scope, cur->name, cur->name_len);
        }
        cur = cur->next;
    }

    /* preamble */
    emit_preamble(e);

    /* emit statements in order */
    cur = prog;
    while (cur) {
        if (cur->type == NODE_ASSIGN) {
            emit_assign_stmt(e, cur);
        } else {
            fd_writes(e->out_fd, "    ; syntax error: unsupported top-level statement\n");
        }
        cur = cur->next;
    }

    /* exit(0) syscall */
    fd_writes(e->out_fd, "    mov rax, 60\n");
    fd_writes(e->out_fd, "    xor rdi, rdi\n");
    fd_writes(e->out_fd, "    syscall\n");
    return 0;
}

void emitter_close(emitter_t* e) { (void)e; }

