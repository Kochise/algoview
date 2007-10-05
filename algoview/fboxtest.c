#include "fbox.h"

#define STRING_BUFFER_SIZE	8192

SML_LIST_DEFINE(fbox)

SML_ARRAY_DEFINE(fbox_p)

fbox box_list,*box_function_start,*box_function;
fbox_p_array *address;

void	fbox_link(){
	fbox *n;
	fbox_p *i;
	
	for(n=box_list.next;n;n=n->next){
		if(n->a1){
			i=get_fbox_p_array(address,abs(n->a1));
			n->t=*i;
		}
		if(n->a2){
			i=get_fbox_p_array(address,abs(n->a2));
			n->f=*i;
		}
	}
}

