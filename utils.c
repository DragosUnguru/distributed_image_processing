#include "utils.h"

img_t* parse_img(char* filename) {
    img_t* image = (img_t*) malloc(sizeof(img_t));
    FILE* fptr = fopen(filename, "r+");
    unsigned int image_size;
    int i, offset, pxl_size;

    DIE(image == NULL, "Malloc failed!");
    DIE(fptr == NULL, "Error opening input file");

    // Skip "P"
    fgetc(fptr);

    // Read image type
    fscanf(fptr, "%hhu\n", &(image->P));

    // Manage pixel type
    pxl_size = (image->P == 5) ? 1 : 3;

    // Skip comment
    fgets(image->comment, COMMENT_LEN, fptr);

    // Read image width, height and pixel maxvalue
    fscanf(fptr, "%d %d\n", &(image->width), &(image->height));
    fscanf(fptr, "%hhu\n", &(image->maxval));

    // Manage image data size including the size needed for padding
    image_size = image->height * image->width + 2 * (image->width + image->height) + 4;
    image_size *= pxl_size;

    // Skip added border
    offset = (image->width + 3) * pxl_size;

    // Read image data
    image->data = (unsigned char*) calloc(image_size, sizeof(unsigned char));
    DIE(image->data == NULL, "Memory allocation failed!");

    const int step = (image->width + 2) * pxl_size;
    for (i = 0; i < image->height; i++, offset += step) {
        fread(image->data + offset, sizeof(unsigned char), image->width * pxl_size, fptr);
    }

    fclose(fptr);
    return image;
}

void flush_img(char* filename, img_t* image) {
    FILE* fptr = fopen(filename, "w+");
    int i, offset;
    int pxl_size = (image->P == 5) ? 1 : 3;

    DIE(fptr == NULL, "Error opening output file");

    // Write image metadata
    fprintf(fptr, "P%u\n", image->P);
    fprintf(fptr, "%d %d\n%u\n", image->width, image->height, image->maxval);

    // Write image data
    offset = (image->width + 3) * pxl_size;
    const int step = (image->width + 2) * pxl_size;

    for (i = 0; i < image->height; i++, offset += step) {
        fwrite(image->data + offset, sizeof(unsigned char), image->width * pxl_size, fptr);
    }

    fclose(fptr);
}

float* get_filter(char* filter) {
    float* kernel = (float*) malloc(9 * sizeof(float));
    DIE(kernel == NULL, "Malloc failed!");
    int i;

    if (!strcmp(filter, "smooth")) {
        for (i = 0; i < 9; ++i) {
            kernel[i] = 1.f / 9.f;
        }
    }
    else if (!strcmp(filter, "blur")) {
        for (i = 0; i < 9; ++i) {
            if (i % 2 == 0) {
                kernel[i] = 1.f / 16.f;
            } else {
                kernel[i] = 2.f / 16.f;
            }
        }
        kernel[4] = 4.f / 16.f;
    }
    else if (!strcmp(filter, "sharpen")) {
        for (i = 0; i < 9; ++i) {
            float fact = (i % 2 == 0) ? 0.f : -2.f;

            kernel[i] = (1.f / 3.f) * fact;
        }
        kernel[4] = 11.f / 3.f;
    }
    else if (!strcmp(filter, "mean")) {
        for (i = 0; i < 9; ++i) {
            kernel[i] = -1.f;
        }
        kernel[4] = 9.f;
    }
    else if (!strcmp(filter, "emboss")) {
        for (i = 0; i < 9; ++i) {
            kernel[i] = 0.f;
        }

        kernel[1] = -1.f;
        kernel[7] = 1.f;
    }
    else {
        DIE(1, "Received invalid filter!");
    }

    return kernel;
}

unsigned char* apply_filter_chunk(
unsigned char P, unsigned char maxvalue,
int rank, int nProcesses, char* filter_name,
int line_width, int buff_len,
unsigned char* img_data,
unsigned char* upper_line,
unsigned char* bottom_line) {

    int pxl_size = (P == 5) ? 1 : 3;
    int i, j, start, end;
    unsigned char* final_data = (unsigned char*) calloc(buff_len, sizeof(unsigned char));
    unsigned char* data;
    int indices[9];
    float* filter = get_filter(filter_name);

    DIE(final_data == NULL, "Malloc failed!");

    // Pad our image chunk with needing upper and / or bottom lines
    if (rank == MASTER_PROC) {
        // Skip the '0' padding at the beggining and manage ending point
        start = line_width + pxl_size;
        end = buff_len - pxl_size;

        // If this is the only process, we won't have a bottom line and the ending point differs
        if (nProcesses > 1) {
            data = (unsigned char*) calloc(buff_len + line_width, sizeof(unsigned char));
            DIE(data == NULL, "Malloc failed!");
            
            // Add bottom missing line
            memcpy(data + buff_len, bottom_line, line_width * sizeof(unsigned char));
        } else {
            data = (unsigned char*) calloc(buff_len, sizeof(unsigned char));
            DIE(data == NULL, "Malloc failed!");

            end -= line_width;
        }
        
        memcpy(data, img_data, buff_len * sizeof(unsigned char));
    }
    else if (rank == nProcesses - 1) {
        // Skip the '0' padding at the end and manage starting point
        start = pxl_size;
        end = buff_len - line_width - pxl_size;

        data = (unsigned char*) calloc(buff_len + line_width, sizeof(unsigned char));
        DIE(data == NULL, "Malloc failed!");

        // Add upper missing line
        memcpy(data, upper_line, line_width * sizeof(unsigned char));
        memcpy(data + line_width, img_data, buff_len * sizeof(unsigned char));
    }
    else {
        // Manage starting and ending points
        start = pxl_size;
        end = buff_len - pxl_size;

        data = (unsigned char*) calloc(buff_len + 2 * line_width, sizeof(unsigned char));
        DIE(data == NULL, "Malloc failed!");

        // Add both upper and bottom missing lines
        memcpy(data, upper_line, line_width * sizeof(unsigned char));
        memcpy(data + line_width, img_data, buff_len * sizeof(unsigned char));
        memcpy(data + buff_len + line_width, bottom_line, line_width * sizeof(unsigned char));
    }

    // Manage indices offsets

    // Add an additional line_width in for chunks that use an upper new line
    int upper_offset = (rank == MASTER_PROC) ? 0 : line_width;

    int dimension_switch = -line_width;
    int offset[3] = { -pxl_size, 0, pxl_size };

    // Build indices that define the square around any pixel pixel
    for (i = 0; i < 9; ++i) {
        // Switch lanes
        if (!(i % 3) && (i)) {
            dimension_switch += line_width;
        }

        indices[i] = dimension_switch + offset[i % 3] + upper_offset;
    }

    for (i = start; i < end; ++i) {
        float tmp = 0.f;

        // Escape '0' padding of the border
        if (i % line_width < pxl_size || i % line_width >= line_width - pxl_size) {
            continue;
        }

        // Compute filtered pixel using neighbours
        for (j = 0; j < 9; ++j) {
            tmp += (float) (data[i + indices[j]]) * filter[j];
        }

        final_data[i] = CLAMP(tmp, maxvalue);
    }

    free(data);
    free(filter);
    
    return final_data;
}

void destroy_img(img_t* image) {
    free(image->data);
    free(image);
}