#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_FLASH_SIZE          (1*1024*1024)   /* 1 MB */
#define MAX_BOOTLOADER_SIZE     (8*1024)        /* 8Kb */
#define MAX_BIN_SIZE            ((MAX_FLASH_SIZE - MAX_BOOTLOADER_SIZE) >> 1)
#define SIGNATURE1              (0xAA55FADE)
#define SIGNATURE2              (0x55AAC0DE)

static char *input_binary;
static char binmap[MAX_BIN_SIZE];
static int bin_size;

struct binary_header {
        uint32_t sig1;
        uint32_t sig2;
        uint32_t bin_size;
        uint32_t crc32;
};

static int get_bin_size(FILE *bin_file)
{
        int size;
        fseek(bin_file, 0, SEEK_END);
        size = ftell(bin_file);
        if (size >= MAX_BIN_SIZE) {
                fprintf(stderr, "binary file is too large to fit in mcu\n");
                exit(-1);
        }
        fseek(bin_file, 0, SEEK_SET);
        return size;
}

static void patch_binary(struct binary_header *hdr, int size, uint32_t checksum)
{
        hdr->bin_size = size;
        hdr->crc32 = checksum;
        return;
}

int main(int argc, char **argv)
{
        FILE *bin_file;
        uint32_t *iter32;
        int s;
        uint32_t checksum = 0;

        //if (argc != 2) {
        //        fprintf(stderr, "%s requires a binary file argument\n", argv[0]);
        //        exit(0);
        //}
        //input_binary = argv[1];
        input_binary = "Z:\\git\\test_bootloader_app\\firmware\\test_bootloader_app.X\\dist\\default\\production\\test_bootloader_app.X.production.bin";
        bin_file = fopen(input_binary, "r+b");
        if (!bin_file) {
                fprintf(stderr, "cannot open binary file %s\n", input_binary);
                exit(-1);
        }
        bin_size = get_bin_size(bin_file);

        /* now copy the binary to memory so we can parse it easier */
        fread(binmap, 1, bin_size, bin_file);

        /* find the bootloader signature */
        for (s = 0, iter32 = binmap; s < (bin_size - 8); s+=4, iter32++) {
                if ((iter32[0] == SIGNATURE1) && (iter32[1] == SIGNATURE2)) {
                        printf("found signature! patching binary...\n");

                        /* we get checksum of the binary, but entirely skipping the header */

                        patch_binary(iter32, bin_size, checksum);
                        break;
                }
        }

        fclose(bin_file);
        return;
}