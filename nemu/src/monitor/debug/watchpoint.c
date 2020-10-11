#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "monitor/monitor.h"
#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;


void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP *new_wp(){ 
	if(free_==NULL)assert(0);

	WP *f,*h;
	f = free_;
	f->next=NULL;
	free_ = free_->next;

	h = head;
	if (h == NULL){
		head = f;
		h = f;
	}
	else {
		while (h->next) h=h->next;
		h->next = f;
	}
	return f;
}

void free_wp(WP *wp){
	if(free_==NULL){
		free_=wp;
		return;
	}
	WP *f,*p;
	p=free_;
	f=head;
	while (p->next) p=p->next;
	p->next = wp;

	if (head == NULL)assert (0);
	if (head->NO == wp->NO)
		head = head->next;
	else {
		while (f->next != NULL && f->next->NO != wp->NO)f = f->next;
		if (f->next == NULL && f->NO == wp->NO)printf ("Wrong");
		else if (f->next->NO == wp->NO)f->next = f->next->next;
		else assert (0);
	}
	wp->next = NULL;
	wp->result = 0;
	wp->expr[0] = '\0';
	return;
}

bool manip(){
        WP *f;
	f = head;
	bool suc=true;
	while (f != NULL){
		uint32_t tmp_expr = expr (f->expr,&suc);
		if (!suc)assert (1);
		if (tmp_expr != f->result){
			printf("Watchpoint %d: %s changes\n",f->NO,f->expr);
			f->result = tmp_expr;
			return false;
		}
		else 
		f = f->next;
	}
	return true;
}

void delete_wp(int num){
	WP *f;
	f = &wp_pool[num];
	free_wp (f);
}

void info_wp(){
	WP *f;
	f=head;
	if(f==NULL)
		printf("no watchpoints left\n");
	while (f!=NULL)
	{
		printf ("Watchpoint %d: %s = %d\n",f->NO,f->expr,f->result);
		f = f->next;
	}
}
