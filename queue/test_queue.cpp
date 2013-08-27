
struct listhead
{
    struct data *lh_first;
}head;

struct data
{ 
    struct list_entry
    {
        struct data *le_next;
        struct data **le_prev;
    }
}*n1,*n2,*np;

LIST_INIT(&head);                       /* Initialize the list. */
 
n1 = malloc(sizeof(struct data));      /* Insert at the head. */

LIST_INSERT_HEAD(&head, n1, list_entry); // (head, elm, field) 

if (((n1)->list_entry.le_next = (head)->lh_first) != NULL)	
	(head)->lh_first->list_entry.le_prev = &(n1)->list_entry.le_next;
(head)->lh_first = (n1);					
(n1)->list_entry.le_prev = &(head)->lh_first;			
 
n2 = malloc(sizeof(struct data));      /* Insert after. */
LIST_INSERT_AFTER(n1, n2, list_entry);
/* Forward traversal. */
LIST_FOREACH(np, &head, list_entry)
np-> 
 
while (head.lh_first != NULL)           /* Delete. */
L
