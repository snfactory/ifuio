/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
.COPYRIGHT    (c)  2000 CRAL
.IDENT        template.c
.LANGUAGE     C
.AUTHOR       AP
.PURPOSE      How to use I/O functions for datacube
.COMMENT
.VERSION      1.0  25-01-00 AP: creation
---------------------------------------------------------------------*/

#include <IFU_io.h>

int main(int argc, char **argv)
{
  TABLE intab, outtab;
  TIGERfile intig, outtig;

  char	**argval, **argname;
  char	text[1024];
  float	x, y;

  set_arglist("-in none -out null");
  init_session(argv, argc, &argname, &argval);

  if (open_tiger_frame(&intig,argval[0],"I") < 0) {
      sprintf(text, "Unable to open input datacube '%s'", argval[0]);
      print_error(text);
      exit_session(ERR_OPEN);
    }
  }

  if (create_tiger_frame(&outtig,argval[1],-1,-1,intig.step,
      intig.data_type,intig.table_name,intig.ident,intig.cunit)) < 0) {

      sprintf(text, "Unable to create output datacube '%s'", argval[1]);
      print_error(text);
      exit_session(ERR_CREAT);
  }

   ......

  print_progress("Truncating data cube", -1, 0);

  for (n=0; n<intig.nbspec; n++) {

    print_progress("Truncating data cube", 100*(n+1)/intig.nbspec, 1.0);

    no = intig.signal[n].nolens;
    get_tiger_spec(&intig, &inspec, NULL, no);
    init_new_tiger_spec(&outtig, &outspec, nbwave, lbda_min);

    i1 = pixel_spec(&inspec, lbda_min);
    i2 = pixel_spec(&inspec, lbda_max);
    for (i=i1, k=0; k<nbwave; k++, i++)
      WR_spec(&outspec, k, RD_spec(&inspec, i));

    put_tiger_spec(&outtig, &outspec, NULL, no);
    free_spec_mem(&inspec);
  }

  CP_non_std_desc(&intig, &outtig);
  close_tiger_frame(&outtig);

  exit_session(0);
}
