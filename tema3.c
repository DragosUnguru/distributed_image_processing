#include "utils.h"

int main(int argc, char * argv[]) {
	int rank;
	int nProcesses;
    int i, toProc, fromProc;

	MPI_Init(&argc, &argv);
	MPI_Status status;
	MPI_Request request;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

    if (rank == MASTER_PROC) {
        img_t* image = parse_img(argv[1]);
        int pxl_size = (image->P == 5) ? 1 : 3;
        int lines = image->height + 2;
        int line_width = (image->width + 2) * pxl_size;
        int seq = ceil((double) lines / (double) nProcesses);
        int my_buff_size = seq * line_width;

        for (i = 3; i < argc; ++i) {

            for (toProc = 1; toProc < nProcesses; ++toProc) {
                int start = seq * toProc;
                int stop = MIN(seq * (toProc + 1), lines);
                int buff_len = (stop - start) * line_width;

                // Send image type
                MPI_Send(&(image->P), 1, MPI_UNSIGNED_CHAR, toProc, 0, MPI_COMM_WORLD);

                // Send number of pixels
                MPI_Send(&buff_len, 1, MPI_INT, toProc, 0, MPI_COMM_WORLD);

                // Send line width with padding
                MPI_Send(&line_width, 1, MPI_INT, toProc, 0, MPI_COMM_WORLD);

                // Send image's maxvalue for pixels
                MPI_Send(&(image->maxval), 1, MPI_UNSIGNED_CHAR, toProc, 0, MPI_COMM_WORLD);
            
                // Send image chunk to be processed
                MPI_Send(image->data + start * line_width, buff_len, MPI_UNSIGNED_CHAR, toProc, 0, MPI_COMM_WORLD);
                
                // Send upper missing line
                MPI_Send(image->data + (start - 1) * line_width, line_width, MPI_UNSIGNED_CHAR, toProc, 0, MPI_COMM_WORLD);

                if (toProc != nProcesses - 1) {
                    // Send bottom missing line
                    MPI_Send(image->data + stop * line_width, line_width, MPI_UNSIGNED_CHAR, toProc, 0, MPI_COMM_WORLD);
                }
            }

            // Apply filter on first chunk
            unsigned char* upper_line = NULL;
            unsigned char* bottom_line = image->data + my_buff_size;
            unsigned char* data = (unsigned char*) malloc(my_buff_size * sizeof(unsigned char));
            DIE(data == NULL, "Malloc failed!");

            memcpy(data, image->data, my_buff_size * sizeof(unsigned char));

            unsigned char* tmp_data = apply_filter_chunk(image->P, image->maxval, rank, nProcesses, argv[i], line_width, my_buff_size, data, upper_line, bottom_line);

            memcpy(image->data, tmp_data, my_buff_size * sizeof(unsigned char));
            free(tmp_data);
            free(data);

            // Gather image data from other procs
            for (fromProc = 1; fromProc < nProcesses; ++fromProc) {
                int start = seq * fromProc;
                int stop = MIN(seq * (fromProc + 1), lines);
                int buff_len = (stop - start) * line_width;

                MPI_Recv(image->data + start * line_width, buff_len, MPI_UNSIGNED_CHAR, fromProc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        flush_img(argv[2], image);
        destroy_img(image);

    } else {

        for (i = 3; i < argc; ++i) {
            int buff_len, line_width, type;
            unsigned char P, maxvalue;
            char filter_name[16];

            // Receive metadata
            MPI_Recv(&P, 1, MPI_UNSIGNED_CHAR, MASTER_PROC, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&buff_len, 1, MPI_INT, MASTER_PROC, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&line_width, 1, MPI_INT, MASTER_PROC, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&maxvalue, 1, MPI_UNSIGNED_CHAR, MASTER_PROC, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Receive image chunk and bording lines
            unsigned char* data = (unsigned char*) malloc(buff_len * sizeof(unsigned char));
            unsigned char* upper_line = (unsigned char*) malloc(line_width * sizeof(unsigned char));
            unsigned char* bottom_line = NULL;

            DIE(data == NULL, "Malloc failed!");
            DIE(upper_line == NULL, "Malloc failed!");


            MPI_Recv(data, buff_len, MPI_UNSIGNED_CHAR, MASTER_PROC, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(upper_line, line_width, MPI_UNSIGNED_CHAR, MASTER_PROC, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (rank != nProcesses - 1) {
                bottom_line = (unsigned char*) malloc(line_width * sizeof(unsigned char));
                DIE(bottom_line == NULL, "Malloc failed!");

                MPI_Recv(bottom_line, line_width, MPI_UNSIGNED_CHAR, MASTER_PROC, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            unsigned char* res = apply_filter_chunk(P, maxvalue, rank, nProcesses, argv[i], line_width, buff_len, data, upper_line, bottom_line);

            // Send back data to master process
            MPI_Send(res, buff_len, MPI_UNSIGNED_CHAR, MASTER_PROC, 0, MPI_COMM_WORLD);

            // Free data
            free(data);
            free(res);
            free(upper_line);
            if (bottom_line != NULL) {
                free(bottom_line);
            }
        }
    }
    
	MPI_Finalize();

	return 0;
}
