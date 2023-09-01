#ifndef VIDEO_H
    #define VIDEO_H

    namespace video {
        void initialize();
        void init_sdcard();
        void record(const char* aviname);
        void copy(const char* file_r, const char* file_w);
        void clear(const char* file_path);
        char * c_read_file(const char * f_name, int * err, size_t * f_size);
    };

#endif