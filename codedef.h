#ifndef	__CODE__DEF_H___
#define	__CODE__DEF_H___

// site define for compile
#define	DEF_K_WATER

#define	DEF_DIS_CODE_RECV_ERR	"S04"


#ifndef  ORACLE_ID

#ifdef	DEF_K_WATER
//	#define  ORACLE_ID      "eqms/sysop2013!@INFOS"
	#define  ORACLE_ID      "eqms/eqms123@EDIS"
#endif	

#endif

#define		DEF_KMA		"KMA"
#define		DEF_NEMA	"NEMA"

#ifndef	DEF_HEVT_THREADHOLD
	#define DEF_HEVT_THREADHOLD		"HEVT_THREADHOLD"
#endif

#ifndef	DEF_OBS_SENSOR_TBL
	#define DEF_OBS_SENSOR_TBL		"OBS_SENSOR"
#endif


#ifndef	DEF_HEVTINFO_TBL
	#define DEF_HEVTINFO_TBL		"HEVTINFO"
#endif

#ifndef	DEF_HEVTINFO_RAW_TBL
	#define DEF_HEVTINFO_RAW_TBL		"HEVTINFO_RAW"
#endif

#ifndef	DEF_HEVTINFO_RAW2_TBL
	#define DEF_HEVTINFO_RAW2_TBL		"HEVTINFO_RAW2"
#endif


#ifndef	DEF_TEMP_FILE_PATH
	#define DEF_TEMP_FILE_PATH		"/opt/work/logs"
#endif

#ifndef	DEF_EventRPT_PATH
	#define DEF_EventRPT_PATH	"/data/RPT"
#endif

#ifndef	DEF_EventRealData_PATH
	#define DEF_EventRealData_PATH	"/data/RPT/REAL"
#endif

#ifndef	DEF_Anal_WORK_PATH
	#define DEF_Anal_WORK_PATH		"/opt/work/EventData"
#endif

#ifndef	DEF_WORK_BIN_PATH
	#define DEF_WORK_BIN_PATH		"/opt/work/bin"
#endif

#ifndef	DEF_analFileMaker_MSEED
	#define DEF_analFileMaker_MSEED	"analFileMaker"
#endif

#ifndef	DEF_analFileMaker_MMA
	#define DEF_analFileMaker_MMA	"analFileMakerMMA"
#endif

#ifndef	DEF_MSEED_WRITE_PATH
	#define DEF_MSEED_WRITE_PATH	"/data/KISStool/raw"
#endif

#ifndef	DEF_MMA_WRITE_PATH
	#define DEF_MMA_WRITE_PATH		"/data/KISStool/QSCD"
#endif

#ifndef	DEF_HEVT_Pre_RANGE
	#define	DEF_HEVT_Pre_RANGE		30
#endif

#ifndef	DEF_HEVT_Post_RANGE
	#define	DEF_HEVT_Post_RANGE		60
#endif

#ifndef	DEF_MseedEventFileMakerUTC
	#define	DEF_MseedEventFileMakerUTC	"mseedEventFileMakerUTC"
#endif

#ifndef	DEF_nmBlockMakeQscd
	#define	DEF_nmBlockMakeQscd	"nmBlockMakeQscd"
#endif

#ifndef	DEF_WAVE_EVENT_PATH			// 이벤트 구간 파일의 상위 경로 + YYYYMMDDHHMISS 디렉토리에 구간 파일 저장 
	#define	DEF_WAVE_EVENT_PATH		"/opt/work/wave_event"
#endif

#ifndef	DEF_KWAVE_EVENT_PATH		// 이벤트 구간 파일의 상위 경로 + YYYYMMDDHHMISS 디렉토리에 구간 파일 저장 
	#define	DEF_KWAVE_EVENT_PATH		"/opt/work/Knema_event"
#endif

#ifndef	DEF_SEND_EVENT_DATA_PATH	// 안전처에 보내야할 이벤트 압축 파일 경로
	#define	DEF_SEND_EVENT_DATA_PATH	"/opt/work/evt"
#endif


#ifndef	DEF_KEVT
	#define DEF_KEVT	"KEVT"
#endif

#ifndef	DEF_HEVT
	#define DEF_HEVT	"HEVT"
#endif

// output file extentions
#ifndef	DEF_MMA_ASCII
	#define	DEF_MMA_ASCII		"_MMA_ASCII.txt"
#endif

#ifndef	DEF_MSEED_COUNT
	#define	DEF_MSEED_COUNT		"_MCOUNT.txt"
#endif

#ifndef	DEF_MSEED_ASCII_100
	#define	DEF_MSEED_ASCII_100	"_ASCII100.txt"
#endif

#ifndef	DEF_MSEED_ASCII_20
	#define	DEF_MSEED_ASCII_20	"_ASCII20.txt"
#endif

#ifndef	DEF_MSEED_ASCII_001
	#define	DEF_MSEED_ASCII_001	"_ASCII001.txt"
#endif

#ifndef	DEF_MSEED_FFT
	#define	DEF_MSEED_FFT		"_FFT.txt"
#endif

#ifndef	DEF_MSEED_SPT
	#define	DEF_MSEED_SPT		"_SPT.txt"
#endif


// excute file name
#ifndef	DEF_MSEED2ASCII
	#define	DEF_MSEED2ASCII	"mseed2Gascii"
#endif

#define	DEF_ACC_Z		'Z'
#define	DEF_ACC_N		'N'
#define	DEF_ACC_Y		'Y'
#define	DEF_ACC_E		'E'
#define	DEF_ACC_X		'X'
#define	DEF_ACC_U		'U'		// unknown code

#define	DEF_H_SAMPLE	'H'
#define	DEF_B_SAMPLE	'B'

#define	DEF_D_KIND_C	'C'
#define	DEF_D_KIND_A	'A'
#define	DEF_D_KIND_V	'V'
#define	DEF_D_KIND_D	'D'

#define	DEF_ACC_Z_ON	0x01
#define	DEF_ACC_N_ON	0x02
#define	DEF_ACC_E_ON	0x04
#define	DEF_ACC_ZNE_ON	0x07


#endif	// __CODE__DEF_H___
