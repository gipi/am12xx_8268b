
#ifndef _ASS_PARSER_H_

#define _ASS_PARSER_H_

#if 0
typedef unsigned long uint32_t;

typedef struct ass_style_s {
	char* Name;
	char* FontName;
	double FontSize;
	uint32_t PrimaryColour;
	uint32_t SecondaryColour;
	uint32_t OutlineColour;
	uint32_t BackColour;
	int Bold;
	int Italic;
	int Underline;
	int StrikeOut;
	double ScaleX;
	double ScaleY;
	double Spacing;
	int Angle;
	int BorderStyle;
	double Outline;
	double Shadow;
	int Alignment;
	int MarginL;
	int MarginR;
	int MarginV;
	//        int AlphaLevel;
	int Encoding;
} ass_style_t;


/// ass track represent either an external script or a matroska subtitle stream (no real difference between them)
/// it can be used in rendering after the headers are parsed (i.e. events format line read)
typedef struct ass_track_s {
	int n_styles; // amount used
	int max_styles; // amount allocated
	int n_events;
	int max_events;
	ass_style_t* styles; // array of styles, max_styles length, n_styles used
	ass_event_t* events; // the same as styles
	
	char* style_format; // style format line (everything after "Format: ")
	char* event_format; // event format line
	
	enum {TRACK_TYPE_UNKNOWN = 0, TRACK_TYPE_ASS, TRACK_TYPE_SSA} track_type;
	
	// script header fields
	int PlayResX;
	int PlayResY;
	double Timer;
	int WrapStyle;
	
	
	int default_style; // index of default style
	char* name; // file name in case of external subs, 0 for streams
	
	ass_library_t* library;
	parser_priv_t* parser_priv;
} ass_track_t;

#endif

#endif
