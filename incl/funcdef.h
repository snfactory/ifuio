/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  1992 Observatoire de Lyon - St Genis-Laval (France)
! IDENT        funcdef.h
! LANGUAGE     C
! AUTHOR       A.Rousset
! KEYWORDS     
! PURPOSE      Functions definition for usual i/o
! COMMENT      
! VERSION      4.0  1992-June-12 : Creation,   AR 
 ---------------------------------------------------------------------*/

				/* SPECTRUM */

#define RD_spec(spectre,i)	\
	(((spectre)->data_type) == SHORT ? (spectre)->data.s_data[i] : \
	(((spectre)->data_type) == LONG ? (spectre)->data.l_data[i] : \
	(((spectre)->data_type) == INT ? (spectre)->data.l_data[i] : \
	(((spectre)->data_type) == FLOAT ? (spectre)->data.f_data[i] : \
	(((spectre)->data_type) == DOUBLE ? (spectre)->data.d_data[i] : MAXSHORT)))))

#define WR_spec(spectre,i,val)	\
	switch ((spectre)->data_type) {\
		case SHORT  : (spectre)->data.s_data[i] = (short)(val); break; \
		case LONG   : (spectre)->data.l_data[i] = (long)(val); break; \
		case INT   : (spectre)->data.l_data[i] = (long)(val); break; \
		case FLOAT  : (spectre)->data.f_data[i] = (float)(val); break; \
		case DOUBLE : (spectre)->data.d_data[i] = (double)(val); break; \
	}

#define in_spectrum(spectre,x) (((x)<((spectre)->start-(spectre)->step/2)) || ((x)>((spectre)->end+(spectre)->step/2))) ? 0:1
#define coord_spec(spectre,i) ((spectre)->start + (i)*(spectre)->step)
#define pixel_spec(spectre,x) (0.5 + ((x) - (spectre)->start)/(spectre)->step)

				/* SLICE */

#define RD_slice(slice,i)	\
	(((slice)->data_type) == SHORT ? (slice)->data.s_data[i] : \
	(((slice)->data_type) == LONG ? (slice)->data.l_data[i] : \
	(((slice)->data_type) == INT ? (slice)->data.l_data[i] : \
	(((slice)->data_type) == FLOAT ? (slice)->data.f_data[i] : \
	(((slice)->data_type) == DOUBLE ? (slice)->data.d_data[i] : MAXSHORT)))))

#define WR_slice(slice,i,val)	\
	switch ((slice)->data_type) {\
		case SHORT  : (slice)->data.s_data[i] = (short)(val); break; \
		case LONG   : (slice)->data.l_data[i] = (long)(val); break; \
		case INT    : (slice)->data.l_data[i] = (long)(val); break; \
		case FLOAT  : (slice)->data.f_data[i] = (float)(val); break; \
		case DOUBLE : (slice)->data.d_data[i] = (double)(val); break; \
	}

#define RD_qslice(slice,i) (slice)->quality[i]
#define WR_qslice(slice,i,val)	(slice)->quality[i] = (long)(val)

				/* FRAMES */
				
#define RD_frame(frame,i,j)	\
   (((frame)->data_type) == SHORT ? (frame)->data.s_data[(i)+(j)*(frame)->nx] : \
   (((frame)->data_type) == USHORT ? (frame)->data.us_data[(i)+(j)*(frame)->nx] : \
   (((frame)->data_type) == LONG ? (frame)->data.l_data[(i)+(j)*(frame)->nx] : \
   (((frame)->data_type) == INT ? (frame)->data.l_data[(i)+(j)*(frame)->nx] : \
   (((frame)->data_type) == FLOAT ? (frame)->data.f_data[(i)+(j)*(frame)->nx]: \
   (((frame)->data_type) == DOUBLE ? (frame)->data.d_data[(i)+(j)*(frame)->nx]: MAXSHORT))))))

#define WR_frame(frame,i,j,val)	\
	switch ((frame)->data_type) {\
		case SHORT  : (frame)->data.s_data[(i)+(j)*(frame)->nx]=(short)(val); \
						break; \
		case USHORT : (frame)->data.us_data[(i)+(j)*(frame)->nx]=(unsigned short)(val); \
						break; \
		case LONG   : (frame)->data.l_data[(i)+(j)*(frame)->nx]=(long)(val); \
						break; \
		case INT   : (frame)->data.l_data[(i)+(j)*(frame)->nx]=(long)(val); \
						break; \
		case FLOAT  : (frame)->data.f_data[(i)+(j)*(frame)->nx]=(float)(val); \
						break; \
		case DOUBLE : (frame)->data.d_data[(i)+(j)*(frame)->nx]=(double)(val);\
						break; \
 }

#define in_frame(frame,x,y)	(((x)<(frame)->startx) || ((x)>(frame)->endx) || ((y)<(frame)->starty) || ((y)>(frame)->endy)) ? 0:1

				/* CUBES */
				
#define RD_cube(cube,i,j,k)	\
 (((cube)->data_type) == SHORT ? (cube)->data.s_data[(i)\
		+(j)*(cube)->nx+(k)*(cube)->nx*(cube)->ny]: \
 (((cube)->data_type) == INT ? (cube)->data.l_data[(i)\
		+(j)*(cube)->nx+(k)*(cube)->nx*(cube)->ny] : \
 (((cube)->data_type) == LONG ? (cube)->data.l_data[(i)\
		+(j)*(cube)->nx+(k)*(cube)->nx*(cube)->ny] : \
 (((cube)->data_type) == FLOAT ? (cube)->data.f_data[(i)\
		+(j)*(cube)->nx+(k)*(cube)->nx*(cube)->ny]: \
 (((cube)->data_type) == DOUBLE ? (cube)->data.d_data[(i)\
		+(j)*(cube)->nx+(k)*(cube)->nx*(cube)->ny]: MAXSHORT)))))

#define WR_cube(cube,i,j,k,val)	\
	switch ((cube)->data_type) {\
		case SHORT  : (cube)->data.s_data[(i) \
			+(j)*(cube)->nx +(k)*(cube)->nx*(cube)->ny]=(short)(val); \
						break; \
		case LONG   : (cube)->data.l_data[(i) \
			+(j)*(cube)->nx +(k)*(cube)->nx*(cube)->ny]=(long)(val); \
						break; \
		case INT   : (cube)->data.l_data[(i) \
			+(j)*(cube)->nx +(k)*(cube)->nx*(cube)->ny]=(long)(val); \
						break; \
		case FLOAT  : (cube)->data.f_data[(i) \
			+(j)*(cube)->nx +(k)*(cube)->nx*(cube)->ny]=(float)(val); \
						break; \
		case DOUBLE : (cube)->data.d_data[(i) \
			+(j)*(cube)->nx +(k)*(cube)->nx*(cube)->ny]=(double)(val);\
						break; \
 }

#define in_cube(cube,x,y,z)	(((x)<(cube)->startx) || ((x)>(cube)->endx) \
 || ((y)<(cube)->starty) || ((y)>(cube)->endy) \
 || ((z)<(cube)->startz) || ((z)>(cube)->endz)) ? 0:1

