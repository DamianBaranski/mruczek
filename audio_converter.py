import wave
import struct
import argparse

def wav_to_c_array(wav_filename, output_header):
    try:
        # Open the WAV file
        with wave.open(wav_filename, 'rb') as wav_file:
            # Get basic information
            n_channels = wav_file.getnchannels()
            sample_width = wav_file.getsampwidth()
            sample_rate = wav_file.getframerate()
            n_frames = wav_file.getnframes()
            audio_data = wav_file.readframes(n_frames)
            
            # Check for supported formats (8-bit or 16-bit PCM)
            if sample_width == 1:  # 8-bit PCM (unsigned)
                fmt = f'{n_frames * n_channels}B'  # Unsigned char
            elif sample_width == 2:  # 16-bit PCM (signed)
                fmt = f'{n_frames * n_channels}h'  # Signed short
            else:
                raise ValueError(f"Unsupported sample width: {sample_width} bytes")

            # Unpack the audio data into Python data
            try:
                audio_samples = struct.unpack(fmt, audio_data)
            except struct.error as e:
                raise ValueError(f"Error unpacking audio data: {e}")

            # Prepare the C array
            c_array = f'const int16_t audio_data[{len(audio_samples)}] = {{\n'
            c_array += ', '.join(map(str, audio_samples))
            c_array += '\n};\n'

            # Write the array to a header file
            with open(output_header, 'w') as header_file:
                header_file.write("// Auto-generated from {}\n".format(wav_filename))
                header_file.write("// Channels: {}, Sample Rate: {} Hz, Frames: {}\n".format(
                    n_channels, sample_rate, n_frames))
                header_file.write(c_array)
            
            print(f"Successfully converted {wav_filename} to {output_header}.")
    except Exception as e:
        print(f"Error processing file: {e}")

def main():
    # Set up argument parser
    parser = argparse.ArgumentParser(description="Convert WAV file to C array")
    parser.add_argument("wav_file", help="Path to the input WAV file")
    parser.add_argument("output_header", help="Path to the output header file")
    
    # Parse command-line arguments
    args = parser.parse_args()
    
    # Call the conversion function with the provided arguments
    wav_to_c_array(args.wav_file, args.output_header)

if __name__ == "__main__":
    main()

