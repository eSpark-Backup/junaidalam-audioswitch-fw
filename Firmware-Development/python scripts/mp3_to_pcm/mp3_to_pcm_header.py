from pydub import AudioSegment
import numpy as np

def mp3_to_pcm_array(mp3_file):
    # Load the MP3 file
    audio = AudioSegment.from_mp3(mp3_file)
    
    # Ensure the audio is in the desired format
    audio = audio.set_frame_rate(44100)  # Set sample rate to 44.1 kHz
    audio = audio.set_channels(2)        # Set to stereo
    audio = audio.set_sample_width(2)    # Set sample width to 2 bytes (16 bits)
    
    # Convert to raw PCM samples
    pcm_data = np.array(audio.get_array_of_samples(), dtype=np.int16)
    
    return pcm_data

def write_pcm_to_header(pcm_array, header_file):
    array_length = len(pcm_array)
    with open(header_file, 'w') as f:
        # Write header guard
        f.write('#ifndef PCM_DATA_H\n')
        f.write('#define PCM_DATA_H\n\n')
        
        # Write array length
        f.write(f'const unsigned int pcm_data_length = {array_length};\n\n')
        
        # Write array declaration
        f.write('const int16_t pcm_data[] = {\n')
        
        # Write PCM data
        for i, sample in enumerate(pcm_array):
            if i % 10 == 0:
                f.write('    ')  # Indent every 10 samples
            f.write(f'{sample}, ')
            if (i + 1) % 10 == 0:
                f.write('\n')
        
        # Close array and header guard
        f.write('\n};\n\n')
        f.write('#endif // PCM_DATA_H\n')

# Example usage
mp3_file = 'audio/file.mp3'
pcm_array = mp3_to_pcm_array(mp3_file)

header_file = 'pcm_data.h'
write_pcm_to_header(pcm_array, header_file)
