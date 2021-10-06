#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "model.h"
#include "quake_types.h"
#include "../util.h"



struct Color {
  unsigned char red;
  unsigned char green;
  unsigned char blue;

  void log() {
    printf("color: %d %d %d\n", this->red, this->green, this->blue);
  }
};


struct BSPTexture {
  char* name;
  int width;
  int height;
  unsigned char* color_data;
  Color* palette;

  void dumpToPPM() const {
    constexpr char const* k_ppm_location_format = "tex/%s.ppm";

    char output_file[64];
    int result = sprintf(output_file, k_ppm_location_format, this->name);

    if (result < 0) {
      throw std::runtime_error("failed to write output file name");
    }

    std::ofstream output;
    output.open(output_file);
    output << "P3\n" << this->width << " " << this->height << "\n255\n";

    for (int i = 0; i < this->width * this->height; i++) {
      Color& color = palette[this->color_data[i]];
      output << (int)color.red << " " << (int)color.green << " " << (int)color.blue << "\n";
    }

    output.close();
  }
};


struct BSP {
  model_t* models;
  int model_count;

  std::vector<BSPTexture> textures;

  texinfo_t* texinfos;
  int texinfo_count;

  plane_t* planes;
  int plane_count;

  face_t* faces;
  int face_count;

  edge_t* edges;
  int edge_count;

  int* edge_list;
  int edge_list_size;

  vec3_t* vertices;
  int vertex_count;

  BSP(std::vector<unsigned char>& raw_bsp, Color* palette) {
    // printf("bsp bytes: %lu\n", raw_bsp.size());
    bsp_header_t* header = (bsp_header_t*)raw_bsp.data();

    // models
    bsp_entry_t& models_entry = header->models;
    this->model_count = models_entry.size / sizeof(model_t);
    this->models = (model_t*)(&(raw_bsp.data()[models_entry.offset]));

    // textures
    bsp_entry_t& textures_entry = header->miptex;
    unsigned char* raw_textures = &(raw_bsp.data()[textures_entry.offset]);
    miptexheader_t* tex_header = (miptexheader_t*)(raw_textures);
    this->textures.resize(tex_header->nummiptex);

    for (int i = 0; i < tex_header->nummiptex; i++) {
      unsigned char* local_tex_head = raw_textures + tex_header->dataofs[i];
      miptex_t* miptex = (miptex_t*)local_tex_head;

      this->textures[i].width = miptex->width;
      this->textures[i].height = miptex->height;
      this->textures[i].color_data = &(local_tex_head[miptex->offset1]);
      this->textures[i].name = miptex->name;
      this->textures[i].palette = palette;
    }

    // texinfos
    bsp_entry_t& texinfos_entry = header->texinfo;
    this->texinfo_count = texinfos_entry.size / sizeof(texinfo_t);
    this->texinfos = (texinfo_t*)(&(raw_bsp.data()[texinfos_entry.offset]));

    // planes
    bsp_entry_t& planes_entry = header->planes;
    this->plane_count = planes_entry.size / sizeof(plane_t);
    this->planes = (plane_t*)(&(raw_bsp.data()[planes_entry.offset]));

    // faces
    bsp_entry_t& faces_entry = header->faces;
    this->face_count = faces_entry.size / sizeof(face_t);
    this->faces = (face_t*)(&(raw_bsp.data()[faces_entry.offset]));

    // face list

    // edges
    bsp_entry_t& edges_entry = header->edges;
    this->edge_count = edges_entry.size / sizeof(edge_t);
    this->edges = (edge_t*)(&(raw_bsp.data()[edges_entry.offset]));

    // edge list
    bsp_entry_t& edge_list_entry = header->edge_list;
    this->edge_list_size = edge_list_entry.size / sizeof(int*);
    this->edge_list = (int*)(&(raw_bsp.data()[edge_list_entry.offset]));

    // vertices
    bsp_entry_t& vertices_entry = header->vertices;
    this->vertex_count = vertices_entry.size / sizeof(vec3_t);
    this->vertices = (vec3_t*)(&(raw_bsp.data()[vertices_entry.offset]));
  }

  void log() {
    printf("number of models: %d\n", this->model_count);
    printf("number of textures: %lu\n", this->textures.size());
    printf("number of texinfos: %d\n", this->texinfo_count);
    printf("number of planes: %d\n", this->plane_count);
    printf("number of faces: %d\n", this->face_count);
    printf("number of edges: %d\n", this->edge_count);
    printf("size of edge list: %d\n", this->edge_list_size);
    printf("number of vertices: %d\n", this->vertex_count);

    // for (int i = 0; i < this->texinfo_count; i++) {
    //   texinfo_t* tex = (texinfo_t*)(&(this->texinfos[i]));

    //   printf("texinfo %d:\n", i);
    //   printf("  texture_id: %d\n", tex->texture_id);
    //   printf("  vectorS: ");
    //   tex->vectorS.log();
    // }

    for (int model_index = 0; model_index < this->model_count; model_index++) {
      model_t* model = (model_t*)(&(this->models[model_index]));

      printf(
          "number of faces in model %d: %d, first_face: %d\n",
          model_index,
          model->face_count,
          model->first_face);
      printf("  boundbox min: ");
      model->boundbox.min.log();
      printf("  boundbox max: ");
      model->boundbox.max.log();
      printf("  origin: ");
      model->origin.log();
      printf("  first bsp node: %d\n", model->first_bsp_node);
      printf("  first clip node: %d\n", model->first_clip_node);
      printf("  second clip node: %d\n", model->second_clip_node);
      printf("  empty node: %d\n", model->empty_node);
      printf("  bsp leaf count: %d\n", model->bsp_leaf_count);

      // // get faces
      // for (int face_index = 0; face_index < model->face_count; face_index++) {
      //   face_t& face = this->faces[face_index];

      //   printf("  face %d\n:", face_index);
      //   printf("    plane_id: %d\n:", face.plane_id);
      //   printf("    side: %d\n:", face.side);
      //   printf("    edge_list_id: %d\n:", face.edge_list_id);
      //   printf("    edge_count: %d\n:", face.edge_count);
      //   printf("    texinfo_id: %d\n:", face.texinfo_id);
      // }

      // // get texinfo
      // for (int face_index = model->first_face; face_index < model->face_count; face_index++) {
      //   face_t& face = this->faces[face_index];

      //   printf("  face %d\n", face_index);

      //   printf("    texinfo_id: %d\n", face.texinfo_id);
      //   texinfo_t* tex = (texinfo_t*)(&(this->texinfos[face.texinfo_id]));
      //   printf("      texture name: %s\n", this->textures[tex->texture_id].name);
      //   printf("      vectorS: ");
      //   tex->vectorS.log();
      //   printf("      vectorT: ");
      //   tex->vectorT.log();
      // }
    }
  }

  Model buildModel() {
    Model model{};

    // set up vertices
    model.vertices.resize(this->vertex_count);
    for (int i = 0; i < this->vertex_count; i++) {
      vec3_t& v = this->vertices[i];
      model.vertices[i] = Vector::point(v.x, v.y, v.z);
    }

    // set up uvs

    // set up triangles

    // set texture

    return model;
  }
};


constexpr const char* k_input_file = "/Users/james/quake/id1/maps/box.bsp";
constexpr const char* k_palette_file = "/Users/james/quake/palette.lmp";


int main() {
  // load the palette
  std::vector<unsigned char> raw_palette = util::readFile(k_palette_file);
  Color* palette = (Color*)raw_palette.data();

  std::vector<unsigned char> raw_bsp = util::readFile(k_input_file);

  BSP bsp(raw_bsp, palette);
  bsp.log();

  return EXIT_SUCCESS;
}
