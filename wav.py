import wave
from scipy.io.wavfile import read
import numpy as np
import sys

# f = wave.open('8bit.wav', 'rb')

# print(f'Channels = {f.getnchannels()}')
# print(f'Bytes per Sample = {f.getsampwidth()}')
# print(f'Sample rate = {f.getframerate()}')

# print(f.getnframes())
# print(f.getnframes() / f.getframerate())
# print(f.readframes(10000))

# f.close()

a = read('tpg8b22k.wav')
# b = read('tpg8b22kN.wav')
# np.set_printoptions(threshold=10000)
array = np.array(a[1])
# b_array = np.array(b[1])
# for i in zip(array[:10000], b_array[:10000]):
    # print(i, end=' ')

for i in array[:10000]:
    print(i, end=' ')