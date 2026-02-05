
/*
 简单opt
 1. cse
 2. 常量传播
 3. 死代码消除
 4. 强度削减文本替换
*/
/*
这里我简单讲一下实现原理
优化器将整个 qbe ir 文件读入内存，
然后按行分割成字符串数组
每行代表一条指令或声明，
这样可以逐行分析和处理

然后就会有三个阶段的优化：

输入 -> 分割 -> 1 -> 2 -> ... -> 4 -> output
                        |             |
                        v             v
                [常量检测与CSE]  [常量传播]
                        |             |
                        v             v
                [死代码消除] <- [迭代判断]

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/qbe-ir/opt.h"
static char *read_file(const char *path, long *out_len) {
	FILE *f = fopen(path, "rb");
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = malloc(len + 1);
	if (!buf) { fclose(f); return NULL; }
	if (fread(buf, 1, len, f) != (size_t)len) { free(buf); fclose(f); return NULL; }
	buf[len] = '\0';
	fclose(f);
	if (out_len) *out_len = len;
	return buf;
}/*
读取文件内容到内存中
它接收文件路径和一个输出参数来返回文件长度
然后返回指向文件内容的字符指针
函数会自动在文件内容末尾添加空字符('\0')以形成有效的C字符串。
*/
static int write_file(const char *path, const char *buf) {
	FILE *f = fopen(path, "wb");
	if (!f) return 0;
	fputs(buf, f);
	fclose(f);
	return 1;
}
/*
将字符串写入文件
函数接收文件路径和要写入的字符串作为参数
以二进制写模式打开文件
将字符串写入后关闭文件
*/
static int is_reg_char(char c) { 
	return c == '%' 
	|| isalnum((unsigned char)c) 
	|| c=='_' 
	|| c=='$'; 
}

static void str_replace_all(char *s, const char *from, const char *to) {
	size_t flen = strlen(from);
	size_t tlen = strlen(to);
	if (flen == 0) return;
	char *p = s;
	while ((p = strstr(p, from)) != NULL) {
		char prev = (p == s) ? '\0' : p[-1];
		char next = p[flen];
		if ((is_reg_char(prev) || is_reg_char(next)) && prev != '\0') {
			p += flen;
			continue;
		}
		char *line_start = p;
		while (line_start > s && *(line_start - 1) != '\n') {
			line_start--;
		}
		if (strstr(line_start, "call ") || strstr(line_start, "ret ")) {
			p += flen;
			continue;
		}
		if (tlen == flen) {
			memcpy(p, to, tlen);
			p += tlen;
		} else if (tlen < flen) {
			memcpy(p, to, tlen);
			memmove(p + tlen, p + flen, strlen(p + flen) + 1);
			p += tlen;
		} else {
			p += flen;
		}
	}
}
/*
基本替换：
将字符串 s 中所有确切出现的 from 替换为 to
但跳过赋值语句左侧的部分
并避免在函数调用参数和返回语句中进行替换
我们执行粗粒度的替换；
原地操作：支持等长/缩短两种情况的就地替换
避免内存重新分配
*/

static char *optimize_buffer(const char *inbuf) {
	char *buf = strdup(inbuf);
	if (!buf) return NULL;
	size_t cap = 1024;
	size_t count = 0;
	char **lines = malloc(sizeof(char*) * cap);
	char *p = buf;
	while (p) {
		if (count + 1 >= cap) { cap *= 2; lines = realloc(lines, sizeof(char*) * cap); }
		lines[count++] = p;
		char *nl = strchr(p, '\n');
		if (!nl) break;
		*nl = '\0';
		p = nl + 1;
	}
	typedef struct KV { char *k; char *v; } KV;
	KV *consts = NULL; size_t consts_n = 0;
	KV *exprs = NULL; size_t exprs_n = 0;
	int *keep = calloc(count, sizeof(int));
	for (size_t i = 0; i < count; i++) keep[i] = 1;
	for (size_t i = 0; i < count; i++) {
		char *ln = lines[i];
		if (ln[0] == '\0') continue;
		if (ln[0] == '@' || strncmp(ln, "data ", 5) == 0 || strncmp(ln, "export function", 15) == 0) continue;
		char *eq = strstr(ln, "=");
		if (!eq) continue;
		char *lhs_end = eq - 1;
		while (lhs_end > ln && isspace((unsigned char)*lhs_end)) lhs_end--;
		char *lhs_start = lhs_end;
		while (lhs_start > ln && !isspace((unsigned char)*(lhs_start-1))) lhs_start--;
		size_t lhs_len = lhs_end - lhs_start + 1;
		char lhs[64]; if (lhs_len >= sizeof(lhs)) continue; strncpy(lhs, lhs_start, lhs_len); lhs[lhs_len]='\0';
		char *rhs = eq + 1;
		while (*rhs && isspace((unsigned char)*rhs)) rhs++;
		if (strstr(rhs, "copy ")) {
			char *cp = strstr(rhs, "copy ") + 5;
			while (*cp && isspace((unsigned char)*cp)) cp++;
			char val[64]; int vi = 0;
			while (*cp && !isspace((unsigned char)*cp) && vi+1 < (int)sizeof(val)) val[vi++]=*cp++;
			val[vi]='\0';
			if (vi>0) {
				if (isdigit((unsigned char)val[0]) || val[0]=='-' ) {
					consts = realloc(consts, sizeof(KV)*(consts_n+1));
					consts[consts_n].k = strdup(lhs);
					consts[consts_n].v = strdup(val);
					consts_n++;
				}
			}
			continue;
		}

		if (strstr(rhs, "mul ") && (strstr(rhs, ", 2") || strstr(rhs, "2,"))) {
			char *m = strstr(rhs, "mul ")+4;
			char op1[64]; char op2[64]; op1[0]=op2[0]='\0';
			sscanf(m, "%63[^,], %63s", op1, op2);
			char *src = NULL;
			if (strcmp(op1, "2") == 0) src = op2; else if (strcmp(op2, "2") == 0) src = op1;
			if (src) {
				char newrhs[256]; snprintf(newrhs, sizeof(newrhs), "add %s, %s", src, src);
				if (strlen(newrhs) <= strlen(rhs)) {
					strcpy(rhs, newrhs);
				}
			}
		}
		if (!strstr(rhs, "call ") 
		&& !strstr(rhs, "load") 
		&& !strstr(rhs, "jmp ") 
		&& !strstr(rhs, "jnz ")) {
			char tmp[256]; int ti=0; int lastsp=0;
			for (char *c=rhs; *c && ti < (int)sizeof(tmp)-1; c++) {
				if (isspace((unsigned char)*c)) { if (!lastsp) { tmp[ti++]=' '; lastsp=1; } }
				else { tmp[ti++]=*c; lastsp=0; }
			}
			tmp[ti]='\0';
			size_t j;
			for (j=0;j<exprs_n;j++) {
				if (strcmp(exprs[j].k, tmp) == 0) break;
			}
			if (j < exprs_n) {
				const char *prevreg = exprs[j].v;
				for (size_t k = i+1; k < count; k++) {
					str_replace_all(lines[k], lhs, prevreg);
				}
				keep[i] = 0;
				continue;
			} else {
				exprs = realloc(exprs, sizeof(KV)*(exprs_n+1));
				exprs[exprs_n].k = strdup(tmp);
				exprs[exprs_n].v = strdup(lhs);
				exprs_n++;
			}
		}
	}

	/*
	2.
	将常量传播到使用位置
	仅当替换不会增加符号数量时
	*/
	for (size_t c = 0; c < consts_n; c++) {
		const char *reg = consts[c].k;
		const char *val = consts[c].v;
		for (size_t i = 0; i < count; i++) {
			if (!keep[i]) continue;
			if (strstr(lines[i], reg) && strstr(lines[i], "=")) {
				char *eq = strstr(lines[i], "=");
				if (eq) {
					char *lhs_end = eq-1; while (lhs_end > lines[i] && isspace((unsigned char)*lhs_end)) lhs_end--; char *lhs_start = lhs_end; while (lhs_start > lines[i] && !isspace((unsigned char)*(lhs_start-1))) lhs_start--; size_t lhs_len = lhs_end - lhs_start + 1;
					if (lhs_len < 64) {
						char lhs[64]; strncpy(lhs, lhs_start, lhs_len); lhs[lhs_len]='\0';
						if (strcmp(lhs, reg) == 0) continue; //skip LHS 
					}
				}
			}
			if (strstr(lines[i], "call ") || strstr(lines[i], "ret ")) continue;
			
			/*
			尽可能用 val 替换 reg
			并且当 val 的长度小于等于 reg 的长度时
			以避免缓冲区增长
			*/
			if (strlen(val) <= strlen(reg)) {
				str_replace_all(lines[i], reg, val);
			}
		}
	}
	char **used_regs = NULL; size_t used_n = 0;
	for (size_t i = 0; i < count; i++) {
		if (!keep[i]) continue;
		char *ln = lines[i];
		char *eq = strstr(ln, "=");
		char savech = 0;
		char *body = ln;
		if (eq) { savech = *eq; *eq = '\0'; body = eq + 1; }

		for (char *q = body; *q; q++) {
			if (*q == '%') {
				char tmp[64]; int ti=0; tmp[ti++]=*q++; while (*q && (isalnum((unsigned char)*q) || *q=='_' ) && ti+1 < (int)sizeof(tmp)) tmp[ti++]=*q++; tmp[ti]='\0';
				/* record */
				int found=0; for (size_t z=0; z<used_n; z++) if (strcmp(used_regs[z], tmp)==0) { found=1; break; }
				if (!found) { used_regs = realloc(used_regs, sizeof(char*)*(used_n+1)); used_regs[used_n++]=strdup(tmp); }
			}
		}
		if (eq) *eq = savech;
	}

	/* 现在删除左边不在 used_regs 中且右边是纯净的 没有 call/jmp/jnz/ret 的定义行 */
	for (size_t i = 0; i < count; i++) {
		if (!keep[i]) continue;
		char *ln = lines[i];
		char *eq = strstr(ln, "=");
		if (!eq) continue;
		char *lhs_end = eq - 1; while (lhs_end > ln && isspace((unsigned char)*lhs_end)) lhs_end--; char *lhs_start = lhs_end; while (lhs_start > ln && !isspace((unsigned char)*(lhs_start-1))) lhs_start--; size_t lhs_len = lhs_end - lhs_start + 1;
		if (lhs_len >= 64) continue;
		char lhs[64]; strncpy(lhs, lhs_start, lhs_len); lhs[lhs_len]='\0';
		int used = 0; for (size_t z=0; z<used_n; z++) if (strcmp(used_regs[z], lhs) == 0) { used=1; break; }
		if (used) continue;
		char *rhs = eq + 1; 
		if (strstr(rhs, "call ") 
		|| strstr(rhs, "ret ") 
		|| strstr(rhs, "jmp ") 
		|| strstr(rhs, "jnz ")) continue;
		if (strstr(rhs, "copy") 
		|| strstr(rhs, "add") 
		|| strstr(rhs, "sub") 
		|| strstr(rhs, "mul") 
		|| strstr(rhs, "div") 
		|| strstr(rhs, "and") 
		|| strstr(rhs, "or") 
		|| strstr(rhs, "xor") 
		|| strstr(rhs, "extsw") 
		|| strstr(rhs, "cslew")) {
			keep[i] = 0;
		}
	}
	size_t outcap = strlen(buf) + 1;
	char *out = malloc(outcap);
	out[0] = '\0';
	for (size_t i=0;i<count;i++) {
		if (!keep[i]) continue;
		size_t need = strlen(out) + strlen(lines[i]) + 2;
		if (need > outcap) { outcap = need * 2; out = realloc(out, outcap); }
		strcat(out, lines[i]); strcat(out, "\n");
	}
	for (size_t i=0;i<consts_n;i++){ free(consts[i].k); free(consts[i].v); }
	for (size_t i=0;i<exprs_n;i++){ 
		free(exprs[i].k); free(exprs[i].v);
    }
	for (size_t i=0;i<used_n;i++) {
		free(used_regs[i]);
	}
	free(used_regs);
	free(consts); free(exprs); free(keep);
	free(lines); free(buf);
	return out;
}

void qbe_opt_file(const char *filename) {
	long len;
	char *buf = read_file(filename, &len);
	if (!buf) return;
	char *cur = buf;
	for (int iter = 0; iter < 4; iter++) {
		char *next = optimize_buffer(cur);
		if (!next) break;
		if (strcmp(next, cur) == 0) { free(next); break; }
		if (cur != buf) free(cur);
		cur = next;
	}
	write_file(filename, cur);
	if (cur != buf) free(cur);
	free(buf);
}
