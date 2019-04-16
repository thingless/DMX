#!/usr/bin/env python
from __future__ import print_function
import sys
import pyaudio
import numpy as np
import btrack
import time
import argparse
import threading

#if ya forget what all numbers mean: https://stackoverflow.com/questions/35970282/what-are-chunks-samples-and-frames-when-using-pyaudio
MIC_RATE=44100
SECONDS_TO_SAMPLE=5

bps_lock=threading.Lock()
bps=2
beat_onset_fn=lambda:True

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def _steam_callback(in_data, frame_count, time_info, status):
    data = np.fromstring(in_data, dtype=np.float32)
    #onset_fn=btrack.calculateOnsetDF(data)
    beats = btrack.trackBeats(data)
    b = round(1.0/np.mean(np.diff(beats)),2)
    eprint('Detected bpm', 60.0/b, 'aka bps', b)
    global bps
    global beat_onset_fn
    with bps_lock:
        bps = b
        #beat_onset_fn = onset_fn
    return (None, pyaudio.paContinue)

def start_calc_beat_delta():
    audio = pyaudio.PyAudio()
    audio.open(format=pyaudio.paFloat32, channels=1, rate=MIC_RATE, input=True, output=False, frames_per_buffer=MIC_RATE*SECONDS_TO_SAMPLE, stream_callback=_steam_callback)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Detects bps and updates based on it")

    parser.add_argument("--brightness", type=int, default=20, help="Maximum brightness per channel (0-255)")
    parser.add_argument("--leds", type=int, default=4, help="How many LEDs to generate channels for? (def 4)")
    args = parser.parse_args()
    start_calc_beat_delta()

    curr_bps=bps
    while True:
        with bps_lock:
            beat_onset_fn()
            mybps=bps
        if abs(mybps-curr_bps) > 5:
            curr_bps=mybps
            print(10, 0, 5,5,5,5,5,5,5,5,5,5,5,5)
            print(int(round((100.0/bps)-10)), 0, 0,0,0,0,0,0,0,0,0,0,0,0)
            print('END')
            sys.stdout.flush()
        time.sleep(SECONDS_TO_SAMPLE-0.25) #nothing going to happen for a while
