// wad types
typedef struct {
	unsigned char magic[4];      // "WAD2", Name of the new WAD format
	int numentries;              // Number of entries
	int diroffset;               // Position of WAD directory in file
} wad_header_t;


typedef struct {
	int offset;                  // Position of the entry in WAD
	int disk;                    // Size of the entry in WAD file
	int size;                    // Size of the entry in memory
	char type;                   // type of entry
	char compression;            // Compression. 0 if none.
	char pad1;                   // Not used
	char pad2;
	char name[16];               // 1 to 16 characters, '\0'-padded
} wad_entry_t;


typedef struct {
	char name[16];               // Name of the texture.
	int width;                   // width of picture, must be a multiple of 8
	int height;                  // height of picture, must be a multiple of 8
	int offset1;                 // offset to u_char Pix[width   * height]
	int offset2;                 // offset to u_char Pix[width/2 * height/2]
	int offset4;                 // offset to u_char Pix[width/4 * height/4]
	int offset8;                 // offset to u_char Pix[width/8 * height/8]
} miptex_t;
