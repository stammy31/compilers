#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "general.h"
#include "symbol.h"
#include "generator.h"
#include "quads.h"

Program_strings program_strings, program_strings_tail;
Program_strings temp;

int unique, counterg=0;
char * unit_name;
char * current_unit;
int call_counter;
int param_byte_table[256];
int inception_function_table[256];

void generator(int * externs, int * offsets){
    int i;
    char program[256];
    FILE * fp, * fp2;

    unique = 0;
    program_strings = (program_string_t *) new(sizeof(program_string_t));
    program_strings = NULL;
    program_strings_tail = (program_string_t *) new(sizeof(program_string_t));
	program_strings_tail = program_strings;

    fp = fopen("a.asm", "w+");
    fp2 = fopen("quads.txt", "r");
    fscanf(fp2, "%[^\t\n]\n", program);
    fprintf(fp, "xseg segment public'code'\n\tassume cs:xseg, ds:xseg, ss:xseg\n\torg 100h\nLIVENESS = 0\nmain proc near");
    fprintf(fp, "\n\tmov cx, OFFSET DGROUP:_start_of_space\n\tmov word ptr _space_from, cx\n\tmov word ptr _next, cx\n\tmov ax, 0FFFEh\n\tsub ax, cx\n\txor dx, dx\n\tmov bx, 3\n\tidiv bx\n\tand ax, 0FFFEh ; even number!\n\tadd cx, ax\n\tmov word ptr _limit_from, cx\n\tmov word ptr _space_to, cx\n\tadd cx, ax\n\tmov word ptr _limit_to, cx");

    for(i = 0; i < unit_counter; i++){
        fprintf(fp, "\n\tmov ax, OFFSET _%s_call_table\n\tcall near ptr _register_call_table", units[i]);
    }

    printexterns2(fp, externs);

    fprintf(fp, "\n\tcall near ptr _%s\n_ret_of_main:\n\tmov ax,4C00h\n\tint 21h\nmain endp\n", program);

    for(i = 0; i < nextquad; i++){
        fprintf(fp, "@%d:\n", i);
        generate(consume_quad(fp2), fp, offsets[counterg]);
        fprintf(fp, "\n");
    }

    printstrings(fp);
    printexterns(fp, externs);
    fprintf(fp, "\tpublic _next\n\tpublic _space_from\n\tpublic _limit_from\n\tpublic _space_to\n\tpublic _limit_to\n\tpublic _ret_of_main\n\n\t_space_from dw ?\n\t_limit_from dw ?\n\t_space_to dw ?\n\t_limit_to dw ?\n\t_next dw ?\n\n\txseg ends\n\n_DATA_END segment byte public 'stack'\n_start_of_space label byte\n_DATA_END ends\n\nDGROUP group xseg,_DATA_END\n\n\tend main\n"
);
    fclose(fp);
}

Interpreted_quad consume_quad(FILE * fp){
    Interpreted_quad interpreted_quad;
    char * line = NULL;
    size_t linesize;

    linesize = 0;
    getline(&line, &linesize, fp);
    interpreted_quad.id           = atoi(strtok (line,"\v"));
    interpreted_quad.quad         = strdup(strtok (NULL,"\v"));
    interpreted_quad.arg1         = strdup(strtok (NULL,"\v"));
    interpreted_quad.arg2         = strdup(strtok (NULL,"\v"));
    interpreted_quad.dest         = strdup(strtok (NULL,"\v"));
    interpreted_quad.nesting      = strdup(strtok (NULL,"\v"));
    interpreted_quad.arg1_pm      = strdup(strtok (NULL,"\v"));
    interpreted_quad.arg1_type    = strdup(strtok (NULL,"\v"));
    interpreted_quad.arg1_nesting = strdup(strtok (NULL,"\v"));
    interpreted_quad.arg1_kind    = strdup(strtok (NULL,"\v"));
    interpreted_quad.arg1_offset  = strdup(strtok (NULL,"\v"));
    interpreted_quad.arg2_pm      = strdup(strtok (NULL,"\v"));
    interpreted_quad.arg2_type    = strdup(strtok (NULL,"\v"));
    interpreted_quad.arg2_nesting = strdup(strtok (NULL,"\v"));
    interpreted_quad.arg2_kind    = strdup(strtok (NULL,"\v"));
    interpreted_quad.arg2_offset  = strdup(strtok (NULL,"\v"));
    interpreted_quad.dest_pm      = strdup(strtok (NULL,"\v"));
    interpreted_quad.dest_type    = strdup(strtok (NULL,"\v"));
    interpreted_quad.dest_nesting = strdup(strtok (NULL,"\v"));
    interpreted_quad.dest_kind    = strdup(strtok (NULL,"\v"));
    interpreted_quad.dest_offset  = strdup(strtok (NULL,"\v"));

    free(line);

    if(strcmp(interpreted_quad.quad, "unit") == 0){
        line = NULL;
        linesize = 0;
        getline(&line, &linesize, fp);
        printf("%s\n", line);
        free(line);
    }

    return interpreted_quad;
}


void generate(Interpreted_quad quad, FILE * fp, int offset){
    char * temp_label, * temp_name, * temp_endof;
	int return_param;
    if (strcmp(quad.quad, ":=") == 0){
        if(strcmp(quad.arg1_kind, "integer") == 0 || strcmp(quad.arg1_kind, "iarray") == 0 || strcmp(quad.arg1_kind, "list") == 0 || strcmp(quad.dest_kind, "integer") == 0 || strcmp(quad.dest_kind, "iarray") == 0 || strcmp(quad.dest_kind, "list") == 0){
            load("ax", quad.arg1, fp, quad.arg1_pm, quad.arg1_type, quad.arg1_nesting, quad.nesting, quad.arg1_kind, quad.arg1_offset);
            store("ax", quad.dest, fp, quad.dest_pm, quad.dest_type, quad.dest_nesting, quad.nesting, quad.dest_kind, quad.dest_offset);
        } else {
            load("al", quad.arg1, fp, quad.arg1_pm, quad.arg1_type, quad.arg1_nesting, quad.nesting, quad.arg1_kind, quad.arg1_offset);
            store("al", quad.dest, fp, quad.dest_pm, quad.dest_type, quad.dest_nesting, quad.nesting, quad.dest_kind, quad.dest_offset);
        }
    }
    else if ((strcmp(quad.quad, "+") == 0) || (strcmp(quad.quad, "-") == 0)){
        load("ax", quad.arg1, fp, quad.arg1_pm, quad.arg1_type, quad.arg1_nesting, quad.nesting, quad.arg1_kind, quad.arg1_offset);
        load("dx", quad.arg2, fp, quad.arg2_pm, quad.arg2_type, quad.arg2_nesting, quad.nesting, quad.arg2_kind, quad.arg2_offset);
        if (strcmp(quad.quad, "+") == 0)
            fprintf(fp, "\tadd ax,dx\n");
        else
            fprintf(fp, "\tsub ax,dx\n");
        store("ax", quad.dest, fp, quad.dest_pm, quad.dest_type, quad.dest_nesting, quad.nesting, quad.dest_kind, quad.dest_offset);
    }
    else if (strcmp(quad.quad, "*") == 0){
        load("ax", quad.arg1, fp, quad.arg1_pm, quad.arg1_type, quad.arg1_nesting, quad.nesting, quad.arg1_kind, quad.arg1_offset);
        load("cx", quad.arg2, fp, quad.arg2_pm, quad.arg2_type, quad.arg2_nesting, quad.nesting, quad.arg2_kind, quad.arg2_offset);
        fprintf(fp, "\timul cx\n");
        store("ax", quad.dest, fp, quad.dest_pm, quad.dest_type, quad.dest_nesting, quad.nesting, quad.dest_kind, quad.dest_offset);
    }
    else if ((strcmp(quad.quad, "/") == 0) || (strcmp(quad.quad, "%") == 0)){
        load("ax", quad.arg1, fp, quad.arg1_pm, quad.arg1_type, quad.arg1_nesting, quad.nesting, quad.arg1_kind, quad.arg1_offset);
        fprintf(fp, "\tcwd\n");
        load("cx", quad.arg2, fp, quad.arg2_pm, quad.arg2_type, quad.arg2_nesting, quad.nesting, quad.arg2_kind, quad.arg2_offset);
        fprintf(fp, "\tidiv cx\n");
        if (strcmp(quad.quad, "/") == 0)
            store("ax", quad.dest, fp, quad.dest_pm, quad.dest_type, quad.dest_nesting, quad.nesting, quad.dest_kind, quad.dest_offset);
        else
            store("dx", quad.dest, fp, quad.dest_pm, quad.dest_type, quad.dest_nesting, quad.nesting, quad.dest_kind, quad.dest_offset);
    }
    else if ((strcmp(quad.quad, "=") == 0) || (strcmp(quad.quad, "<>") == 0) || (strcmp(quad.quad, "<") == 0) || (strcmp(quad.quad, ">") == 0) || (strcmp(quad.quad, "<=") == 0) || (strcmp(quad.quad, ">=") == 0)){
		if ((strcmp(quad.arg1_kind, "boolean") == 0) || (strcmp(quad.arg1_kind, "char") == 0) ){
			load("al", quad.arg1, fp, quad.arg1_pm, quad.arg1_type, quad.arg1_nesting, quad.nesting, quad.arg1_kind, quad.arg1_offset);
			load("dl", quad.arg2, fp, quad.arg2_pm, quad.arg2_type, quad.arg2_nesting, quad.nesting, quad.arg2_kind, quad.arg2_offset);
			fprintf(fp, "\tcmp al,dl\n");
		} else {
			load("ax", quad.arg1, fp, quad.arg1_pm, quad.arg1_type, quad.arg1_nesting, quad.nesting, quad.arg1_kind, quad.arg1_offset);
			load("dx", quad.arg2, fp, quad.arg2_pm, quad.arg2_type, quad.arg2_nesting, quad.nesting, quad.arg2_kind, quad.arg2_offset);
			fprintf(fp, "\tcmp ax,dx\n");
		}

        temp_label = label(quad.dest);

        if (strcmp(quad.quad, "=") == 0)
            fprintf(fp, "\tje %s\n", temp_label);
        if (strcmp(quad.quad, "<>") == 0)
            fprintf(fp, "\tjne %s\n", temp_label);
        if (strcmp(quad.quad, ">") == 0)
            fprintf(fp, "\tjg %s\n", temp_label);
        if (strcmp(quad.quad, "<") == 0)
            fprintf(fp, "\tjl %s\n", temp_label);
        if (strcmp(quad.quad, ">=") == 0)
            fprintf(fp, "\tjge %s\n", temp_label);
        if (strcmp(quad.quad, "<=") == 0)
            fprintf(fp, "\tjle %s\n", temp_label);
    }
    else if (strcmp(quad.quad, "unit") == 0){
        unit_name = name(quad.arg1);
        current_unit = strdup(quad.arg1);
        call_counter = 1;
		inception_function_table[call_counter] = 0;
        fprintf(fp, "%s proc near\n\tpush bp\n\tmov bp,sp\n\tsub sp,%d\n", unit_name, offset);
    }
    else if (strcmp(quad.quad, "endu") == 0){
        unit_name = name(quad.arg1);
        temp_endof = endof(unit_name);
        fprintf(fp, "%s: mov sp,bp\n\tpop bp\n\tret\n%s endp\n", temp_endof, unit_name);
        print_call_table(fp, current_unit, call_counter, offset, param_byte_table, inception_function_table);
		counterg++;
    }
    else if (strcmp(quad.quad, "call") == 0){
        temp_name = name(quad.dest);
		if (strcmp(quad.dest_kind, "procedure") == 0)
        	fprintf(fp, "\tsub sp,2\n");
        updateAL(fp, quad.dest_nesting, quad.nesting);
        fprintf(fp, "\tcall near ptr %s", temp_name);
		if (strcmp(quad.dest_kind, "procedure") == 0)
			return_param = 0;
		else if (strcmp(quad.dest_kind, "byte") == 0)
			return_param = 1;
		else
			return_param = 2;
		inception_function_table[call_counter] = inception_function_table[call_counter] - atoi(quad.dest_offset) - return_param;
		param_byte_table[call_counter] = atoi(quad.dest_offset);
        fprintf(fp, "\n@%s_call_%d:", current_unit, call_counter++);
		inception_function_table[call_counter] = inception_function_table[call_counter-1];
        fprintf(fp, "\n\tadd sp,%d+4\n", atoi(quad.dest_offset));
    }
    else if (strcmp(quad.quad, "ret") == 0){
        temp_endof = endof(unit_name);
        fprintf(fp, "\tjmp %s\n", temp_endof);
    }
    else if (strcmp(quad.quad, "retv") == 0){
        if (strcmp(quad.arg1_kind, "integer") == 0 || strcmp(quad.arg1_kind, "iarray") == 0 || strcmp(quad.arg1_kind, "list") == 0) {
            load("ax", quad.arg1, fp, quad.arg1_pm, quad.arg1_type, quad.arg1_nesting, quad.nesting, quad.arg1_kind, quad.arg1_offset);
            fprintf(fp, "\tmov\tsi,[bp+6]\n");
            fprintf(fp, "\tmov\t[si],ax\n");
        }
        else {
            load("al", quad.arg1, fp, quad.arg1_pm, quad.arg1_type, quad.arg1_nesting, quad.nesting, quad.arg1_kind, quad.arg1_offset);
            fprintf(fp, "\tmov\tsi,[bp+6]\n");
            fprintf(fp, "\tmov\t[si],al\n");
        }
    }
    else if (strcmp(quad.quad, "jump") == 0){
        temp_label = label(quad.dest);
        fprintf(fp, "\tjmp %s\n", temp_label);
    }
    else if (strcmp(quad.quad, "nil?") == 0){
        temp_label = label(quad.dest);
        load("ax", quad.arg2, fp, quad.arg2_pm, quad.arg2_type, quad.arg2_nesting, quad.nesting, quad.arg2_kind, quad.arg2_offset);
        fprintf(fp, "\tor ax,ax\n");
        fprintf(fp, "\tjnz %s\n", temp_label);
    }
    else if (strcmp(quad.quad, "par") == 0){
        if ((strcmp(quad.arg2, "VALUE") == 0) && (strcmp(quad.arg1_kind, "integer") == 0)){
            load("ax", quad.arg1, fp, quad.arg1_pm, quad.arg1_type, quad.arg1_nesting, quad.nesting, quad.arg1_kind, quad.arg1_offset);
            fprintf(fp, "\tpush ax\n");
			inception_function_table[call_counter] += 2;
        }
        else if (strcmp(quad.arg2, "VALUE") == 0){
            load("al", quad.arg1, fp, quad.arg1_pm, quad.arg1_type, quad.arg1_nesting, quad.nesting, quad.arg1_kind, quad.arg1_offset);
			fprintf(fp, "\tsub sp,1\n");
			fprintf(fp, "\tmov si,sp\n");
            fprintf(fp, "\tmov byte ptr [si],al\n");
			inception_function_table[call_counter] += 1;
        }
        else if ((strcmp(quad.arg2, "REFERENCE") == 0) || (strcmp(quad.arg2, "RET") == 0)){
            loadAddr("si", quad.arg1, fp, quad.arg1_pm, quad.arg1_type, quad.arg1_nesting, quad.nesting, quad.arg1_kind, quad.arg1_offset);
            fprintf(fp, "\tpush si\n");
			if(((strcmp(quad.arg1_kind, "boolean") == 0) || (strcmp(quad.arg1_kind, "char") == 0)) && (strcmp(quad.arg2, "RET") == 0))
				inception_function_table[call_counter] += 1;
			else
				inception_function_table[call_counter] += 2;
        }
    }
	else if (strcmp(quad.quad, "array") == 0){
		load("ax", quad.arg2, fp, quad.arg2_pm, quad.arg2_type, quad.arg2_nesting, quad.nesting, quad.arg2_kind, quad.arg2_offset);
		if (strcmp(quad.arg1_kind, "integer") == 0)
			fprintf(fp, "\tmov cx,2\n");
		else if (strcmp(quad.arg1_kind, "iarray") == 0)
			fprintf(fp, "\tmov cx,2\n");
		else if (strcmp(quad.arg1_kind, "list") == 0)
			fprintf(fp, "\tmov cx,2\n");
		else
			fprintf(fp, "\tmov cx,1\n");
		fprintf(fp, "\timul cx\n");
		loadAddr("cx", quad.arg1, fp, quad.arg1_pm, quad.arg1_type, quad.arg1_nesting, quad.nesting, quad.arg1_kind, quad.arg1_offset);
		fprintf(fp, "\tadd ax,cx\n");
		store("ax", quad.dest, fp, quad.dest_pm, quad.dest_type, quad.dest_nesting, quad.nesting, quad.dest_kind, quad.dest_offset);
	}
    else {
        fprintf(fp, "*** quad: %s not implemented yet***\n", quad.quad);
    }
}

void getAR(char * a, FILE * fp, char * nesting){
    int na, ncur, times, i;

    na = atoi(a);
    ncur = atoi(nesting);
    times = ncur - na - 1;
    fprintf(fp, "\tmov si, word ptr [bp+4]\n");
    for (i=0; i < times ; i++)
        fprintf(fp, "\tmov si, word ptr [bp+4]\n");
    /*printf("%d\n", times);
    fprintf(fp, "\tgetAR(%s)\n", a);*/
}

void load(char * a, char * b, FILE * fp, char * data_pm, char * data_type, char * data_nesting, char * nesting, char * data_kind, char * data_offset){

	if(isdigit(b[0]))
        fprintf(fp, "\tmov %s,%s\n", a, b);
    else if (strcmp(b, "true") == 0)
        fprintf(fp, "\tmov %s,1\n", a);
    else if ((strcmp(b, "false") == 0) || (strcmp(b, "nil") == 0 ))
        fprintf(fp, "\tmov %s,0\n", a);
    else if (b[0] == '\'')
        if (b[1] != '\\')
            fprintf(fp, "\tmov %s,%d\n", a, (int) b[1]);
        else {
            char buf;
            if (b[2] == 'n')
                buf = '\n';
            else if (b[2] == 't')
                buf = '\t';
            else if (b[2] == 'r')
                buf = '\r';
            else if (b[2] == '0')
                buf = '\0';
            else if (b[2] == '\\')
                buf = '\\';
            else if (b[2] == '\'')
                buf = '\'';
            else if (b[2] == '\"')
                buf = '\"';
            else {
                int i, j;
                if ((int) b[3] >= 97)
                    i = (int) b[3] - 87;
                else if ((int) b[3] >= 65)
                    i = (int) b[3] - 55;
                else
                    i = (int) b[3] - 48;
                if ((int) b[4] >= 97)
                    j = (int) b[4] - 87;
                else if ((int) b[4] >= 65)
                    j = (int) b[4] - 55;
                else
                    j = (int) b[4] - 48;
                buf = (char) (16*i+j);
            }
            fprintf(fp, "\tmov %s,%d\n", a, (int) buf);
        }
    else if (b[0] == '[') {
		memmove(b, b+1, strlen(b));
        b[strlen(b)-1] = 0;
		load("di", b, fp, data_pm, data_type, data_nesting, nesting, data_kind, data_offset);
        if(strcmp(data_kind, "integer") == 0 || strcmp(data_kind, "iarray") == 0 || strcmp(data_kind, "list") == 0)
                fprintf(fp, "\tmov %s, word ptr [di]\n", a);
       	else
                fprintf(fp, "\tmov %s, byte ptr [di]\n", a);
    } else if(atoi(data_nesting)==atoi(nesting)) {
        if((strcmp(data_type, "variable") == 0) || (strcmp(data_type, "parameter") == 0 && strcmp(data_pm, "value") == 0) ||
            (strcmp(data_type, "temporary") == 0))
            if(strcmp(data_kind, "integer") == 0 || strcmp(data_kind, "iarray") == 0 || strcmp(data_kind, "list") == 0)
                fprintf(fp, "\tmov %s, word ptr [bp + (%d)]\n", a, atoi(data_offset));
            else
                fprintf(fp, "\tmov %s, byte ptr [bp + (%d)]\n", a, atoi(data_offset));
        else if ((strcmp(data_type, "parameter") == 0 && strcmp(data_pm, "reference") == 0) ) {
            fprintf(fp, "\tmov si, word ptr [bp + (%d)]\n", atoi(data_offset));
            if(strcmp(data_kind, "integer") == 0 || strcmp(data_kind, "iarray") == 0 || strcmp(data_kind, "list") == 0)
                fprintf(fp, "\tmov %s, word ptr [si]\n", a);
            else
                fprintf(fp, "\tmov %s, byte ptr [si]\n", a);
        }
    } else
        if((strcmp(data_type, "variable") == 0) || (strcmp(data_type, "parameter") == 0 && strcmp(data_pm, "value") == 0) ||
            (strcmp(data_type, "temporary") == 0)){
            getAR(data_nesting, fp, nesting);
            if(strcmp(data_kind, "integer") == 0 || strcmp(data_kind, "iarray") == 0 || strcmp(data_kind, "list") == 0)
                fprintf(fp, "\tmov %s, word ptr [si + (%d)]\n", a, atoi(data_offset));
            else
                fprintf(fp, "\tmov %s, byte ptr [si + (%d)]\n", a, atoi(data_offset));
        } else if ((strcmp(data_type, "parameter") == 0 && strcmp(data_pm, "reference") == 0) ) {
            getAR(data_nesting, fp, nesting);
            fprintf(fp, "\tmov si, word ptr [si + (%d)]\n", atoi(data_offset));
            if(strcmp(data_kind, "integer") == 0 || strcmp(data_kind, "iarray") == 0 || strcmp(data_kind, "list") == 0)
                fprintf(fp, "\tmov %s, word ptr [si]\n", a);
            else
                fprintf(fp, "\tmov %s, byte ptr [si]\n", a);
        }
}

void store(char * a, char * b, FILE * fp, char * data_pm, char * data_type, char * data_nesting, char * nesting, char * data_kind, char * data_offset){
    if (b[0] == '[') {
        memmove(b, b+1, strlen(b));
        b[strlen(b)-1] = 0;
		load("di", b, fp, data_pm, data_type, data_nesting, nesting, data_kind, data_offset);
        if(strcmp(data_kind, "integer") == 0 || strcmp(data_kind, "iarray") == 0 || strcmp(data_kind, "list") == 0)
                fprintf(fp, "\tmov word ptr [di], %s\n", a);
       	else
                fprintf(fp, "\tmov byte ptr [di], %s\n", a);
    }
    else if(atoi(data_nesting)==atoi(nesting)) {
        if((strcmp(data_type, "parameter") == 0 && strcmp(data_pm, "value") == 0) || (strcmp(data_type, "temporary") == 0))
            if(strcmp(data_kind, "integer") == 0 || strcmp(data_kind, "iarray") == 0 || strcmp(data_kind, "list") == 0)
                fprintf(fp, "\tmov word ptr [bp + (%d)], %s\n", atoi(data_offset), a);
            else
                fprintf(fp, "\tmov byte ptr [bp + (%d)], %s\n", atoi(data_offset), a);
        else if ((strcmp(data_type, "parameter") == 0 && strcmp(data_pm, "reference") == 0) ) {
            fprintf(fp, "\tmov si, word ptr [bp + (%d)]\n", atoi(data_offset));
            if(strcmp(data_kind, "integer") == 0 || strcmp(data_kind, "iarray") == 0 || strcmp(data_kind, "list") == 0)
                fprintf(fp, "\tmov word ptr [si], %s\n", a);
            else
                fprintf(fp, "\tmov byte ptr [si], %s\n", a);
        }
    } else
        if((strcmp(data_type, "variable") == 0) || (strcmp(data_type, "parameter") == 0 && strcmp(data_pm, "value") == 0) ||
            (strcmp(data_type, "temporary") == 0)){
            getAR(data_nesting, fp, nesting);
            if(strcmp(data_kind, "integer") == 0 || strcmp(data_kind, "iarray") == 0 || strcmp(data_kind, "list") == 0)
                fprintf(fp, "\tmov word ptr [si + (%d)], %s\n", atoi(data_offset), a);
            else
                fprintf(fp, "\tmov byte ptr [si + (%d)], %s\n", atoi(data_offset), a);
        } else if ((strcmp(data_type, "parameter") == 0 && strcmp(data_pm, "reference") == 0) ) {
            getAR(data_nesting, fp, nesting);
            fprintf(fp, "\tmov si, word ptr [si + (%d)]\n", atoi(data_offset));
            if(strcmp(data_kind, "integer") == 0 || strcmp(data_kind, "iarray") == 0 || strcmp(data_kind, "list") == 0)
                fprintf(fp, "\tmov word ptr [si], %s\n", a);
            else
                fprintf(fp, "\tmov byte ptr [si], %s\n", a);
        }
}

void loadAddr(char * a, char * b, FILE * fp, char * data_pm, char * data_type, char * data_nesting, char * nesting, char * data_kind, char * data_offset){
    if (b[0] == '\"'){
        unique++;
        memmove(b, b+1, strlen(b));
        b[strlen(b)-1] = 0;
        fprintf(fp, "\tlea %s,byte ptr @str%d\n", a, unique);
        temp = (program_string_t *) new(sizeof(program_string_t));
        temp->node_str = strdup(b);
        temp->id = unique;
        temp->next = NULL;
		if (program_strings == NULL)
        	program_strings = temp;
		else
			program_strings_tail->next = temp;
		program_strings_tail = temp;
    }
    else if (b[0] == '[') {
        memmove(b, b+1, strlen(b));
        b[strlen(b)-1] = 0;
        load(a, b, fp, data_pm, data_type, data_nesting, nesting, data_kind, data_offset);
    }
    else if(atoi(data_nesting)==atoi(nesting)) {
        if((strcmp(data_type, "parameter") == 0 && strcmp(data_pm, "value") == 0) || (strcmp(data_type, "temporary") == 0))
            if(strcmp(data_kind, "integer") == 0 || strcmp(data_kind, "iarray") == 0 || strcmp(data_kind, "list") == 0)
                fprintf(fp, "\tlea %s, word ptr [bp + (%d)]\n", a, atoi(data_offset));
            else
                fprintf(fp, "\tlea %s, byte ptr [bp + (%d)]\n", a, atoi(data_offset));
        else if ((strcmp(data_type, "parameter") == 0 && strcmp(data_pm, "reference") == 0) ) {
            fprintf(fp, "\tmov %s, word ptr [bp + (%d)]\n", a, atoi(data_offset));
        }
    }
    else
        if((strcmp(data_type, "parameter") == 0 && strcmp(data_pm, "value") == 0) || (strcmp(data_type, "temporary") == 0)){
            getAR(data_nesting, fp, nesting);
            if(strcmp(data_kind, "integer") == 0 || strcmp(data_kind, "iarray") == 0 || strcmp(data_kind, "list") == 0)
                fprintf(fp, "\tlea %s, word ptr [si + (%d)]\n", a, atoi(data_offset));
            else
                fprintf(fp, "\tlea %s, byte ptr [si + (%d)]\n", a, atoi(data_offset));
        } else if ((strcmp(data_type, "parameter") == 0 && strcmp(data_pm, "reference") == 0) ) {
            getAR(data_nesting, fp, nesting);
            fprintf(fp, "\tmov %s, word ptr [si + (%d)]\n", a, atoi(data_offset));
        }
}

void updateAL(FILE * fp, char * a, char * nesting){
    /*fprintf(fp, "\tupdateAL()\n");*/
    int np, nx, times, i;

    nx = atoi(a);
    np = atoi(nesting);
    times = np - nx - 1;

    if (np<nx)
        fprintf(fp, "\tpush bp\n");
    else if (np==nx)
        fprintf(fp, "\tpush word ptr [bp+4]\n");
    else {
        fprintf(fp, "\tmov si, word ptr [bp+4]\n");
        for (i=0; i < times ; i++)
            fprintf(fp, "\tmov si, word ptr [si+4]\n");
        fprintf(fp, "\tpush word ptr [si+4]\n");
    }
}

char * endof(char * a){
    char msg[256];
    char * endof;

    sprintf(msg, "@%s_", a);
    endof = strdup(msg);
    return endof;
}

char * label(char * a){
    char msg[256];
    char * label;

    sprintf(msg, "@%s", a);
    label = strdup(msg);
    return label;
}

char * name(char * a){
    char msg[256];
    char * name;

    sprintf(msg, "_%s", a);
    name = strdup(msg);
    return name;
}

void print_consumed_quad(Interpreted_quad quad){
    printf("%d: [%s, %s, %s, %s]\n", quad.id, quad.quad, quad.arg1, quad.arg2, quad.dest);
}

void printstrings(FILE * fp){
    while (program_strings != NULL){
        /*fprintf(fp, "@str%d db '%s'\n\tdb 0\n", program_strings->id, program_strings->node_str);*/
        fprintf(fp, "@str%d ", program_strings->id);
        string_to_db(fp, program_strings->node_str);
        fprintf(fp, "\tdb 0\n");
        program_strings = program_strings->next;
    }
    fprintf(fp, "\n");
}

void string_to_db(FILE * fp, char * node_str){
    int i, flag, flag2;
	char * temp = malloc(3);
    i = 0;
    flag = 0;
    flag2 = 0;
    while (node_str[i] != '\0'){
        if (node_str[i] == '\\'){
            flag2 = 1;
            i++;
            if ( (node_str[i] == 'n') || (node_str[i] == 't') || (node_str[i] == 'r') || (node_str[i] == '0') || (node_str[i] == '\\') || (node_str[i] == '\'') || (node_str[i] == '\"') || (node_str[i] == 'x') ){
                if (flag == 1)
                    fprintf(fp, "'\n");
                if (node_str[i] == 'n')
                    fprintf(fp, "\tdb 10\n");
                else if (node_str[i] == 't')
                    fprintf(fp, "\tdb 9\n");
                else if (node_str[i] == 'r')
                    fprintf(fp, "\tdb 13\n");
                else if (node_str[i] == '0')
                    fprintf(fp, "\tdb 0\n");
                else if (node_str[i] == '\\')
                    fprintf(fp, "\tdb 92\n");
                else if (node_str[i] == '\'')
                    fprintf(fp, "\tdb 39\n");
                else if (node_str[i] == '\"')
                    fprintf(fp, "\tdb 34\n");
                else{
                    i++;
					sprintf(temp, "%c%c", node_str[i], node_str[i+1]);
					i++;
                    fprintf(fp, "\tdb '%c'\n", (char) strtol(temp,NULL,16));
                }
                flag = 0;
            }
        }
        else if (flag == 0){
			fprintf(fp, "\tdb '");
            flag = 1;
        }
        if (flag2 == 0){
            fprintf(fp, "%c", node_str[i]);
        }
        flag2 = 0;
        i++;
    }
    if (flag == 1)
        fprintf(fp, "'\n");
}

void printexterns(FILE * fp, int * externs){
    int i;
    fprintf(fp,"\textrn _register_call_table : proc\n");
    for (i=0;i<21;i++)
        if (externs[i] != 0)
            switch(i){
                case 0:
	                fprintf(fp,"\textrn _puti : proc\n");
                    /*fprintf(fp,"\textrn _puti_call_table : word\n");*/
                    break;
                case 1:
	                fprintf(fp,"\textrn _putb : proc\n");
                    /*fprintf(fp,"\textrn _putb_call_table : word\n");*/
                    break;
                case 2:
	                fprintf(fp,"\textrn _putc : proc\n");
                    /*fprintf(fp,"\textrn _putc_call_table : word\n");*/
                    break;
                case 3:
	                fprintf(fp,"\textrn _puts : proc\n");
                    /*fprintf(fp,"\textrn _puts_call_table : word\n");*/
                    break;
                case 4:
	                fprintf(fp,"\textrn _geti : proc\n");
                    /*fprintf(fp,"\textrn _geti_call_table : word\n");*/
                    break;
                case 5:
	                fprintf(fp,"\textrn _getb : proc\n");
                    /*fprintf(fp,"\textrn _getb_call_table : word\n");*/
                    break;
                case 6:
	                fprintf(fp,"\textrn _getc : proc\n");
                    /*fprintf(fp,"\textrn _getc_call_table : word\n");*/
                    break;
                case 7:
	                fprintf(fp,"\textrn _gets : proc\n");
                    /*fprintf(fp,"\textrn _gets_call_table : word\n");*/
                    break;
                case 8:
	                fprintf(fp,"\textrn _abs : proc\n");
                    /*fprintf(fp,"\textrn _abs_call_table : word\n");*/
                    break;
                case 9:
	                fprintf(fp,"\textrn _ord : proc\n");
                    /*fprintf(fp,"\textrn _ord_call_table : word\n");*/
                    break;
                case 10:
                    fprintf(fp,"\textrn _chr : proc\n");
                    /*fprintf(fp,"\textrn _chr_call_table : word\n");*/
                    break;
                case 11:
	                fprintf(fp,"\textrn _strlen : proc\n");
                    /*fprintf(fp,"\textrn _strlen_call_table : word\n");*/
                    break;
                case 12:
	            	fprintf(fp,"\textrn _strcmp : proc\n");
                    /*fprintf(fp,"\textrn _strcmp_call_table : word\n");*/
                    break;
                case 13:
	            	fprintf(fp,"\textrn _strcpy : proc\n");
                    /*fprintf(fp,"\textrn _strcpy_call_table : word\n");*/
                    break;
                case 14:
	            	fprintf(fp,"\textrn _strcat : proc\n");
                    /*fprintf(fp,"\textrn _strcat_call_table : word\n");*/
                    break;
                case 15:
                    fprintf(fp,"\textrn _consp : proc\n");
                    fprintf(fp,"\textrn _consp_call_table : word\n");
                    break;
                case 16:
                    fprintf(fp,"\textrn _consv : proc\n");
                    fprintf(fp,"\textrn _consv_call_table : word\n");
                    break;
                case 17:
                    fprintf(fp,"\textrn _head : proc\n");
                    /*fprintf(fp,"\textrn _head_call_table : word\n");*/
                    break;
                case 18:
                    fprintf(fp,"\textrn _newarrp : proc\n");
                   /* fprintf(fp,"\textrn _newarrp_call_table : word\n");*/
                    break;
                case 19:
                    fprintf(fp,"\textrn _newarrv : proc\n");
                    /*fprintf(fp,"\textrn _newarrv_call_table : word\n");*/
                    break;
                case 20:
                    fprintf(fp,"\textrn _tail : proc\n");
                    /*fprintf(fp,"\textrn _tail_call_table : word\n");*/
                    break;
            }
}

void printexterns2(FILE * fp, int * externs){
    int i;
    for (i=0;i<21;i++)
        if (externs[i] != 0)
            switch(i){
                /*case 0:
                    fprintf(fp,"\n\tmov ax, OFFSET _puti_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 1:
                    fprintf(fp,"\n\tmov ax, OFFSET _putb_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 2:
                    fprintf(fp,"\n\tmov ax, OFFSET _putc_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 3:
                    fprintf(fp,"\n\tmov ax, OFFSET _puts_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 4:
                    fprintf(fp,"\n\tmov ax, OFFSET _geti_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 5:
                    fprintf(fp,"\n\tmov ax, OFFSET _getb_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 6:
                    fprintf(fp,"\n\tmov ax, OFFSET _getc_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 7:
                    fprintf(fp,"\n\tmov ax, OFFSET _gets_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 8:
                    fprintf(fp,"\n\tmov ax, OFFSET _abs_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 9:
                    fprintf(fp,"\n\tmov ax, OFFSET _ord_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 10:
                    fprintf(fp,"\n\tmov ax, OFFSET _chr_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 11:
                    fprintf(fp,"\n\tmov ax, OFFSET _strlen_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 12:
                    fprintf(fp,"\n\tmov ax, OFFSET _strcmp_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 13:
                    fprintf(fp,"\n\tmov ax, OFFSET _strcpy_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 14:
                    fprintf(fp,"\n\tmov ax, OFFSET _strcat_call_table\n\tcall near ptr _register_call_table");
                    break;*/
                case 15:
                    fprintf(fp,"\n\tmov ax, OFFSET _consp_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 16:
                    fprintf(fp,"\n\tmov ax, OFFSET _consv_call_table\n\tcall near ptr _register_call_table");
                    break;
                /*case 17:
                    fprintf(fp,"\n\tmov ax, OFFSET _head_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 18:
                    fprintf(fp,"\n\tmov ax, OFFSET _newarrp_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 19:
                    fprintf(fp,"\n\tmov ax, OFFSET _newarrv_call_table\n\tcall near ptr _register_call_table");
                    break;
                case 20:
                    fprintf(fp,"\n\tmov ax, OFFSET _tail_call_table\n\tcall near ptr _register_call_table");
                    break;*/
            }
}

void print_call_table(FILE * fp, char * fun_name, int call_counter, int temp_var_offset, int * param_byte_table, int * inception_function_table){
    /* this needs fixing */
    int temp_number;
    /* this needs fixing */

    int i, j;

    /* this needs fixing */
    temp_number = 3;
    /* this needs fixing */

    fprintf(fp, "_%s_call_table:\n", fun_name);
    for(i = 1; i < call_counter; i++){
        fprintf(fp, "@call_%s_%d\tdw @%s_call_%d\n", fun_name, i, fun_name, i);
        if((call_counter - i) == 1)
            fprintf(fp, "\tdw 0\n");
        else
            fprintf(fp, "\tdw @call_%s_%d\n", fun_name, i+1);
        fprintf(fp, "\tdw 4 + %d + %d + %d + 4\n", param_byte_table[i], inception_function_table[i], temp_var_offset);
        fprintf(fp, "IF LIVENESS eq 0\n");
        /* this needs fixing */
        for(j =0; j < temp_number; j++){
            fprintf(fp, "\tdw 10\n");
        }
        /* this needs fixing */
        fprintf(fp, "ENDIF\n");
        if((call_counter - i) > 1)
            fprintf(fp, "\tdw -10\n");
        fprintf(fp, "\tdw 0\n");
    }
}
