#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"
#include "mlfq_imp.h"

void init_queue(struct proc_list *l){
	l->used_size = 0;
	l->begin = 0;
}

bool is_empty(struct proc_list *l){
	return l->used_size == 0;
}

void enqueue(struct proc_list *l,struct proc *p){
	if (l->used_size < (NPROC/NPRIO)){
		int pos = (l->used_size + l->begin) % (NPROC/NPRIO);
		l->used_list[pos] = p;
		l->used_size++;
	}
	else{
	 	panic("Full proc table");
	}
}


struct proc *dequeue(struct proc_list *l){
	struct proc *p = 0;
	if (l->used_size > 0){
		p = l->used_list[l->begin];
		l->begin = (l->begin + 1) % (NPROC/NPRIO);
		l->used_size--;
		return p;
	}	
	else{
		panic("Empty proc table");
	}
}




