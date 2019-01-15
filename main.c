#include <endian.h>
#include <stdint.h>
#include <stdio.h>

struct chunk_header_t {
  uint8_t chunk_id[4];
  uint32_t chunk_size;
};

struct header_t {
  struct chunk_header_t chunk_header;
  uint16_t format_type;
  uint16_t number_of_tracks;
  uint16_t time_division;
};

void read_header(struct header_t *header, FILE *file) {
  fread(header, 14, 1, file);
  header->chunk_header.chunk_size = be32toh(header->chunk_header.chunk_size);
  header->format_type = be16toh(header->format_type);
  header->number_of_tracks = be16toh(header->number_of_tracks);
  header->time_division = be16toh(header->time_division);

  printf("%.4s\n", header->chunk_header.chunk_id);
  printf("format_type %u\n", header->format_type);
  printf("number_of_tracks %u\n", header->number_of_tracks);
  printf("time_division %u\n", header->time_division);
}

void read_var(int *out, FILE *file) {
  uint8_t temp;
  *out = 0;
  for (int i = 0; i < 4; i++) {
    fread(&temp, 1, 1, file);
    *out = *out << 7 | (temp & 0x7F);
    if (!(temp & 0x80)) {
      break;
    }
  }
}

void read_event(FILE *file) {
  int deltaTime;
  uint8_t event_type;

  read_var(&deltaTime, file);
  fread(&event_type, 1, 1, file);

  printf("%d %x\n", deltaTime, event_type);

  if (event_type < 0xF0) {
    // MIDI Channel Event
  } else if (event_type >= 0xFF) {
    // Meta Event
    uint8_t meta_type;
    int size;
    fread(&meta_type, 1, 1, file);
    read_var(&size, file);
    uint8_t bytes[size+1];
    fread(bytes, size, 1, file);

    printf("meta %d %s", meta_type, bytes);
  } else {
    // System Exclusive Event
  }
}

void read_track(FILE *file) {
  struct chunk_header_t chunk_header;
  int end;

  fread(&chunk_header, 8, 1, file);
  chunk_header.chunk_size = be32toh(chunk_header.chunk_size);
  end = ftell(file) + chunk_header.chunk_size;

  printf("%.4s\n", chunk_header.chunk_id);
  
  while (ftell(file) < end) {
    read_event(file);
  }
}

int main(int argc, char** argv) {
  struct header_t header;
  FILE *file;

  file = fopen(argv[argc-1], "rb");

  read_header(&header, file);
  for (int i = 0; i < header.number_of_tracks; i++) {
    read_track(file);
  }
}
