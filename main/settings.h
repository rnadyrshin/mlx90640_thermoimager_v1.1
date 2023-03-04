#ifndef SETTINGS_H_
#define SETTINGS_H_


// Разрешение экрана
#define dispWidth 				320
#define dispHeight				240
// Разрешение исходной термограммы
#define termWidth				32
#define termHeight				24

#define MIN_TEMP				-40
#define MAX_TEMP				300

#define MIN_TEMPSCALE_DELTA		10		// Минимальная дельта на шкале
#define SCALE_DEFAULT_MIN		10		// Минимальная температура шкалы по умолчанию
#define SCALE_DEFAULT_MAX		50		// Максимальная температура шкалы по умолчанию


#define BRIGHTNESS_STEP		5
#define BRIGHTNESS_MIN		5
#define BRIGHTNESS_MAX		100

#define EMISSIVITY_STEP		0.01
#define EMISSIVITY_MIN		0.01
#define EMISSIVITY_MAX		1

#define COLORSCALE_NUM		5


typedef enum
{
	int8,
	uint8,
	int16,
	uint16,
	int32,
	uint32,
	int64,
	uint64,
	str,
	float32
} eType;

typedef enum
{
	Emissivity_Plus = 0,
	Emissivity_Minus = 1,
	Scale_Next = 2,
	Scale_Prev = 3,
	Markers_OnOff = 4,
	Save_BMP16 = 5,
	Save_CSV = 6,
	Brightness_Plus = 7,
	Brightness_Minus = 8
} eButtonFunc;

typedef enum
{
	ByNearest = 0,
	LineInterpol = 1,
	HQ3X_2X = 2
} eScaleMode;

typedef enum
{
	Iron = 0,
	Rainbow = 1,
	Rainbow2 = 2,
	BlueRed = 3,
	BlackNWhite = 4
} eColorScale;


extern eButtonFunc FuncUp;
extern eButtonFunc FuncDown;
extern eColorScale ColorScale;
extern eScaleMode ScaleMode;
extern float Emissivity;		// Отражательная способность поверхности
extern int LcdBrightness, LcdBrightnessOld;
extern uint8_t TempMarkers;
extern uint8_t AutoScaleMode;					// Режим автоподстройки шкалы
extern float Emissivity;	// Отражательная способность поверхности
extern const float FPS_rates[];
extern uint8_t FPS_Idx, FPS_Idx_Old;

int settings_storage_init(void);
int settings_read_all(void);
int settings_write_all(void);

int setting_read(char *pKey, eType type, void *pValue);
int setting_write(char *pKey, eType type, void *pValue);
int32_t settings_commit(void);

#endif /* SETTINGS_H_ */
