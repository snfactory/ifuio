/* -------------------------------------------------------------------
.COPYRIGHT    (c)  1995 CRAL
.IDENT        decode.c
.LANGUAGE     C
.AUTHOR       RB
.PURPOSE      decode arguments dans une chaine
.COMMENT
.VERSION      1.0  26-06-95 RB : creation
.             1.1  21-12-95 RB : enleve get_adxycol pour table.c
.                                ajoute decode_argval_float
.             1.2  29-06-06 YC : decode_argval_char can handle long names   
---------------------------------------------------------------------*/

#include <stdlib.h>
#include <gendef.h>
#include <items.h>

double trunc_double(double var, int k)
{
	double frac, ipart, e, res;
	
	e = pow(10.,k);
	frac = modf(var, &ipart)*e;
	res = ipart + NINT(frac)/e;
	return(res);
}

int decode_argval_char(char *arg, char **val) 
{
  int i, k, n;
  char *car;
  
  car = strdup(arg);
  
  for (i=k=n=0; i<strlen(arg); i++) {
    if (arg[i] == ',') {
      car[i] = '\0';
      sscanf(car+k, "%s", val[n++]);
      k = i+1;
    }
  }
  sscanf(car+k, "%s", val[n++]);
  
  free(car);

  return(n);  
}

int decode_argval_double(char *arg, double *val, char *sep) 
{
	int i, k, n, j;
	char car[81];
	char code[5];

	strcpy(code, ",/:");

	for (i=0, k=0, n=0; i<strlen(arg); i++, k++) {
		for (j=0; j<3; j++) {
			if (arg[i] == code[j]) {
				sep[n] = code[j];
				car[k] = '\0';
				sscanf(car, "%lf", &(val[n]));
				n++;
				k = -1;
				break;
			}
		}
		if (k >= 0)
			car[k] = arg[i];
	}
	car[k] = '\0';
	sscanf(car, "%lf", &(val[n]));
	n++;

	return(n);
}

int decode_argval_int(char *arg, int *val, char *sep) 
{
	int i, k, n, j;
	char car[81];
	char code[5];

	strcpy(code, ",/:");

	for (i=0, k=0, n=0; i<strlen(arg); i++, k++) {
		for (j=0; j<3; j++) {
			if (arg[i] == code[j]) {
				sep[n] = code[j];
				car[k] = '\0';
				sscanf(car, "%d", &(val[n]));
				n++;
				k = -1;
				break;
			}
		}
		if (k >= 0)
			car[k] = arg[i];
	}
	car[k] = '\0';
	sscanf(car, "%d", &(val[n]));
	n++;

	return(n);
}

int decode_argval_float(char *arg, float *val, char *sep) 
{
	int i, k, n, j;
	char car[81];
	char code[5];

	strcpy(code, ",/:");

	for (i=0, k=0, n=0; i<strlen(arg); i++, k++) {
		for (j=0; j<3; j++) {
			if (arg[i] == code[j]) {
				sep[n] = code[j];
				car[k] = '\0';
				sscanf(car, "%f", &(val[n]));
				n++;
				k = -1;
				break;
			}
		}
		if (k >= 0)
			car[k] = arg[i];
	}
	car[k] = '\0';
	sscanf(car, "%f", &(val[n]));
	n++;

	return(n);
}

int decode_open_tablexy(char *arg, TABLE *tab, int *colx, int *coly)
{
	char **car;
	car = (char **)malloc(sizeof(char *));
	car[0] = (char *)malloc(80*sizeof(char));
	car[1] = (char *)malloc(80*sizeof(char));
	car[2] = (char *)malloc(80*sizeof(char));

	if (decode_argval_char(arg, car) != 3) {
		printf("\nERROR cannot decode table,xcol,ycol in  %s\n",
			arg);
		exit_session(1);
	}
	open_table(tab, car[0], "I");
	*colx = get_col_ref(tab, car[1]);
	*coly = get_col_ref(tab, car[2]);
	return(0);
}

int find_selected_arg(char **argval, char **argname, int *list, int n)
{
	int i, nsel, sel;

	for (i=0, nsel=0; i<n; i++) {
		if (strcmp(argval[list[i]], "null") == 0)
			continue;
		sel = list[i];
		nsel++;
	}
	if (nsel > 1) {
		printf("Fatal Error in line parameters\n");
		printf("You can not set more than one of the following parameters:\n");
		for (i=0; i<n; i++) 
			printf("\t%s\n", argname[list[i]]);
		exit_session(1);
	} else if (nsel == 0) {
		printf("Fatal Error in line parameters\n");
		printf("You must set one of the following parameters:\n");
		for (i=0; i<n; i++) 
			printf("\t%s\n", argname[list[i]]);
		exit_session(1);
	}
	return(sel);
}
