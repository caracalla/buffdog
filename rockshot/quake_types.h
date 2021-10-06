struct vec3_t {
	float x;
	float y;
	float z;

	void log() {
		printf("%f %f %f\n", this->x, this->y, this->z);
	}
};


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


// BSP types
typedef struct {
	int offset;                   // Offset to entry, in bytes, from start of file
	int size;                     // Size of entry in file, in bytes
} bsp_entry_t;


typedef struct {
	int version;                 // Model version, must be 0x17 (23).
	bsp_entry_t entities;        // List of Entities.
	bsp_entry_t planes;          // Map Planes.
															 // numplanes = size/sizeof(plane_t)
	bsp_entry_t miptex;          // Wall Textures.
	bsp_entry_t vertices;        // Map Vertices.
															 // numvertices = size/sizeof(vertex_t)
	bsp_entry_t visilist;        // Leaves Visibility lists.
	bsp_entry_t nodes;           // BSP Nodes.
															 // numnodes = size/sizeof(node_t)
	bsp_entry_t texinfo;         // Texture Info for faces.
															 // numtexinfo = size/sizeof(texinfo_t)
	bsp_entry_t faces;           // Faces of each surface.
															 // numfaces = size/sizeof(face_t)
	bsp_entry_t lightmaps;       // Wall Light Maps.
	bsp_entry_t clipnodes;       // clip nodes, for Models.
															 // numclips = size/sizeof(clipnode_t)
	bsp_entry_t leaves;          // BSP Leaves.
															 // numleaves = size/sizeof(leaf_t)
	bsp_entry_t face_list;       // List of indexes into faces
															 // only used by BSP tree leaf, not models)
	bsp_entry_t edges;           // Edges of faces.
															 // numedges = Size/sizeof(edge_t)
															 // first edge is never used (index 0 makes no sense, see below)
	bsp_entry_t edge_list;       // List of indexes into edges.
															 // positive index means edge should be walked from start to end vertex
															 // negative index means edge should be walked in reverse
	bsp_entry_t models;          // List of Models.
															 // nummodels = Size/sizeof(model_t)
} bsp_header_t;


// textures at bsp_header_t.miptex.offset start with the miptexheader_t, and
// are followed by each miptex_t i at miptexheader_t.dataofs[i]
typedef struct {
	int nummiptex;
	int dataofs[4]; // [nummiptex]
} miptexheader_t;


typedef struct {
	char name[16];               // Name of the texture.
	int width;                   // width of picture, must be a multiple of 8
	int height;                  // height of picture, must be a multiple of 8
	int offset1;                 // offset to u_char Pix[width   * height]
	int offset2;                 // offset to u_char Pix[width/2 * height/2]
	int offset4;                 // offset to u_char Pix[width/4 * height/4]
	int offset8;                 // offset to u_char Pix[width/8 * height/8]
} miptex_t;


// For a given face, the (s,t) coordinates are calculated from the Vertex
// coordinates and the Texture definitions by a simple dot product with the S
// and T vectors:
//   s = dotproduct(Vertex,vectorS) + distS;
//   t = dotproduct(Vertex,vectorT) + distT;
typedef struct{
	vec3_t vectorS;              // S vector, horizontal in texture space)
	float distS;                 // horizontal offset in texture space
	vec3_t vectorT;              // T vector, vertical in texture space
	float distT;                 // vertical offset in texture space
	int texture_id;              // Index of Mip Texture
															 //           must be in [0,numtex[
	int animated;                // 0 for ordinary textures, 1 for water
} texinfo_t;


// models
typedef struct {
	vec3_t min;
	vec3_t max;
} boundbox_t;

// #define	MAX_MAP_HULLS		4
// typedef struct
// {
// 	float		mins[3], maxs[3];
// 	float		origin[3];
// 	int			headnode[MAX_MAP_HULLS];
// 	int			visleafs;		// not including the solid leaf 0
// 	int			firstface, numfaces;
// } dmodel_t;

typedef struct {
	boundbox_t boundbox;
	vec3_t origin;
	int first_bsp_node;
	int first_clip_node;
	int second_clip_node;
	int empty_node;
	int bsp_leaf_count;
	int first_face;
	int face_count;
} model_t;


typedef struct {
	short plane_id;              // The plane in which the face lies
															 //   must be in [0,numplanes)
	short side;                  // 0 if in front of the plane, 1 if behind the plane
	int edge_list_id;                // first edge in the List of edges
															 //   must be in [0,numledges)
	short edge_count;         // number of edges in the List of edges
	short texinfo_id;            // index of the Texture info the face is part of
															 //   must be in [0,numtexinfos)
	unsigned char light_type;    // type of lighting, for the face
	unsigned char baselight;     // from 0xFF (dark) to 0 (bright)
	unsigned char light[2];      // two additional light models
	int lightmap;                // Pointer inside the general light map, or -1
															 // this define the start of the face light map
} face_t;

// Plane types:
// 0: Axial plane, in X
// 1: Axial plane, in Y
// 2: Axial plane, in Z
// 3: Non axial plane, roughly toward X
// 4: Non axial plane, roughly toward Y
// 5: Non axial plane, roughly toward Z
typedef struct {
	vec3_t normal;               // Vector orthogonal to plane (Nx,Ny,Nz)
															 // with Nx2+Ny2+Nz2 = 1
	float dist;                  // Offset to plane, along the normal vector.
															 // Distance from (0,0,0) to the plane
	int type;                    // Type of plane, depending on normal vector.
} plane_t;

typedef struct {
	short start_vertex;
	short end_vertex;
} edge_t;
