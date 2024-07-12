import numpy as np

def tone_generator():
    # Parameters
    frequency = 1000  # Frequency of the tone in Hz
    sample_rate = 44100  # Sample rate in Hz
    buffer_length = 256  # Buffer length

    # Generate the time values for one buffer length
    t = np.arange(buffer_length) / sample_rate

    # Generate the sine wave
    sine_wave = 0.5 * np.sin(2 * np.pi * frequency * t)

    # Convert to 16-bit PCM format (standard for I2S)
    sine_wave_pcm = np.int16(sine_wave * 32767)

    # Display the array
    return sine_wave_pcm
    
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
            f.write(f'{sample}, ')
            if (i + 1) % 10 == 0:
                f.write('\n')
        
        # Close array and header guard
        f.write('\n};\n\n')
        f.write('#endif // PCM_DATA_H\n')

# Example usage
sine_wave = tone_generator()

header_file = 'pcm_data.h'
write_pcm_to_header(sine_wave, header_file)