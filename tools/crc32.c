// crc32 msb implementation

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define BIT_31 0x80000000u

static uint32_t crc32(const uint8_t *data, size_t len);
static uint32_t const divisor = 0x04c11db7;

typedef struct{
    uint8_t *buf;
    size_t size;
    char error[128];
} file_handle;

__attribute__((format(printf, 1, 2)))
static file_handle error_file_handle(const char *fmt, ...) {
    file_handle h = { .buf = NULL };

    va_list args;
    va_start(args, fmt);
    vsnprintf(h.error, sizeof h.error, fmt, args);
    va_end(args);

    return h;
}

static file_handle data_and_size(char *input_path) {
    FILE *file = fopen(input_path, "rb");
    if (file == NULL){
        return error_file_handle("error while opening file");
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    if (size <= 0) {
        fclose(file);
        return error_file_handle("ftell returned %ld", size);
    }
    fseek(file, 0, SEEK_SET);
    uint8_t *buf = malloc(size);
    if (buf == NULL){
        fclose(file);
        return error_file_handle("malloc returned NULL");
    }
    size_t got = fread(buf, 1, size, file);
    fclose(file);

    if (got != (size_t)size){
        free(buf);
        return error_file_handle("got size %zu, expected %ld", got, size);
    }

    if (got < 256){
        free(buf);
        return error_file_handle("file size less than 256: %zu", got);
    }

    file_handle h = { .buf = buf, .size = size, .error = ""};
    return h;
}

int main(int argc, char **argv){
    if(argc != 3){
        fprintf(stderr, "expected 2 arguments, given %d\n", argc - 1);
        fprintf(stderr, "USAGE: %s <path_to_file> <path_to_out_file>\n", argv[0]);
        return 1;
    }
    file_handle handle = data_and_size(argv[1]);
    if (handle.error[0] != '\0'){
        fprintf(stderr, "getting data and size: %s\n", handle.error);
        return 1;
    }

    uint32_t crc = crc32(handle.buf, 252);
    handle.buf[0xfc] = crc >> 0;
    handle.buf[0xfd] = crc >> 8;
    handle.buf[0xfe] = crc >> 16;
    handle.buf[0xff] = crc >> 24;

    FILE *target_file = fopen(argv[2], "wb");
    if (target_file == NULL) {
        fprintf(stderr, "open file '%s' error", argv[2]);
        return 1;
    }
    unsigned long write_result = fwrite(handle.buf, 1,  handle.size, target_file);
    if (write_result != handle.size ){
        fclose(target_file);
        free(handle.buf);
    fprintf(stderr, "written changed binary is %lu legnth, expected: %zu\n", write_result, handle.size);
        return 1;
    }

    fclose(target_file);
    free(handle.buf);
    return 0;
}

// crc32 calcualtes crc and returns 4 bytes.
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
