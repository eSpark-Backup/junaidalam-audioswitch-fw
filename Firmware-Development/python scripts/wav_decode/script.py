import wave
import struct

def wav_to_header(wav_filename, header_filename):
    # Open the WAV file
    with wave.open(wav_filename, 'rb') as wav_file:
        # Read WAV file header fields
        num_channels = wav_file.getnchannels()
        sample_width = wav_file.getsampwidth()
        framerate = wav_file.getframerate()
        num_frames = wav_file.getnframes()
        comp_type = wav_file.getcomptype()
        comp_name = wav_file.getcompname()
        wav_header = wav_file.readframes(0)  # Read header only

        # Read audio data
        audio_data = wav_file.readframes(num_frames)
        
        # Determine format string for unpacking audio data
        fmt = '<' + 'h' * (len(audio_data) // sample_width)

        # Unpack audio data into a list of samples
        samples = struct.unpack(fmt, audio_data)

    # Prepare WAV header struct
    wav_header_struct = {
        'chunkID': b'RIFF',
        'chunkSize': 36 + len(audio_data),
        'format': b'WAVE',
        'subchunk1ID': b'fmt ',
        'subchunk1Size': 16,
        'audioFormat': 1,
        'numChannels': num_channels,
        'sampleRate': framerate,
        'byteRate': framerate * num_channels * sample_width,
        'blockAlign': num_channels * sample_width,
        'bitsPerSample': sample_width * 8,
        'subchunk2ID': b'data',
        'subchunk2Size': len(audio_data),
    }

    # Create the header file
    with open(header_filename, 'w') as header_file:
        header_file.write("#ifndef WAV_DATA_H\n")
        header_file.write("#define WAV_DATA_H\n\n")

        # Write the WAV header struct
        header_file.write("typedef struct {\n")
        header_file.write("    char chunkID[4];\n")
        header_file.write("    uint32_t chunkSize;\n")
        header_file.write("    char format[4];\n")
        header_file.write("    char subchunk1ID[4];\n")
        header_file.write("    uint32_t subchunk1Size;\n")
        header_file.write("    uint16_t audioFormat;\n")
        header_file.write("    uint16_t numChannels;\n")
        header_file.write("    uint32_t sampleRate;\n")
        header_file.write("    uint32_t byteRate;\n")
        header_file.write("    uint16_t blockAlign;\n")
        header_file.write("    uint16_t bitsPerSample;\n")
        header_file.write("    char subchunk2ID[4];\n")
        header_file.write("    uint32_t subchunk2Size;\n")
        header_file.write("} WAV_Header;\n\n")

        # Write the WAV header data
        header_file.write("const WAV_Header wav_header = {\n")
        for key, value in wav_header_struct.items():
            if isinstance(value, bytes):
                header_file.write(f"    .{key} = {{")
                header_file.write(", ".join(f"0x{byte:02x}" for byte in value))
                header_file.write("},\n")
            else:
                header_file.write(f"    .{key} = {value},\n")
        header_file.write("};\n\n")

        # Write the audio data buffer
        header_file.write(f"const int16_t audio_data[{len(samples)}] = {{\n")
        for i, sample in enumerate(samples):
            if i % 12 == 0:
                header_file.write("    ")
            header_file.write(f"{sample}, ")
            if (i + 1) % 12 == 0:
                header_file.write("\n")
        if len(samples) % 12 != 0:
            header_file.write("\n")
        header_file.write("};\n\n")

        header_file.write("#endif // WAV_DATA_H\n")

# Usage
wav_to_header('audio.wav', 'wav_data.h')
