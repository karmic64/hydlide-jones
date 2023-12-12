#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>


#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

#define set_min(a,b) a = min(a,b);
#define set_max(a,b) a = max(a,b);


/****************************** ft module ******************************/

#define FT_MAX_CHANNELS 5
#define FT_MAX_ORDERS 0x100
#define FT_MAX_ROWS 0x100
#define FT_MAX_EFFECTS 4
#define FT_MAX_SONGS 64

const char ft_note_names[12][2] = {"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"};

typedef struct {
	char effect;
	uint8_t param;
} ft_effect_t;

typedef struct {
	uint8_t note;
	ft_effect_t effects[FT_MAX_EFFECTS];
} ft_row_t;

typedef ft_row_t ft_pattern_t[FT_MAX_CHANNELS][FT_MAX_ROWS];

typedef struct {
	uint8_t speed;
	uint8_t tempo;
	
	uint8_t orders;
	uint8_t orderlist[FT_MAX_CHANNELS][FT_MAX_ORDERS];
	
	uint8_t rows;
	ft_pattern_t pattern_data[FT_MAX_ORDERS];
} ft_song_t;

typedef struct {
	uint8_t songs;
	ft_song_t song_data[FT_MAX_SONGS];
} ft_module_t;


enum {
	CMD_NASHI = 0,
	CMD_TRACK,
	CMD_ORDER,
	CMD_PATTERN,
	CMD_ROW,
};


ft_module_t * read_ft_text_export(const char * filename) {
	FILE * f = fopen(filename, "r");
	if (!f) {
		printf("can't open %s: %s\n", filename,strerror(errno));
		return NULL;
	}
	
	ft_module_t * m = malloc(sizeof(ft_module_t));
	memset(m, 0, sizeof(*m));
	
	char line[0x100];
	unsigned lineno = 0;
	int song_id = -1;
	int pattern_id = -1;
	
	const char * whitespace = " \n\r\t\v\f";
	while (1) {
		int cmd = CMD_NASHI;
		
		// read line
		char * fgets_result = fgets(line, 0x100, f);
		
		if (fgets_result) {
			// remove newline
			size_t linelen = strlen(line);
			if (line[linelen-1] == '\n') {
				line[linelen-1] = '\0';
				linelen--;
			}
			lineno++;
			
			// kill comment
			if (line[0] == '#') {
				cmd = CMD_NASHI;
			} else {
				// get command
				char * cmd_tok = strtok(line, whitespace);
				if (!cmd_tok) {
					cmd = CMD_NASHI;
				} else if (!strcmp(cmd_tok, "TRACK")) {
					cmd = CMD_TRACK;
				} else if (!strcmp(cmd_tok, "ORDER")) {
					cmd = CMD_ORDER;
				} else if (!strcmp(cmd_tok, "PATTERN")) {
					cmd = CMD_PATTERN;
				} else if (!strcmp(cmd_tok, "ROW")) {
					cmd = CMD_ROW;
				} else {
					cmd = CMD_NASHI;
				}
			}
		}
		
		// flush data if needed
		
		
		// act on command
		switch (cmd) {
			case CMD_TRACK: {
				// new song
				song_id = m->songs++;
				pattern_id = -1;
				
				char * rows_tok = strtok(NULL, whitespace);
				m->song_data[song_id].rows = strtoul(rows_tok, NULL, 10);
				char * speed_tok = strtok(NULL, whitespace);
				m->song_data[song_id].speed = strtoul(speed_tok, NULL, 10);
				char * tempo_tok = strtok(NULL, whitespace);
				m->song_data[song_id].tempo = strtoul(tempo_tok, NULL, 10);
				break;
			}
			case CMD_ORDER: {
				// orderlist row
				if (song_id >= 0) {
					char * index_tok = strtok(NULL, whitespace);
					unsigned orderlist_index = strtoul(index_tok, NULL, 16);
					m->song_data[song_id].orders = max(m->song_data[song_id].orders, orderlist_index+1);
					strtok(NULL, whitespace);
					for (unsigned i = 0; i < FT_MAX_CHANNELS; i++) {
						char * order_tok = strtok(NULL, whitespace);
						m->song_data[song_id].orderlist[i][orderlist_index] = strtoul(order_tok, NULL, 16);
					}
				}
				break;
			}
			case CMD_PATTERN: {
				// new pattern
				if (song_id >= 0) {
					char * id_tok = strtok(NULL, whitespace);
					pattern_id = strtol(id_tok, NULL, 16);
				}
				break;
			}
			case CMD_ROW: {
				// pattern row
				if (song_id >= 0 && pattern_id >= 0) {
					char * row_tok = strtok(NULL, whitespace);
					unsigned pattern_row = strtoul(row_tok, NULL, 16);
					
					int channel = -1;
					int column = -1;
					while (1) {
						char * tok = strtok(NULL, whitespace);
						if (!tok) {
							break;
						}
						
						if (!strcmp(tok, ":")) {
							channel++;
							column = 0;
						} else if (channel >= 0 && column >= 0) {
							switch (column) {
								case 0: {
									// note/octave
									if (!strcmp(tok, "...")) {
										m->song_data[song_id].pattern_data[pattern_id][channel][pattern_row].note = 0xff;
									} else if (!strcmp(tok, "---")) {
										m->song_data[song_id].pattern_data[pattern_id][channel][pattern_row].note = 0x00;
									} else {
										int n;
										for (n = 0; n < 12; n++) {
											if (!memcmp(tok, ft_note_names[n], 2)) {
												break;
											}
										}
										unsigned octave = strtoul(tok+2, NULL, 16);
										m->song_data[song_id].pattern_data[pattern_id][channel][pattern_row].note = n + (octave * 12);
									}
									break;
								}
								case 1: {
									// instrument
									// (nothing)
									break;
								}
								case 2: {
									// volume
									// (nothing)
									break;
								}
								default: {
									// effect
									int effect_column = column - 3;
									m->song_data[song_id].pattern_data[pattern_id][channel][pattern_row].effects[effect_column].effect = tok[0];
									m->song_data[song_id].pattern_data[pattern_id][channel][pattern_row].effects[effect_column].param = strtoul(tok+1, NULL, 16);
									break;
								}
							}
							column++;
						}
					}
				}
				break;
			}
		}
		
		
		
		
		if (!fgets_result) {
			break;
		}
	}
	
	
	fclose(f);
	return m;
}

void free_ft_module(ft_module_t * m) {
	free(m);
}








/********************************* main ********************************/

int main(int argc, char * argv[]) {
	if (argc != 3) {
		puts("usage: export-tracks infile outfile");
		return EXIT_FAILURE;
	}
	
	ft_module_t * m = read_ft_text_export(argv[1]);
	if (!m) {
		return EXIT_FAILURE;
	}
	
	
	FILE * f = fopen(argv[2],"wb");
	if (!f) {
		printf("can't open %s: %s\n", argv[2],strerror(errno));
		free_ft_module(m);
		return EXIT_FAILURE;
	}
	
	for (unsigned song = 0; song < m->songs; song++) {
		ft_song_t * s = &m->song_data[song];
		
		const unsigned channel_map[] = {1,2};
		
		for (unsigned chni = 0; chni < 2; chni++) {
			fprintf(f,"song_data_%u_%u .byte ", song, chni);
			
			const unsigned chn = channel_map[chni];
			
			unsigned order = 0;
			unsigned row = 0;
			
			int note = -1;
			int dur = -1;
			int prv_dur = -1;
			
			while (1) {
				int is_over = order >= s->orders;
				
				int this_note = -1;
				if (!is_over) {
					ft_row_t * r = &s->pattern_data[s->orderlist[chn][order]][chn][row];
					if (r->note != 0xff) {
						this_note = r->note;
					}
					
					int has_dxx = 0;
					for (unsigned c = 0; c < FT_MAX_CHANNELS; c++) {
						for (unsigned i = 0; i < FT_MAX_EFFECTS; i++) {
							char effect = s->pattern_data[s->orderlist[c][order]][c][row].effects[i].effect;
							if (effect == 'D' || effect == 'B') {
								has_dxx = 1;
								goto end_dxx_check;
							}
						}
					}
end_dxx_check:;
					
					if (has_dxx || ++row >= s->rows) {
						order++;
						row = 0;
					}
				}
				
				if (is_over || this_note >= 0) {
					if (note >= 0) {
						int new_dur = 1;//dur != prv_dur;
						unsigned out_note = note;
						if (out_note) {
							out_note -= (12 * 2) - 1;
						}
						fprintf(f, "$%02X,", out_note | (new_dur ? 0 : 0x80));
						if (new_dur)
							fprintf(f, "$%02X,", dur);
					}
					note = this_note;
					prv_dur = dur;
					dur = 0;
				}
				dur++;
				
				if (is_over) {
					break;
				}
			}
			
			fseek(f, -1, SEEK_CUR);
			fputc('\n', f);
		}
		
		
		
	}
	
	
	fclose(f);
	
	
	
	free_ft_module(m);
}



