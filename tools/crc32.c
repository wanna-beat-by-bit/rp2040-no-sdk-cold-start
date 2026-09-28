// crc32 msb implementation

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static uint32_t const divisor = 0x04c11db7;
#define BIT_31 0x80000000u

#define USAGE "expected input and output path of elf file"

static uint32_t crc32(const uint8_t *data, size_t len);

typedef struct{
    uint8_t *buf;
    char *error;
} file_handle;

static file_handle error_file_handle(char *error_text){
    file_handle h = { .buf = NULL, .error = error_text};
    return h;
}

static file_handle data_and_size(char *input_path) {
    FILE *file = fopen(input_path, "rb");
    if (file == NULL){
        return error_file_handle( "error while opening file");
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t *buf = malloc(size);
    size_t got = fread(buf, 1, size, file);
    if (got != size){
        return error_file_handle( "fail while getting size of file");
    }
    fclose(file);

    file_handle h = { .buf = buf, .error = NULL};
    return h;
}

int main(int argc, char **argv){
    if(argc < 3 || argc > 3){
        fprintf(stderr, "expected exactly 3 arguments, given %d", argc);
        printf("USAGE: /path/to/binary <path_to_file> <path_to_out_file>");
        return 1;
    }
    file_handle handle = data_and_size(argv[1]);
    if (handle.error != NULL){
        fprintf(stderr, "getting data and size: %s", handle.error);
    }

    uint32_t sum = crc32(handle.buf, 252);

    return 0;
}


static uint32_t crc32(const uint8_t *data, size_t len){
    uint32_t crc = 0xffffffff;
    uint32_t is_set = 0;

    for(size_t i = 0; i < len; i++){
        uint8_t byte = data[i];
        crc ^= (uint32_t)byte << 24;
        for(int j = 0; j < 8; j++){
            is_set = crc & BIT_31;
            crc <<= 1;
            if(is_set){
                crc ^= divisor;
            }
        }
    }

    return crc;
}