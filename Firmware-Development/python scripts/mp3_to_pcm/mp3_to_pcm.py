from pydub import AudioSegment
import numpy as np

def mp3_to_pcm_array(mp3_file):
    # Load the MP3 file
    audio = AudioSegment.from_mp3(mp3_file)
    
    # Ensure the audio is in the desired format
    audio = audio.set_frame_rate(44100)  # Set sample rate to 44.1 kHz
    audio = audio.set_channels(2)        # Set to stereo
    audio = audio.set_sample_width(3)    # Set sample width to 3 bytes (24 bits)
    
    # Convert to raw PCM samples
    pcm_data = np.array(audio.get_array_of_samples())
    
    # If stereo, interleave left and right channels
    if audio.channels == 2:
        pcm_data = pcm_data.reshape((-1, 2))
    
    return pcm_data

# Example usage
mp3_file = 'audio/file.mp3'
pcm_array = mp3_to_pcm_array(mp3_file)

# Save the array to a file (optional)
np.save('pcm_array.npy', pcm_array)
