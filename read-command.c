// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include "alloc.h"


#ifdef __APPLE__
#include <err.h>
#define error(args...) errc(args)
#else
#include <error.h>
#endif

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

//additional includes
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//end of additional includes

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

//auxiliary functions declarations
bool isWord (char c);
bool isSpecial (char c);
bool isNotValid (char c);
command_t init_command(enum command_type type);
enum command_type cmd_type(char *cc);
command_stream_t init_stream();
command_t store_simple_command (char *c, int *i, size_t size);
bool mayBeOperator (char c);
char *get_operator (char *c, int *i, size_t size);
int precedence (char *operator);
char *get_next_word (char *c, int *i, size_t size);



//global variable
int line_number;

//auxiliary functions
bool isWord (char c)
{
    return (isalpha(c) || isdigit(c) || (c == '!') || (c == '%') || (c == '+') || (c == ',') ||
            (c == '-') || (c == '.') || (c == '/') || (c == ':') || (c == '@') || (c == '^') || (c == '_') );
    
}

bool isSpecial (char c)
{
    return ( (c == ';') || (c == '|') || (c == '(') || (c == ')') || (c == '<') || (c == '>') );
}

bool isNotValid (char c)
{
    return ( (c!=' ') && (c!='\t') && (c!='#') && (c!='\n') && (c!='&') && (!isWord(c)) && (!isSpecial(c)));
}
//end of auxiliary functions


//initialize command
enum command_type cmd_type(char *cc)
{
    if(!strcmp(cc,"&&")) return AND_COMMAND;
    if(!strcmp(cc,"||")) return OR_COMMAND;
    if(!strcmp(cc,"|")) return PIPE_COMMAND;
    if(!strcmp(cc,";")) return SEQUENCE_COMMAND;
    if(!strcmp(cc,"(")) return SUBSHELL_COMMAND;
    else {
        err(1, "invalid type");
        return SIMPLE_COMMAND;
    }
    
}

//initialize command
command_t init_command(enum command_type type)
{
    command_t command=(command_t)checked_malloc(sizeof(struct command));
    command->type=type;
    command->status=-1;
    command->input=NULL;
    command->output=NULL;
    return command;
}


//initialize command stream
command_stream_t init_stream()
{
    command_stream_t stream=(command_stream_t) checked_malloc(sizeof(struct command_stream));
    stream->head=NULL;
    stream->tail=NULL;
    stream->cursor=NULL;
    return stream;
}


//simple command consists of one or more words, with no operators, \n, #...others in between
command_t store_simple_command (char *c, int *i, size_t size)
{
    command_t command= init_command(SIMPLE_COMMAND);
    int size_word=1;
    int count_w=0;
    command->u.word=(char**)malloc(sizeof(char*)*size_word);
    
    while ((*i)<size)
    {
        char tmp_c=c[*i];
        
        //if meet operator, then return command
        if (tmp_c==' ' || tmp_c=='\t')
        {
            (*i)++;
        }
        else if (isWord(tmp_c))
        {
            char *w=get_next_word(c, i, size);
            
            if (count_w==size_word)
            {
                size_word=size_word*2;
                command->u.word=realloc(command->u.word,sizeof(char*)*size_word);
            }
            command->u.word[count_w]=w;//entire word
            count_w++;
            command->u.word[count_w]=NULL;
        }
        else if (isNotValid(tmp_c))
        {
            fprintf(stderr,"line %d: character, '%c', is not valid. \n", line_number, tmp_c );
            exit(1);
        }
        else//meet an operator of any sort or others ...i.e. \n, < >  #
        {
            (*i)--;//can deal with it later in make_comamnd_tree function
            break;
        }
        
    }
    
   
    return command;
}



char *get_next_word (char *c, int *i, size_t size)
{

    int starting=*i;
    int size_word=0;
    
    
    
    while (isWord(c[*i]) && (*i)<size)
    {
        size_word++;
        (*i)++;
    }
    
    char *word=(char *)checked_malloc(sizeof(char)*(size_word+1) );

    if (size_word == 0)
        return NULL;
    
    int j=0;
    while (j<size_word)
    {
        word[j]=c[starting+j];
        j++;
    }
    word[size_word]='\0';

    
    return word;
}

//create subshell command
command_t store_subshell_command()
{
    command_t command= init_command(SUBSHELL_COMMAND);
    return command;
}


//create the tree. operator stack and command stack

//precedance
int precedence (char *operator)
{
 /*
  lowest --> highest
  ; \n    1
  &&/||   2
  |       3
*/
    if (strcmp(operator,";")==0)
        return 1;
    else if (strcmp(operator,"\n")==0)
        return 1;
    else if (strcmp(operator,"&&")==0)
        return 2;
    else if (strcmp(operator,"||")==0)
        return 2;
    else if (strcmp(operator,"|")==0)
        return 3;
    else
        return -1;
}


//get operator function??

//implement stacks


/*stacks to build the command tree         */
int OPERATOR_LEN=16;
int COMMAND_LEN=16;//root of command tree. still 16??

//stack defs
struct op_stack
{
    char **operator;
    int len;
    int top;
}op_s;

struct cmd_stack
{
    command_t* command;
    int len;
    int top;
}cmd_s;

//push pop

bool push_op(struct op_stack *op, char* cc)
{
    if (op->len==op->top)
        return false;
    else{
        op->top++;
        op->operator[op->top]=cc;
        return true;
    }
}

bool pop_op(struct op_stack *op, char* cc)
{
    if(op->top==0)
    {
        fprintf(stderr,"line %d: The command is not valid. \n",line_number);
        exit(1);
        return false;
    }
    else
    {
        cc=op->operator[op->top];
        op->top--;
        return true;
    }
}

bool push_cmd(struct cmd_stack *cmd, command_t cc)
{
    if (cmd->len==cmd->top)
        return false;
    else{
        cmd->top++;
        cmd->command[cmd->top]=cc;
        return true;
    }
}

bool pop_cmd(struct cmd_stack *cmd, command_t cc)
{
    if(cmd->top==0)
    {
        //print out there is syntax error and exit
        fprintf(stderr,"line %d: The command is not valid. \n",line_number);
        exit(1);
        return false;
    }
    else
    {
        cc=cmd->command[cmd->top];
        cmd->top--;
        return true;
    }
}


/*make command stream              */
command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
    size_t buf_size=2048;
    size_t read_size=0;
    int c;
    char *buf=checked_malloc(buf_size);
    
    //get everything in a buffer
    
    while ((c=get_next_byte(get_next_byte_argument))!=EOF)
    {
        buf[read_size++]=c;
        if(read_size==buf_size)
        {
            buf_size=buf_size*2;
            buf=(char*)checked_grow_alloc(buf,&buf_size );
        }
    }
    
    //initialize operator stack and command stack
    op_s.operator=checked_malloc(OPERATOR_LEN*sizeof(char));
    op_s.len=0;
    op_s.top=0;
    
    cmd_s.command=checked_malloc(COMMAND_LEN*sizeof(command_t));
    cmd_s.len=0;
    cmd_s.top=0;
    
    
    int s=0;
    while (s<read_size)
    {
        char c_tmp=buf[s];
        //char operator [3];
        if ((c_tmp==' ') || (c_tmp=='\t'))
        {
            s++;
        }
        else if (c_tmp=='\n')//need to deal with double \n
        {
            line_number++;
            s++;
        }
        else if (c_tmp=='(')
        {
            push_op(&op_s,"(");
            s++;
        }
        else if (c_tmp==';')
        {
            if (op_s.len==0)//if operator stack is empty, push
                push_op(&op_s, ";");
            else
            {
                while ( (precedence(op_s.operator[op_s.top])>=precedence(";")) &&(strcmp(op_s.operator[op_s.top],"(")!=0))
                {
                    char* tmp_op=NULL;
                    pop_op(&op_s, tmp_op);
                    command_t tmp=init_command(cmd_type(tmp_op));
                    pop_cmd( &cmd_s, tmp->u.command[1]);//output error when nothing in command stack
                    pop_cmd(&cmd_s, tmp->u.command[0]);
                    push_cmd(&cmd_s, tmp);
                }
                push_op(&op_s, ";");
                s++;
            }
        }else if (c_tmp=='|')
        {
            if ( (s+1)<read_size && buf[s+1]=='|')//it's ||
            {
                if (op_s.len==0)
                    push_op(&op_s, "||");
                else{
                    while ( (precedence(op_s.operator[op_s.top])>=precedence("||")) &&(strcmp(op_s.operator[op_s.top],"||")!=0))
                    {
                        char* tmp_op=NULL;
                        pop_op(&op_s, tmp_op);
                        command_t tmp=init_command(cmd_type(tmp_op));
                        pop_cmd( &cmd_s, tmp->u.command[1]);
                        pop_cmd(&cmd_s, tmp->u.command[0]);
                        push_cmd(&cmd_s, tmp);
                    }
                    push_op(&op_s, "||");
                    
                }
                s+=2;
            }
            else //it's |
            {
                
                
                if (op_s.len==0)
                    push_op(&op_s, "|");
                else
                {
                    while ( (precedence(op_s.operator[op_s.top])>=precedence("|")) &&(strcmp(op_s.operator[op_s.top],"|")!=0))
                    {
                        char* tmp_op=NULL;
                        pop_op(&op_s, tmp_op);
                        command_t tmp=init_command(cmd_type(tmp_op));
                        pop_cmd( &cmd_s, tmp->u.command[1]);//if pop is false, return error statement
                        pop_cmd(&cmd_s, tmp->u.command[0]);
                        push_cmd(&cmd_s, tmp);
                    }
                    push_op(&op_s, "|");
                    
                }
                s++;
            }
            break;

        }else if (c_tmp=='&')
        {
            if ( (s+1)<read_size && buf[s+1]=='&')//it's &&
            {
                if (op_s.len==0)
                    push_op(&op_s, "&&");
                else{
                    while ( (precedence(op_s.operator[op_s.top])>=precedence("&&")) &&(strcmp(op_s.operator[op_s.top],"&&")!=0))
                    {
                        char* tmp_op=NULL;
                        pop_op(&op_s, tmp_op);
                        command_t tmp=init_command(cmd_type(tmp_op));
                        pop_cmd( &cmd_s, tmp->u.command[1]);
                        pop_cmd(&cmd_s, tmp->u.command[0]);
                        push_cmd(&cmd_s, tmp);
                    }
                    push_op(&op_s, "&&");
                    
                }
                s+=2;
            }
            else //it's &, print error
            {
                fprintf(stderr,"line %d: The character '%c' is not valid. \n",line_number,c_tmp);
                exit(1);
            }
        }else if (c_tmp=='<')
        {
            
        }else if (c_tmp=='>')
        {
            
        }else if (c_tmp==')')
        {
            if (op_s.len==0)
            {
                push_op(&op_s, ")");//????
                //fprintf(stderr,"line %d: No '(' that matches ')'. \n",line_number);
                //exit(1);
            }
            else{//need fix
                
                while ((strcmp(op_s.operator[op_s.top],"(")!=0))
                {
                    char* tmp_op=NULL;
                    pop_op(&op_s, tmp_op);
                    command_t tmp=init_command(cmd_type(tmp_op));
                    pop_cmd( &cmd_s, tmp->u.command[1]);
                    pop_cmd(&cmd_s, tmp->u.command[0]);
                    push_cmd(&cmd_s, tmp);
                }
                command_t tmp=init_command(SUBSHELL_COMMAND);
                // input u.subshell_cmd[0]== root inside the subshell ??
                //more work...
                push_cmd(&cmd_s, tmp);
            }

        }else if (c_tmp=='#')
        {
            while (s<read_size)
            {
                if (buf[s]=='\n')
                {
                    line_number++;
                    break;
                }
                (s)++;
            }
        
        }
        else if (isWord(c_tmp))
        {
            command_t com_tmp=store_simple_command(buf, &s, read_size);//need to double check this function
            push_cmd(&cmd_s, com_tmp);
        }
        else if (isNotValid(c_tmp))
        {
            fprintf(stderr,"line %d: The character '%c' is not valid. \n",line_number,c_tmp);
            exit(1);
        }

    }
    
    
    
    
    
    
    
    
    //end of stacks
    command_stream_t stream=init_stream();
    
    
    
    return stream;
    
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
    
    
  //error (1, 0, "command reading not yet implemented");
  //return 0;
}
    
command_t
read_command_stream (command_stream_t s)
{
    if (s->cursor==NULL)
    {
        return NULL;
    }
    command_t tmp=s->cursor->c;
    s->cursor=s->cursor->next;
    return tmp;
    
  
}
