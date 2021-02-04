
#ifndef app_h_
#define app_h_

typedef struct
{
    char riff_header[4]; 
    int wav_size;        
    char wave_header[4]; 
    
    char fmt_header[4]; 
    int fmt_chunk_size; 
    short audio_format; 
    short num_channels;
    int sample_rate;
    int byte_rate;          
    short sample_alignment; 
    short bit_depth;        
    
    char data_header[4]; 
    int data_bytes;      
} wav_header_t;

#endif