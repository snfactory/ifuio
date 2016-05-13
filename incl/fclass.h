/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! COPYRIGHT    (c)  2004 Observatoire de Lyon - St Genis Laval (FRANCE)
! IDENT        fclass.h
! LANGUAGE     C
!
! AUTHOR       A. Pecontal
!
! KEYWORDS     
! PURPOSE      File classes definition (previously contained in gendef.h)
! COMMENT      
! VERSION      1.0  2004-May-28 : Creation,   AP 
!              1.1  2004-Jun-01 : Add new fclasses related to dome flat
!
---------------------------------------------------------------------*/

	/* content of the frame */

#define DONT_KNOW	0	/* unknown */
#define RAW_MPUP 	1	/* raw micro-pupils */
#define PRE_MPUP 	2	/* preprocessed micro-pupils */
#define RAW_CAL_FRAME 	3	/* raw calibration frame */
#define PRE_CAL_FRAME	4	/* preprocessed calibration frame */
#define RAW_CAL_CUBE	5	/* raw calibration datacube */
#define WAV_CAL_CUBE	6	/* wavelength calib calibration datacube */
#define	RAW_CON_FRAME	7	/* raw continuum frame */
#define	PRE_CON_FRAME	8	/* preprocessed continuum frame */
#define RAW_CON_CUBE    9	/* raw continuum datacube */
#define WAV_CON_CUBE	10	/* wavelength calib continuum datacube */
#define FLA_CON_CUBE	11	/* flat field continuum datacube */
#define FLAT_CUBE       30  	/* Flat cube */
#define RAW_SKY_FRAME	12	/* raw sky frame */ 
#define PRE_SKY_FRAME	13	/* preprocessed sky frame */
#define RAW_SKY_CUBE	14	/* raw sky datacube */
#define WAV_SKY_CUBE	15	/* wavelength calib sky datacube */
#define FLA_SKY_CUBE	16	/* flat fielded sky datacube */
#define	RAW_OBJ_FRAME	17	/* raw object frame */
#define	PRE_OBJ_FRAME	18	/* preprocessed object frame */
#define	RAW_OBJ_CUBE	19	/* raw object datacube */
#define	WAV_OBJ_CUBE	20	/* wavelength calib object datacube */
#define	FLA_OBJ_CUBE	21	/* flat fielded object datacube */
#define	COS_OBJ_CUBE	22	/* cosmic removed object datacube */
#define	FLX_OBJ_CUBE	23	/* flux calib object datacube */
#define THR_SPEC        32  	/* Throughput Spectrum */
#define	RAW_BIAS  	24	/* Raw Bias frame */
#define	DARK_FRAME	25	/* Dark frame */
#define	TBL_FILE	26	/* Table */
#define TBL_FILE_CAL    29  	/* Table of wavelength calibrated cube */
#define TBL_FILE_REF    31  	/* Table of Reference Wavelength */
#define	MSK_FILE	27	/* Mask */
#define	MAX_FILE	28	/* Maxima */
#define REC_IMAGE       33  	/* Reconstructed Image */
/*      CATALOG         34         Frame Catalog (used by TCL/Tk) */
#define MER_OBJ_CUBE    35  	/* Merged Datacube */
#define DEBUG_FILE      36  	/* Debug File */
#define	BIAS_FRAME	37	/* Bias frame without overscan window */
#define	SKY_OBJ_CUBE	38	/* sky subtracted object datacube */
#define	LOSVD_RAW	39	/* Raw LOSVD */
#define	LOSVD_FIT	40	/* Fitted LOSVD */
#define	TBL_FLUX_REF	41	/* Table of Reference Flux */
#define	SKY_FILE_REF	42	/* Sky spectrum */
#define	SKY_FILE_VAR	43	/* Sky spectrum (variance) */
#define	COS_CON_CUBE	44	/* cosmic removed continuum datacube */
#define	COS_SKY_CUBE	45	/* cosmic removed sky datacube */
#define	HF_FLAT     	46	/* HF Flat-field */
#define FLT_CON_FRAME   47	/* HF flat-fielded continuum frame */
#define FLT_CAL_FRAME   48	/* HF flat-fielded calibration frame */
#define FLT_SKY_FRAME   49	/* HF flat-fielded sky frame */
#define FLT_OBJ_FRAME   50	/* HF flat-fielded object frame */
#define RAW_IMA_FRAME   51	/* raw image frame */
#define PRE_IMA_FRAME   53	/* preprocessed image frame */
#define RAW_DOM_FRAME   52	/* raw flat dome frame */
#define PRE_DOM_FRAME   54	/* preprocessed dome frame */
#define RAW_DOM_CUBE    55	/* raw dome datacube */
#define WAV_DOM_CUBE    56	/* wavelength calib. dome datacube */
#define FLA_DOM_CUBE    57	/* flat-fielded dome datacube */
#define COS_DOM_CUBE    58	/* cosmic removed dome datacube */
#define FLT_DOM_FRAME   59      /* HF flat-fielded dome frame */ 

#define	SUPER_CLASS	99	/* SuperClass */

